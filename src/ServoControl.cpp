#include "ServoControl.h"

void setupServoHardware() {
    //Setup LEDC timer channel for parameter natively inside the chip
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);

    //Bind the physical GPIO pin to the configured hardware channel
    ledcAttachPin(SERVO_PIN, PWM_CHANNEL);

    //Position servo at home state (0 degrees) during boot
    writeServoAngle(0);
}

void writeServoAngle(int angle) {
    //Constrain input to avoid physical over travel damages to internal gears
    int sanitizedAngle = constrain(angle, 0, 180);

    //Linear map from human degrees to 16 bit discrete timing ticks
    int ticks = map(sanitizedAngle, 0, 180, MIN_TICK, MAX_TICK);

    //Inject raw ticks directly into the hardware silicon registers
    ledcWrite(PWM_CHANNEL, ticks);
}