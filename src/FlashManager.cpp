#include "FlashManager.h"

//instantiate the static preferences drives object
Preferences preferences;

void setupFlashStorage() {
    Serial.println("[SYSTEM] Initializing Non Volatile Storage (NVS) partition...");

    //Open the namespace called "station_nv" in read/write mode
    preferences.begin("station_nv", false);

    Serial.println("[SUCCES] NVS Flash Subsystem mounted successfully");
    preferences.end(); //Always close the partition handle when when handle when operation finishes

}

void saveClimateToFlash(int temp, int hum) {
    //Open storage channel in active Write mode
    preferences.begin("station_nv", false);

    //Write integer primitives to unique keys within the active partition block
    preferences.putInt("saved_temp", temp);
    preferences.putInt("saved_hum", hum);

    Serial.printf("[STORAGE_NV] Permanent Write Sucess -> Keys update: Temp=%d, Hum=%d\n", temp, hum);
    preferences.end(); //Fress physical Flash sectors from potential locking conditions

}

void loadClimateFromFlash(int *temp, int *hum) {
    //open storage partition in strict read only mode for safety
    preferences.begin("station_nv", true);

    //Read keys. if the flash is empty, return 20 and 50 as safety fallbacks
    *temp = preferences.getInt("saved_temp", 20);
    *hum = preferences.getInt("saved_hum", 50);

    Serial.printf("[STORAGE-NV] Initial Recovery -> Loaded from Flash: Temp:%d, Hum:%d\n", *temp, *hum);
    preferences.end();
}