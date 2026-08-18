#include "MqttClient.h"
#include <WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include "ServoControl.h" //Required to inject received angles into hardware layer
#include "secrets.h"

extern bool remoteControlActive; //define in main.cpp

//Official tier Adafruit IO host configuration
const char* mqtt_server = "io.adafruit.com";
const int mqtt_port = 1883; //Satandard http/tcp port without heavy encryption to optimize RAM

//Instatie the native network client and Adafruit broker interface
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, mqtt_server, mqtt_port, AIO_USERNAME, AIO_KEY);

//Rubtime buffer to hold the constructed feed paths
static char temperatureFeedPath[64];
static char servoFeedPath[64];

//Outbound/Inbound feed pointers built at runtime inside SetupMQTT
Adafruit_MQTT_Publish* temp_feed = nullptr;
Adafruit_MQTT_Subscribe* servo_feed = nullptr;

//Antispa, scheduling guard for cloud publication
unsigned long lastMqttPublish = 0;
const unsigned long MQTT_PUBLISH_INTERVAL = 20000; //Account constraint: 20 seconds minimum

//Internal routine to handle secure and non blocking broker reconnection sequences
void connectMQTT() {
    if (mqtt.connected()) return;

    Serial.print("[MQTT] Connecting to Adafruit IO Broker...");
    int8_t ret;
    int reattempts = 0;
    
    while ((ret = mqtt.connect()) != 0 && reattempts < 3) { //maximum 3 attempt to prevent loop freezing
        Serial.println(mqtt.connectErrorString(ret));
        Serial.println("[MQTT] Retrying socket connection in 2 seconds...");
        mqtt.disconnect();
        delay(2000);
        reattempts++;
    }

    if (mqtt.connected()) {
        Serial.println("\n[SUCCESS] Connected to Adafruit IO Broker!");
    } else {
        Serial.println("[WARNING] MQTT Broker unreachable. Continuing offline mode ");

    }
}

void setupMQTT() {
    //Build the feed paths at runtime now that AIO_USERNAME holds a real value
    snprintf(temperatureFeedPath, sizeof(temperatureFeedPath), "%s/feeds/temperature", AIO_USERNAME);
    snprintf(servoFeedPath, sizeof(servoFeedPath), "%s/feeds/servo_control", AIO_USERNAME);

    temp_feed = new Adafruit_MQTT_Publish(&mqtt, temperatureFeedPath);
    servo_feed =new Adafruit_MQTT_Subscribe(&mqtt, servoFeedPath);

    //Bind the callback function upon inbound cloud packet arrivals
    servo_feed->setCallback([](uint32_t angle) {
        Serial.printf("[CLOUD MQTT] New angular directive received: %d deg\n", angle);
        if (angle >= 0 && angle <= 180) {
            remoteControlActive = true; //Switch to manual mode: stop the automatic sweep
            writeServoAngle(angle); // Move physical servo actuator in real time

        }
    });

    //Register the subscription feed within the broker listening daemon
    mqtt.subscribe(servo_feed);
    connectMQTT();
}

void maintainMQTT() {
    connectMQTT();

    //Asynchronous listener daemon: check for inbound packets within a 10ms window
    mqtt.processPackets(10);

    //Background keep alive ping to prevent adafruit server from closing inactive sockets
    if (millis() - lastMqttPublish > 30000) {
        if (!mqtt.ping()) {
            mqtt.disconnect();
        } 
    }
}

void publishTemperature(int temperature) {
    if (!mqtt.connected() || temp_feed == nullptr) return;

    unsigned long currentMillis = millis();
    if (currentMillis - lastMqttPublish >= MQTT_PUBLISH_INTERVAL || lastMqttPublish == 0) {
        lastMqttPublish = currentMillis;

        Serial.printf("[MQTT] Dispatched telemetry -> Temp %d C\n", temperature);
        if (!temp_feed->publish(temperature)) {
            Serial.println("[ERROR] MQTT telemetry packet dropped by broker");
        }
    }
}
