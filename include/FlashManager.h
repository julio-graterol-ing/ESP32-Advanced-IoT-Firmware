#ifndef FLASH_MANAGER_H
#define FLASH_MANAGER_H

#include <Arduino.h>
#include <Preferences.h>

//Initializes the low level NVS flash memory partition
void setupFlashStorage();

//Save climate metrics inside NVS keys only if data has drifted from previous state
void saveClimateToFlash(int temp, int hum);

// Retrieves previously logged climate metrics from the non volatile storage
void loadClimateFromFlash(int *temp, int *hum);

#endif // FLASH_MANAGER_H

