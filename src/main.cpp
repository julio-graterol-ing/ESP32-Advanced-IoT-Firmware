#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <DHT.h>

// Hardware pinout and driver configuration
#define DHTPIN 4          // Pin where the DHT sensor is connected
#define DHTTYPE DHT11

DHT dht (DHTPIN, DHTTYPE);
AsyncWebServer server (80); //Establish local internet server on standard HTTP

//Local network credentials registry
const char* ssid = "Your_SSID";
const char* password = "Your_Pasword";

//Global volatile register for telemetry storage
int currentTemperature = 0;
int currentHumidity = 0;
unsigned long previuosDHTMillis = 0;
const unsigned long DHT_INTERVAL = 2000;

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
    </style> 
</head>
<body>
    <h1>UMH Industrial Telemetry</h1>

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
        //Instantie steady execution interval scheduler every 2000ms
        setInterval(updateTelemetry, 2000);
        window.onload = updateTelemetry;
    </script>
</body>
</html>
)rawliteral";

void setup() {
  Serial.begin(115200);
  dht.begin();

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

//Start the underlying network listening daemon
server.begin();
Serial.println("[STATUS] Async HTTP Server Engine running");
}

void loop() {
  unsigned long currentMillis = millis();

  //Asynchrous background task Scheduler for enviromental monitoring
  if (currentMillis - previuosDHTMillis >= DHT_INTERVAL) {
    previuosDHTMillis = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    //shiel telemetry registers from reading failures (NaN values)
    if (!isnan(t) && !isnan(h)) {
      currentTemperature = (int)t;
      currentHumidity = (int)h;
    }
  }
}


