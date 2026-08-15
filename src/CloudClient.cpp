#include "CloudClient.h"
#include "secrets.h"       // Contains IFTTT_KEY, WIFI_SSID, WIFI_PASSWORD (gitignored)
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// IFTTT Maker Webhooks event name
const char* event_name = "temp_alert";

// Alert threshold and anti-spam cooldown timer
const int TEMP_THRESHOLD = 25;                 // Trigger alert above 30°C
unsigned long lastAlertMillis = 0;
const unsigned long COOLDOWN_INTERVAL = 60000; // Minimum 1 minute between alerts

void checkCloudAlerts(int temperature) {
    unsigned long currentMillis = millis();

    if (temperature > TEMP_THRESHOLD &&
        (currentMillis - lastAlertMillis >= COOLDOWN_INTERVAL || lastAlertMillis == 0)) {

        // IMPORTANT FIX: update the timestamp BEFORE attempting the request,
        // so failed attempts also respect the cooldown instead of retrying every second
        lastAlertMillis = currentMillis;

        if (WiFi.status() == WL_CONNECTED) {

            Serial.printf("[DEBUG] Free heap before TLS: %d bytes\n", ESP.getFreeHeap());

            IPAddress resolvedIP;
            if (WiFi.hostByName("maker.ifttt.com", resolvedIP)) {
                Serial.print("[DEBUG] DNS resolved to: ");
                Serial.println(resolvedIP);
            } else {
                Serial.println("[DEBUG] DNS resolution FAILED");
            }

            WiFiClientSecure client;
            client.setInsecure();
            client.setHandshakeTimeout(30); // give the handshake more time on unstable networks

            HTTPClient http;
            String url = "https://maker.ifttt.com/trigger/" + String(event_name) +
                         "/with/key/" + String(IFTTT_KEY);

            Serial.println("[CLOUD] Thermal threshold breached! Sending secure request...");

            http.begin(client, url);
            http.addHeader("Content-Type", "application/json");

            String jsonPayload = "{\"value1\":\"" + String(temperature) + "\"}";
            int httpResponseCode = http.POST(jsonPayload);

            if (httpResponseCode == 200) {
                Serial.print("[SUCCESS] Webhook dispatched. HTTP Code: ");
                Serial.println(httpResponseCode);
            } else if (httpResponseCode > 0) {
                Serial.print("[WARNING] Unexpected HTTP response code: ");
                Serial.println(httpResponseCode);
            } else {
                Serial.print("[ERROR] Request failed. Error: ");
                Serial.println(http.errorToString(httpResponseCode).c_str());
            }

            http.end();
        }
    }
}