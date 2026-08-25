#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include "WatchdogManager.h"


extern "C" void esp_task_wdt_isr_user_handler(void) {
    esp_restart();
}

void setupHardwareWatchdog() {
    Serial.println("[SYSTEM] Initializing Hardware Task Watchdog Timer...");

    //Initialize the legacy TWDT peripheral: 5 seconds timeout, trigger hard panic on expiration
    esp_err_t err = esp_task_wdt_init(WDT_TIMEOUT_SECONDS, false);

    if (err == ESP_OK) { 
        //Subscribe the current main loop execution thread core 1 to the watch group
        esp_task_wdt_add(NULL);
        Serial.println("[SUCCESS] Hardware Watchdog armed. 5 seconds safety net Activate"); 
    } else {
        Serial.println("[CRITICAL ERROR] Failed to initialize hardware Watchdog peripheral");

    }
}

void feedHardwareWatchdog() {
    //Reset the contdown register back to zero ticks
    esp_task_wdt_reset();
}