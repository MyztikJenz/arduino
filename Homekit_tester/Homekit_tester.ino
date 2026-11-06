#include <Arduino.h>
#include <arduino_homekit_server.h>
#include <ESP8266WiFi.h>
#include "wifi_credentials.h"

#define LED_PIN 2

// access your HomeKit characteristics defined in my_accessory.c
extern "C" homekit_server_config_t config;
extern "C" homekit_characteristic_t cha_switch_on;
extern "C" homekit_characteristic_t cha_brightness;
extern "C" homekit_characteristic_t cha_saturation;
extern "C" homekit_characteristic_t cha_hue;

static uint32_t next_heap_millis = 0;

void setup() {
    Serial.begin(115200);
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH);

    WiFi.begin(wifi_ssid, wifi_password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    
    Serial.println("");
    Serial.println("WiFi connected");  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Netmask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());

    //homekit_storage_reset();

    cha_switch_on.setter = cha_switch_on_setter;
    cha_hue.setter = cha_hue_setter;
    cha_brightness.setter = cha_brightness_setter;
    cha_saturation.setter = cha_saturation_setter;
    arduino_homekit_setup(&config);
}

void loop() {
    my_homekit_loop();
    delay(10);
}

void cha_switch_on_setter(const homekit_value_t value) {
    bool on = value.bool_value;
    cha_switch_on.value.bool_value = on;    //sync the value
    Serial.printf("Switch: %s\n", on ? "ON" : "OFF");
    digitalWrite(LED_PIN, on ? LOW : HIGH);
}

void cha_brightness_setter(const homekit_value_t value) {
    Serial.printf("brightness: %d\n", value.int_value);
}

void cha_saturation_setter(const homekit_value_t value) {
    Serial.printf("saturation: %f\n", value.float_value);
}

void cha_hue_setter(const homekit_value_t value) {
    Serial.printf("hue: %f\n", value.float_value);
}

void my_homekit_loop() {
    arduino_homekit_loop();
    const uint32_t t = millis();
    if (t > next_heap_millis) {
        // show heap info every 5 seconds
        next_heap_millis = t + 5 * 1000;
        Serial.printf("Free heap: %d, HomeKit clients: %d\n", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());

    }
}
