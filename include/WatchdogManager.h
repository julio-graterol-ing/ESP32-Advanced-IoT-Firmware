#ifndef WATCHDOG_MANAGER_H
#define WATCHDOG_MANAGER_H

#include <Arduino.h>
#include <esp_task_wdt.h>

//defines the maximum allowed freeze window before forcing a hardware reset
#define WDT_TIMEOUT_SECONDS 5

// Configures the silicon level Task Watchdog timer for the system
void setupHardwareWatchdog();

//Reset the hardware timer counter to prevent CPU panic restarts
void feedHardwareWatchdog();

#endif // WATCHDOG_MANAGER_H


