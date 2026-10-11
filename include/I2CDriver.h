#ifndef I2C_DRIVER_H
#define I2C_DRIVER_H

#include <Arduino.h>

// Physical IO MUX pin allocation for the independent I2C bus
#define I2C_SCL_PIN 26
#define I2C_SDA_PIN 27

// Bare-metal hardware register base mapping for ESP32 I2C0 peripheral
#define DR_REG_I2C_BASE          0x3ff53000
#define I2C0_SCL_LOW_PERIOD_REG  ((volatile uint32_t*)(DR_REG_I2C_BASE + 0x0000))
#define I2C0_SCL_HIGH_PERIOD_REG ((volatile uint32_t*)(DR_REG_I2C_BASE + 0x0004))
#define I2C0_CTR_REG             ((volatile uint32_t*)(DR_REG_I2C_BASE + 0x001C))
#define I2C0_INT_CLR_REG         ((volatile uint32_t*)(DR_REG_I2C_BASE + 0x0024))
#define I2C0_CMD_BASE_REG        ((volatile uint32_t*)(DR_REG_I2C_BASE + 0x005C))

/**
 * @brief Configures native hardware registers to initialize the I2C bus at 100kHz.
 * Executes a hardware-level register probe to detect the industrial sensor.
 */
void initI2CBareMetal();

#endif // I2C_DRIVER_H
