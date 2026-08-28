#ifndef OTA_MANAGER_H
#define OTA_MANAGER_H

#include <Arduino.h>
#include <ArduinoOTA.h>

//Configures and initializes the wireless network OTA bootloader engine
void setupWirelessOTA();

//Polls the underlying network socket for incoming firmware update binary
void handleWirelessOTA();

#endif //OTA_MANAGER_H
