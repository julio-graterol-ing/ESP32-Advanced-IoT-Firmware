#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "ServoControl.h"
#include "SensorRead.h"
#include "CloudClient.h"
#include "secrets.h"
#include "WiFiClientSecure.h"

AsyncWebServer server (80); //Establish local internet server on standard HTTP

//Control servo motor angle for physical actuator
int currentServoAngle = 0;
int sweepDirection = 1;
unsigned long previousServoMillis = 0;
const unsigned long SERVO_INTERVAL = 15; //Update kinematic every 15ms

//Embedded high performance HTML js ui source code
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta name="viewport" content="width=device-width, initial-scale=1" charset="UTF-8">
    <title>UMH IoT Station</title>
    <style>
        body { font-family: Arial, sans-serif; text-align: center; background: #f4f4f4; color: #333; margin: 0; padding: 20px;}
        h1 { color: #0056b3; margin-bottom: 30px; }
        .card  { background: white; padding: 25px; margin: 20px auto; max-width: 400px; border-radius: 12px; box-shadow: 0 4px 10px rgba(0,0,0,0.1); }
        .metric { font-size: 3rem; font-weight: bold; color: #2c3e50; margin: 10px 0; }
        .label { font-size: 1.1rem; color: #7f8c8d; text-transform: uppercase; letter-spacing: 1px; }
        .actuator { color: #d35400; }
    </style> 
</head>
<body>
    <h1>UMH IoT Station: telemetry & Actuators</h1>

    <div class="card">
        <div class="label"> Temperature</div>
        <div class="metric" id="temp">--</div>
        <div class="label">&deg;C</div>
    </div>

    <div class="card">
        <div class="label"> Humidity</div>
        <div class="metric" id="hum">--</div>
        <div class="label">%</div>
    </div>

    <div class="card">
        <div class="label">Servo Motor Position</div>
        <div class="metric actuator"><span id="servo">0</span>&deg;</div>
        <div class="label">Kinematic Angle</div>
    </div>

    <script>
        //High speed asynchonous routine to fetch sensor endpoints without reloading
        function updateTelemetry() {
            fetch('/temperature').then(response => response.text()).then(data => {
                document.getElementById('temp').innerText = data;
            });

            fetch('/humidity').then(response => response.text()).then (data => {
                document.getElementById('hum').innerText = data;
            });

        }

        //New petition to fetch servo actuator position
        function updateActuatorState() {
            fetch('/servo').then(response => response.text()).then(data => {
                document.getElementById('servo').innerText = data;
            });
         }

        
        //Instantie steady execution interval scheduler every 2000ms
        setInterval(updateTelemetry, 2000);
        setInterval(updateActuatorState, 150); //Sincronize servo actuator position with high frequency
        window.onload = function(){ updateTelemetry(); updateActuatorState(); };
    </script>
</body>
</html>
)rawliteral";

//FreeRTOS background task handle use to run cloud alerts independent
TaskHandle_t CloudTaskHandle = NULL;

void cloudAlertWorker(void * parameter) {
    //Secondary infinite loop, isolated from the main thread
    for(;;) {
        checkCloudAlerts(currentTemperature);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//Diagnostic test: is there any outboud internet reachability at all
void testInternetReachable() {
    WiFiClient client;
    IPAddress googleDNS(8, 8, 8, 8);
    Serial.println("[TEST-NET] Attempting plain TCP connect to 8.8.8.8:53...");
    if (client.connect(googleDNS, 53)) {
        Serial.println("[TEST-NET] Internet reachable! Connection SUCCESS");
        client.stop();
    } else {
        Serial.println ("[TEST-NET] Internet not reachable. Connection Failed");

    }
}

//Diagnostic test: raw TLS connection bypassing HTTPClient entirely
void testRawTLS () {
    WiFiClientSecure client;
    client.setInsecure();
    client.setHandshakeTimeout(30);
    Serial.println ("[TEST] Attempting raw TLS connect to maker.ifttt.com:443");
    if (client.connect("maker.ifttt.com", 443)) {
        Serial.print("[TEST] Raw TLS connection success");
        client.stop();
    } else {
        Serial.println("[TEST] Raw TLS connection failed");
    }
}

void setup() {
  Serial.begin(115200);
  setupClimateSensor(); //Initialize DHT11 sensor hardware

  //Initialize Servo hardware and PWM driver
  setupServoHardware();

  //Trigger internal Wifi hardware peripheral
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);


//non blocking connection watchdog loop
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

//Connection established: Print local IPaddress credentials
Serial.println("\n[SUCCESS] Wifi Connected active.");
Serial.print("[INFO] Server local IP Address");
Serial.println(WiFi.localIP());

//Disable Wifi radio power saving modem sleep for stability on strict networks
WiFi.setSleep(false);
Serial.print("[DEBUG] Wifi sleep mode is now: ");
Serial.println(WiFi.getSleep () ? "Enable (still sleeping!)" : "Disable (radio always on)");

//NetworkDiagnostics: confirm gateway/subnet/DNS 
Serial.print("[DEBUG] Gataway IP: ");
Serial.println(WiFi.gatewayIP());
Serial.print("[DEBUG] Subnet Mask: ");
Serial.println(WiFi.subnetMask());
Serial.print("[DEBUG] DNS IP: ");
Serial.println(WiFi.dnsIP());

//Run outbound connectivity diagnostic
testInternetReachable();
testRawTLS();

//Gateway Route: Serve master user interface HTML document
server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200,"text/html", index_html);
});

//Gateway route: Real time dynamic temperature API Endpoint
server.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(currentTemperature));
});

//Gataway Route: real time humidity API Endpoint
server.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200,"text/plain", String(currentHumidity));
});

//Gateway Route: Real time servo actuator position API Endpoint
server.on("/servo", HTTP_GET, [](AsyncWebServerRequest *request){
  request->send(200, "text/plain", String(currentServoAngle));
});

//Start the underlying network listening daemon
server.begin();
Serial.println("[STATUS] Async HTTP Server Engine running");

//Multi core task creation:
//Assing the HTTPS alert checking task to core 0 with 16kn of dedicated stack
xTaskCreatePinnedToCore(
    cloudAlertWorker, //Function that runs the task
    "CloudTask", //Task name
    16384, //Stack size
    NULL, //Input parameters
    1, // task priority
    &CloudTaskHandle, //Task handle for tracking
    0 //Core ID 
);

Serial.println("[SYSTEM] Multi core architecture initialized. Cloud assigned to core 0");

}


void loop() {
  unsigned long currentMillis = millis();

  updateClimateTelemetry();

  //Asynchronous kinematic servo sweep 
  if (currentMillis - previousServoMillis >= SERVO_INTERVAL) {
    previousServoMillis = currentMillis;
    currentServoAngle += sweepDirection;

    if (currentServoAngle >= 180 || currentServoAngle <= 0) {
      sweepDirection = -sweepDirection; //Reverse sweep direction
    }

    writeServoAngle(currentServoAngle); //Directly inject new angle
  }
}