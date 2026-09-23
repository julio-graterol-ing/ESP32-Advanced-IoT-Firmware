#ifndef ADC_FILTER_H
#define ADC_FILTER_H

#include <Arduino.h>

//Specific analog input pin configuraed for hardwareADC1 channel 6
#define ANALOG_INPUT_PIN 32

//Total window size allocated for the rolling moving average buffer array
#define FILTER_WINDOW_SIZE 16

//Initializes the target analog input peripheral registers
void setupAdcHardware();

//Captures a raw sample and return the mathematically filteres running average
uint16_t getFilteredAdcValue();

#endif //ADC_FILTER_H