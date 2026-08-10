#include "SensorRead.h"

// Instatntiate local physical objects and schedulers
DHT dht (DHTPIN, DHTTYPE);
unsigned long previousDHTMillis = 0;
const unsigned long DHT_INTERVAL = 2000; // Update interval for DHT sensor readings

//definition of global telemetry variables declared as extern
int currentTemperature = 0;
int currentHumidity = 0;

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
      currentTemperature = (int)(t);
      currentHumidity = (int)(h);
    }
  }
}