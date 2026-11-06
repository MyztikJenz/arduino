#include <homekit/homekit.h>
#include <homekit/characteristics.h>

void my_accessory_identify(homekit_value_t _value) {
    Serial.printf("accessory identify\n");
}

// Switch (HAP section 8.38)
// required: ON
// optional: NAME

// format: bool; HAP section 9.70; write the .setter function to get the switch-event sent from iOS Home APP.
homekit_characteristic_t cha_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);

// format: string; HAP section 9.62; max length 64
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "Switch");

homekit_characteristic_t cha_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);
homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, 180);
homekit_characteristic_t cha_saturation= HOMEKIT_CHARACTERISTIC_(SATURATION, 50);



homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_switch, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "JimLightBulb"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "JimArduino HomeKit"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "X8675309"),
            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            &cha_switch_on,
            &cha_name,
            &cha_hue,
            &cha_brightness,
            &cha_saturation,
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
        .accessories = accessories,
        .password = "123-12-123"
};
