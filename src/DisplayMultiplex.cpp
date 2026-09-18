#include "DisplayMultiplex.h"

//Mapping: [A=D23, B=D22, C=D19, D=D2, E=D21, F=TX2, G=RX2, DP=D5
const uint8_t SEGMENT_PINS[8] = {23, 22, 19, 2, 21, 17, 16, 5};

//Position sequence layout: [D1=D15, D2=D14, D3=D13, D4=D12]
const uint8_t DIGIT_PINS[4] = {15, 14, 13, 12};

//Binary truth table lookup mapping hex indices (0-9) into segment activation cambinations
const uint8_t SEVEN_SEGMENT_LUT[10] ={
    0b00111111, //0
    0b00000110, //1
    0b01011011, //2
    0b01001111, //3
    0b01100110, //4
    0b01101101, //5
    0b01111101, //6
    0b00000111, //7
    0b01111111, //8
    0b01101111  //9

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
    //Bounds guard validation check to protect memory stacks from illegal array drift
    if (digitIndex > 3 || numberValue >9) return;

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
    const TickType_t xDelay4ms = pdMS_TO_TICKS(4);

    //continous real time scheduling loop for Core 1 execution context
    for(;;) {
        switch (activeDigitSlot) {
            case 0:
                projectDigitToSlot(0, 1);
                activeDigitSlot = 1;
                break;

            case 1:
                projectDigitToSlot(1, 2);
                activeDigitSlot = 2;
                break;

            case 2:
                projectDigitToSlot(2, 3); 
                activeDigitSlot = 3;
                break;

            case 3:
                projectDigitToSlot(3, 4);
                activeDigitSlot = 0;
                break;

        }

        //Block the task exactly 4 milliseconds to allow the other task to run
        vTaskDelay(xDelay4ms);
    }
}