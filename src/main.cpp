#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "ServoControl.h"
#include "SensorRead.h"

AsyncWebServer server (80); //Establish local internet server on standard HTTP

//Local network credentials registry
const char* ssid = "YourSSID";
const char* password = "YourPassword";

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

void setup() {
  Serial.begin(115200);
  setupClimateSensor(); //Initialize DHT11 sensor hardware

  //Initialize Servo hardware and PWM driver
  setupServoHardware();

  //Trigger internal Wifi hardware peripheral
  WiFi.begin(ssid, password);


//non blocking connection watchdog loop
while (WiFi.status() != WL_CONNECTED) {
  delay(500);
  Serial.print(".");
}

//Connection established: Print local IPaddress credentials
Serial.println("\n[SUCCESS] Wifi Connected active.");
Serial.print("[INFO] Server local IP Address");
Serial.println(WiFi.localIP());

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


