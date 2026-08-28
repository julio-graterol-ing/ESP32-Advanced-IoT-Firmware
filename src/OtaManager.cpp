#include "OtaManager.h"

void setupWirelessOTA() {
    //port definition for OTA listening sequence 
    ArduinoOTA.setPort(3232);

    //Set a human readdable hostname to identify the IoT node on the local network router
    ArduinoOTA.setHostname("umh-iot-station");

    //callback fired when the update hand starts successfully
    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else {
            type = "filesystem";
        }
        Serial.println("\n[OTA-START] Receiving remote binary update execution path" + type);

    });

    //Callback fired when the update completes its download phase
    ArduinoOTA.onEnd([]() {
        Serial.println("\n[OTA-SUCCESS] Flash write operation complete. Rebooting SoC...");
    });

    //Callback fired continuosly to track internal partition block write progress
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("[OTA-PROGRESS] Upload Progress: %u%%\r", (progress / (total / 100)));
    });

    //Error catching callback to log partition or authentication exceptions
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("[OTA.ERROR] Error code [%u]: ", error);
        if(error == OTA_AUTH_ERROR) Serial.println("Authentication Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Beging Flash Partition Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Network connection Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Binary Packet Received Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Transmission Verification Failed");

    });

    //Arm the underlyning network bootloader server engine
    ArduinoOTA.begin();
    Serial.println("[SUCCESS] Wireless over the air OTA Engine initialized");

}

void handleWirelessOTA() {
    //listen for incoming dynamic socket connection from the network
    ArduinoOTA.handle();
}