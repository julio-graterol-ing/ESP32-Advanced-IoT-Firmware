#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "QueueManager.h"

//instatiate the Queue Handle Object as null pointer before Hardware allocation
QueueHandle_t climateQueue = NULL;
QueueHandle_t potentiometerQueue = NULL;

void setupQueueSystem() {
    //create a secure queue capable of holding up to 5 elements of type ClimateData
    //This allocates a thread safe static biffer size between Core 0 and Core 1
    climateQueue = xQueueCreate(5, sizeof(ClimateData));

    //Allocate memory for a single 16 bit unsigned integer slot for ADC storage
    potentiometerQueue = xQueueCreate(1, sizeof(uint16_t));

    if (climateQueue == NULL) {
        Serial.println ("[Critical Error] Failed to allocate memory for FreeRTOS Queue!");    
    } else {
        Serial.println("[SYSTEM] FreeRTOS Hardware Queue initialized successfully!");
    }
}