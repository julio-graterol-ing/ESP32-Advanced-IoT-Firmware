#ifndef DISPLAY_MULTIPLEX_H
#define DISPLAY_MULTIPLEX

#include <Arduino.h>

//Array definitions mappin GPIO pins representing segments pins (A, B, C, D, E, F, G, DP)
extern const uint8_t SEGMENT_PINS[8];

//Array definition mapping GPIO pins representing digit common cathodes (D1, D2, D3, D4)
extern const uint8_t DIGIT_PINS[4];

//configures all target GPIO lines to output mode for driving the display
void setupDisplayHardware();

//Clear all segments to blank out the active display digits
void clearDisplay();

//Render a raw numerical character array onto a single designated digit slot
void projectDigitToSlot(uint8_t digitIndex, uint8_t numberValue);

//FreeRtos task worker responsible for hihg priority display refreshing
void displayUpdateTask(void *parameter);

#endif //DisplayMultiplex_h