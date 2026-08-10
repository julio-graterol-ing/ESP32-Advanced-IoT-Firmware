#ifndef SENSOR_READ_H
#define SENSOR_READ_H

#include <Arduino.h>
#include <DHT.h>

//Hardware pinout and sensor type definition
#define DHTPIN 4          // Pin where the DHT sensor is connected
#define DHTTYPE DHT11     // DHT 11 sensor type

//Share colatile telemetry registers with external modules
extern int currentTemperature;
extern int currentHumidity;

void setupClimateSensor();

void updateClimateTelemetry();

#endif //SENSOR_READ_H