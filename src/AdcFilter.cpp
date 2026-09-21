#include "AdcFilter.h"

//circular memory array layout used to accumulate sequential conversion ticks
static uint16_t sampleHistory[FILTER_WINDOW_SIZE] = {0};

//Tracking index tracking the current write pointer inside the rolling cache bounds
static uint8_t writePointer = 0;

//Internal accumalated summation cache avoiding linear array parsing inside ticks
static uint32_t runningSum = 0;

void setupAdcHardware() {
    //Establish target GPIO line strictly as an anlog input high impedance node
    pinMode(ANALOG_INPUT_PIN, INPUT);

    //Configure default atteniation to scale the 0V to 3.3V input range to 12bit
    analogSetAttenuation(ADC_11db);
}

uint16_t getFilteredAdcValue() {
    //Acquire instantaneous raw conversion data from target analog registry
    uint16_t rawSample = analogRead(ANALOG_INPUT_PIN);

    //Deduct the obsolete oldest index metadata from the active historical summation
    runningSum -= sampleHistory[writePointer];

    //Overwrite the specific ring buffer slot with the newly acquired sample
    sampleHistory[writePointer] = rawSample;

    //Inject the newest bit combination into the fast rolling sum registry
    runningSum += rawSample;

    //Roll the indexing counter tracking cursor forward using boundary validation wrap
    writePointer = ( writePointer + 1) % FILTER_WINDOW_SIZE;

    //Compute the definitive running mean and return teh stabilized 12bit interger
    return (uint16_t)(runningSum / FILTER_WINDOW_SIZE);
}