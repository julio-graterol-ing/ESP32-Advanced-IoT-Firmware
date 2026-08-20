#include "SensorRead.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "QueueManager.h"

// Instatntiate local physical objects and schedulers
DHT dht (DHTPIN, DHTTYPE);
unsigned long previousDHTMillis = 0;
const unsigned long DHT_INTERVAL = 2000; // Update interval for DHT sensor readings


void setupClimateSensor() {
  dht.begin(); // Initialize the DHT sensor
}

void updateClimateTelemetry() {
  unsigned long currentMillis = millis();

  // Asynchronous background task Scheduler for environmental monitoring
  if (currentMillis - previousDHTMillis >= DHT_INTERVAL) {
    previousDHTMillis = currentMillis;

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    // Shield telemetry registers from reading failures (NaN values)
    if (!isnan(t) && !isnan(h)) {
      ClimateData localPackage;
      localPackage.temperature = (int)(t);
      localPackage.humidity = (int)(h);

      //Push data structure to queue with 0ms block time if full to avoid freezzing core 1
      if (xQueueSend(climateQueue, &localPackage, 0) != pdPASS) {
        Serial.println("[QUEUE-WARN] Climate Queue full. Dropping older frame");
      } else {
        Serial.printf("[QUEUE-TX] Sent to queue -> Temp: %d, Hum: %d\n", localPackage.temperature, localPackage.humidity);
      }
    }
  }
}