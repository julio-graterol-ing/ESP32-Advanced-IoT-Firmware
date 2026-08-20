#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "QueueManager.h"

//instatiate the Queue Handle Object as null pointer before Hardware allocation
QueueHandle_t climateQueue = NULL;

void setupQueueSystem() {
    //create a secure queue capable of holding up to 5 elements of type ClimateData
    //This allocates a thread safe static biffer size between Core 0 and Core 1
    climateQueue = xQueueCreate(5, sizeof(ClimateData));

    if (climateQueue == NULL) {
        Serial.println ("[Critical Error] Failed to allocate memoru for FreeRTOS Queue!");    
    } else {
        Serial.println("[SYSTEM] FreeRTOS Hardware Queue initialized successfully!");
    }
}