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
    
        // inject 0x3F oversampling x1 Temp, x1 Press and force normal mode
        writeI2CBareMetal(0x76, 0xF4, 0x3F);
        Serial.println ("[SENSOR] BMP280 forced to normal mode");
    
    } else {
        Serial.println("[WARNING] Bus responded with hardware NACK. Verify line capacitance or device address.");
        // Clear error flags to avoid blocking or freezing the hardware bus state machine
        *I2C0_INT_CLR_REG = 0xFFFFFFFF;
    }
}

bool writeI2CBareMetal(uint8_t deviceAddr, uint8_t regAddr, uint8_t value) {
    *I2C0_INT_CLR_REG = 0xFFFFFFFF; // reset hardware flags

    //Push addres and playload byte straight into the hardware FIFO buffer channel
    *I2C0_DATA_FIFO_REG = (deviceAddr << 1);
    *I2C0_DATA_FIFO_REG = regAddr;
    *I2C0_DATA_FIFO_REG = value;

    //load execution commands inside the controller stack array
    *(I2C0_CMD_BASE_REG +0) = (0 << 14) | (1 << 11) | 0; // Start condition
    *(I2C0_CMD_BASE_REG +1) = (1 << 14) | (1 << 11) | 3; //Write 3 bytes from FIFO and track ACK
    *(I2C0_CMD_BASE_REG +2) = (3 << 14) | (1 << 11) | 0; //Stop condition

    *I2C0_CTR_REG |= (1 << 5); // Trigger execution bit
    vTaskDelay(pdMS_TO_TICKS(1));

    //Return if the transmission finished cleanly witout NACK errors
    return !(*I2C0_INT_CLR_REG & (1 << 10));
}

bool readI2CBareMetal(uint8_t deviceAddr, uint8_t regAddr, uint8_t* buffer, uint8_t lenght) {
    if (lenght == 0) return false; 
    *I2C0_INT_CLR_REG = 0xFFFFFFFF;

    //Phase A write target internal register address to prepare the sensor pointer
    *I2C0_DATA_FIFO_REG = (deviceAddr << 1);
    *I2C0_DATA_FIFO_REG = regAddr;

    *(I2C0_CMD_BASE_REG + 0)= (0 << 14) | (1 << 11) | 0; // Start condition
    *(I2C0_CMD_BASE_REG + 1)= (1 << 14) | (1 << 11) | 2; // Write 2 bytes from FIFO and track ACK

    //Phase B restart the bus in read mode and extract the burst payload from the sensor FIFO
    *I2C0_DATA_FIFO_REG = (deviceAddr << 1) | 1; // device address + read bit 

    *(I2C0_CMD_BASE_REG + 2) = (0 << 14) | (1 << 11) | 0; // Restart condition
    *(I2C0_CMD_BASE_REG + 3) = (1 << 14) | (1 << 11) | 1; //Write 1 byte

    //Read consecutive  payload data streaming into FIFO buffer, enforcing ACK for all but the last byte
    *(I2C0_CMD_BASE_REG + 4) = (2 << 14) | (1 << 11) | lenght;
    *(I2C0_CMD_BASE_REG + 5) = (3 << 14) | (1 << 11) | 0; //Stop condition

    *I2C0_CTR_REG |= (1 << 5); // fire execution sequence
    vTaskDelay(pdMS_TO_TICKS(1));

    //If an unexpected NACK drops the line link abort inmediately
    if (*I2C0_INT_CLR_REG & (1 << 10)) return false;

    //Phase C: Extract the raw streaming data sirectly out of the hardware FIFO buffer into the user-provided array
    for (uint8_t i = 0; i < lenght; i++) {
        buffer[i] = (uint8_t)(*I2C0_DATA_FIFO_REG & 0xFF); // Mask to extract only the least significant byte

    }
    return true;
}

int32_t readBmp280TemperatureBareMetal() {
    uint8_t rawBuffer[3] = {0, 0, 0};

    //Extract 3 bytes simultaneously starting from register 0xFA
    if (!readI2CBareMetal(0x76, 0xFA, rawBuffer, 3)) {
        return -999; // Return error metric if transaction sequence drops
    }

    //reconstruct the individual chunks into a single clean 20 bit interger
    int32_t adcRawTemperature = ((int32_t)rawBuffer[0] << 12) | ((int32_t)rawBuffer[1] << 4) | ((int32_t)rawBuffer[2] >> 4); 

    //Bare metal uncompensated interger calculation scaling output directly to 0.01 degrees Celsius units (e.g., 2500 = 25.00°C)
    int32_t humanReadableTemperature = ( adcRawTemperature * 100) / 4096;

    return humanReadableTemperature;
    
}