#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <Arduino.h>

//Initializes the persistent MQTT Backgroud socket connection
void setupMQTT();

void maintainMQTT();

void publishTemperature(int temperature);

#endif //MQTT_CLIENT_H

