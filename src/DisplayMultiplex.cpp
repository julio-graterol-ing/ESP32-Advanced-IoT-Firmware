#include "DisplayMultiplex.h"
#include "QueueManager.h"

//Mapping: [A=D23, B=D22, C=D19, D=D2, E=D21, F=TX2, G=RX2, DP=D5
const uint8_t SEGMENT_PINS[8] = {23, 22, 19, 2, 21, 17, 16, 5};

//Position sequence layout: [D1=D15, D2=D14, D3=D13, D4=D12]
const uint8_t DIGIT_PINS[4] = {15, 14, 13, 12};

//New extended version with numbers and letters
const uint8_t SEVEN_SEGMENT_LUT[13] ={
    0b00111111, //0
    0b00000110, //1
    0b01011011, //2
    0b01001111, //3
    0b01100110, //4
    0b01101101, //5
    0b01111101, //6
    0b00000111, //7
    0b01111111, //8
    0b01101111,  //9
    0b01111000, //index 10 charecter 't'
    0b01110110, //index 11 character 'h'
    0b00000000, //index 12 blank display / off 

};

void setupDisplayHardware() {
    //Configure all common anode segment lines as standard digital outputs
    for (int i = 0; i < 8; i++) {
        pinMode(SEGMENT_PINS[i], OUTPUT);
        digitalWrite(SEGMENT_PINS[i], LOW); //Grouund lines initially
    
    }
    //Configure all common cathode digit selection lines as standard digital outputs
    for(int i = 0; i < 4; i++) {
        pinMode(DIGIT_PINS[i], OUTPUT);
        digitalWrite(DIGIT_PINS[i], HIGH); // Deacyivate common pins
    }
    Serial.println("[SUCCESS] Bare metal 5461AS Display Driver pins configured");
}

void clearDisplaySegments() {
    //Drive all segment line register back to zero ticks
    for (int i = 0; i < 8; i++) {
        digitalWrite(SEGMENT_PINS[i], LOW);

    }
}

void projectDigitToSlot(uint8_t digitIndex, uint8_t numberValue) {
    //Bound guard validation check extended to index 12 for alphabetical and blank characters
    if (digitIndex > 3 || numberValue >12) return;

    //Phase A: Enforce a strict clear write to complety eliminate phantom ghosting artifact
    for (int i = 0; i < 4; i++) {
        digitalWrite(DIGIT_PINS[i], HIGH); //Blind out all common cathodes

    }
    clearDisplaySegments();

    //Phase B: Extract binary mask from lookuo truth table register match
    uint8_t segmentMask = SEVEN_SEGMENT_LUT[numberValue];

    //Bit shift loops evaluating specific bits to set corresponsing physical pun registers
    for (int segment = 0; segment < 8; segment++) {
        bool bitValue = (segmentMask >> segment) & 0x01;
        digitalWrite(SEGMENT_PINS[segment], bitValue ? HIGH : LOW);

    }
    //Phase C: Selectively activate the common cathode ground return pin for the targeted digit slot
    digitalWrite(DIGIT_PINS[digitIndex], LOW); 
}

void displayUpdateTask(void *parameter) {
    uint8_t activeDigitSlot = 0;
    uint16_t sharedPotentiometerValue = 0;
    uint8_t targetDigits[4] = {12, 12, 12, 12}; //Initialized to blank slots

    //external tracking structures for the 3 second state sequencing engine
    uint8_t activeDashboardState = 0; // 0 = Potentiometer, 1 = Temperature, 2 = humidity
    TickType_t lastStateChangeTicks = xTaskGetTickCount();
    const TickType_t stateDurationTicks = pdMS_TO_TICKS(3000); // 3000ms duration windows
    const TickType_t xDelay4ms = pdMS_TO_TICKS(4);

    //References to the colatile telemetry variables updated by Core 1 inside SensorRead module
     extern int currentTemperature;
     extern int currentHumidity;

    //continous real time scheduling loop for Core 1 execution context
    for(;;) {

        TickType_t currentTick = xTaskGetTickCount();

        //Non blocking poll attempt to extract the latest 12 bit sample from the hardware queue
        if (xQueueReceive(potentiometerQueue, &sharedPotentiometerValue, 0) == pdTRUE) {
        }

        // Asynchornous state trasition evaluator validating if the 3000ms window has expired
        if (currentTick - lastStateChangeTicks >= stateDurationTicks) {
            lastStateChangeTicks = currentTick;
            activeDashboardState = (activeDashboardState + 1) %3; // Cycle smootly between states 0, 1, 2
        }
        
        //Multiplexing encoder processing numerical formatting based on active dashboard view
        if (activeDashboardState == 0) {
            //Decompose the 12 bit ADC sample into individual decimal digits for display projection
            targetDigits[0] = (sharedPotentiometerValue / 1000) % 10; //Thousands place
            targetDigits[1] = (sharedPotentiometerValue / 100) % 10;  //Hundreds place
            targetDigits[2] = (sharedPotentiometerValue / 10) % 10;   //Tens place
            targetDigits[3] = sharedPotentiometerValue % 10;          //Units place
        }

        else if (activeDashboardState == 1) {
            //View 1 Temperature representation (Displays format 't', blank, tens, units -> e.g, "t 25")
            targetDigits[0] = 10; //character 't' index mapping
            targetDigits[1] = 12; // character 'Blank' index mapping
            targetDigits[2] = (currentTemperature / 10) % 10;
            targetDigits[3] = currentTemperature % 10;
        }

        else if (activeDashboardState == 2) {
            //View 2 Humidity representation 
            targetDigits[0] = 11; //Character 'H' index mapping
            targetDigits[1] = 12; //Character 'Blank' index mapping
            targetDigits[2] = (currentHumidity / 10) % 10;
            targetDigits[3] = currentHumidity % 10;
        }

        //Hardware multiplexing execution  switch updating the physical 7 segment lines
        switch (activeDigitSlot) {
            case 0:
                projectDigitToSlot(0, targetDigits[0]);
                activeDigitSlot = 1;
                break;

            case 1:
                projectDigitToSlot(1, targetDigits[1]);
                activeDigitSlot = 2;
                break;

            case 2:
                projectDigitToSlot(2, targetDigits[2]);
                activeDigitSlot = 3;
                break;

            case 3:
                projectDigitToSlot(3, targetDigits[3]);
                activeDigitSlot = 0;
                break;

        }

        //Block the task exactly 4 milliseconds to allow the other task to run
        vTaskDelay(xDelay4ms);
    }
}