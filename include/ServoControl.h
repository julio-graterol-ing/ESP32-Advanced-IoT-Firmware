#ifndef SERVO_CONTROL_H
#define SERVO_CONTROL_H

#include <Arduino.h>

//hardware LEDC configuration parameter
#define SERVO_PIN 18 //GPIO assigned to the servo PWM signal
#define PWM_CHANNEL 0 //Selct internal hardware LEDC channel 0
#define PWM_FREQ 50 // Standard analog servo frame rate 50Hz
#define PWM_RESOLUTION 16 //16 bit register timer resolution 0 to 65535 discrete steps

//Calibrated 16 bit timing ticks for SG90 kinematic limits 
#define MIN_TICK 1638 // 0.5ms pulse (0degrees) 
#define MAX_TICK 8192 //2.5ms pulse ( 180 degrees)

void setupServoHardware();

void writeServoAngle(int angle);

#endif //Servo_Control_H
