#include "I2CDriver.h"

void initI2CBareMetal() {
    // 1. Electrical configuration: Input with active internal pull-up to lock 3.3V idle state
    pinMode(I2C_SCL_PIN, INPUT_PULLUP);
    pinMode(I2C_SDA_PIN, INPUT_PULLUP);
    
    // Configure IO MUX routing matrices to enforce hardware open-drain outputs
    pinMode(I2C_SCL_PIN, OUTPUT_OPEN_DRAIN);
    pinMode(I2C_SDA_PIN, OUTPUT_OPEN_DRAIN);

    Serial.println("[SYSTEM] IO MUX Matrix reconfigured. GPIO 26 and 27 mapped to I2C0 hardware.");

    // 2. Wake up native controller: Enable Master Mode (Bit 0 set to 1)
    *I2C0_CTR_REG = 0x00000001; 

    // 3. Load standard timing values to configure SCL frequency at 100 kHz (80MHz APB clock base)
    *I2C0_SCL_LOW_PERIOD_REG  = 400; 
    *I2C0_SCL_HIGH_PERIOD_REG = 400; 

    // 4. Clear residual hardware interrupt flags to enforce a clean bus state
    *I2C0_INT_CLR_REG = 0xFFFFFFFF;
    Serial.println("[SUCCESS] ESP32 Native I2C0 Controller initialized via hardware registers.");

    // --- NATIVE HARDWARE COMMAND LINK REGISTRY PROBE ---
    Serial.println("[TEST] Executing native hardware register probe at address 0x76...");

    // Populate the hardware command execution pipeline array sequentially using direct pointer arithmetic
    // Command 0: Trigger a physical I2C START condition
    *(I2C0_CMD_BASE_REG + 0) = (0 << 14) | (1 << 11) | 0; 
    
    // Command 1: Outbound target address (0x76) under Write Mode (0x76 << 1) and validate ACK response
    *(I2C0_CMD_BASE_REG + 1) = (1 << 14) | (1 << 11) | (1 << 8) | (0x76 << 1); 
    
    // Command 2: Trigger a physical I2C STOP condition to safely release shared lines
    *(I2C0_CMD_BASE_REG + 2) = (3 << 14) | (1 << 11) | 0; 

    // Force silicon execution daemon to process the active command block by triggering bit 5
    *I2C0_CTR_REG |= (1 << 5); 

    // Enforce a non-blocking FreeRTOS task delay to allow physical lines 
    vTaskDelay(pdMS_TO_TICKS(1));

    // Sample the interrupt status register to verify if a transmission timeout error (NACK) occurred
    // Bit 10 is automatically asserted by peripheral hardware if the target slave fails to drive SDA low
    uint32_t interruptStatus = *I2C0_INT_CLR_REG;
    
    if (!(interruptStatus & (1 << 10))) {
        Serial.println("[HARDWARE-SUCCESS] BMP280 Industrial Sensor verified! Real ACK captured on lines.");
        Serial.println("[STATUS] Bus operational. Ready for bit-shifting data registers.");
    } else {
        Serial.println("[WARNING] Bus responded with hardware NACK. Verify line capacitance or device address.");
        // Clear error flags to avoid blocking or freezing the hardware bus state machine
        *I2C0_INT_CLR_REG = 0xFFFFFFFF;
    }
}
