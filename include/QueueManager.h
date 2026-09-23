#ifndef QUEUE_MANAGER_H
#define QUEUE_MANAGER_H

#include <Arduino.h>

//Structure to pack enviromental metrics into a single binary payload
struct ClimateData {
    int temperature;
    int humidity;

};

//global external handle to access the safe registry from other modules
extern QueueHandle_t climateQueue;

//Global external handle to route potentiometer metrics across threads
extern QueueHandle_t potentiometerQueue; 

void setupQueueSystem();

#endif //QUEUE_MANAGER_H