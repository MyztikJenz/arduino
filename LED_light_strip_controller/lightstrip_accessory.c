#include <homekit/homekit.h>
#include <homekit/characteristics.h>
//#include <Serial.h>

void my_accessory_identify(homekit_value_t _value) {
    // Not sure what this does.
//    Serial.printf("accessory identify... not sure what this does yet.\n");
}

homekit_characteristic_t cha_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_name = HOMEKIT_CHARACTERISTIC_(NAME, "LED Light Strip");
homekit_characteristic_t cha_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);
homekit_characteristic_t cha_hue = HOMEKIT_CHARACTERISTIC_(HUE, 180);
homekit_characteristic_t cha_saturation= HOMEKIT_CHARACTERISTIC_(SATURATION, 50);

// Names and brightness are set at runtime.
homekit_characteristic_t cha_trails_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_trails_name = HOMEKIT_CHARACTERISTIC_(NAME, "_trails_");
homekit_characteristic_t cha_trails_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_rainbow_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_rainbow_name = HOMEKIT_CHARACTERISTIC_(NAME, "_rainbow_");
homekit_characteristic_t cha_rainbow_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_rainbow_glitter_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_rainbow_glitter_name = HOMEKIT_CHARACTERISTIC_(NAME, "_rainbowglitter_");
homekit_characteristic_t cha_rainbow_glitter_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_confetti_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_confetti_name = HOMEKIT_CHARACTERISTIC_(NAME, "_confetti_");
homekit_characteristic_t cha_confetti_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_sinelon_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_sinelon_name = HOMEKIT_CHARACTERISTIC_(NAME, "_sinelon_");
homekit_characteristic_t cha_sinelon_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_bpm_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_bpm_name = HOMEKIT_CHARACTERISTIC_(NAME, "_bpm_");
homekit_characteristic_t cha_bpm_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_characteristic_t cha_juggle_switch_on = HOMEKIT_CHARACTERISTIC_(ON, false);
homekit_characteristic_t cha_juggle_name = HOMEKIT_CHARACTERISTIC_(NAME, "_juggle_");
homekit_characteristic_t cha_juggle_brightness = HOMEKIT_CHARACTERISTIC_(BRIGHTNESS, 20);

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "LED Light Strip"),
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Jim!"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "Jenny-8675309"),
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
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_trails_switch_on,
            &cha_trails_name,
            &cha_trails_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_rainbow_switch_on,
            &cha_rainbow_name,
            &cha_rainbow_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_rainbow_glitter_switch_on,
            &cha_rainbow_glitter_name,
            &cha_rainbow_glitter_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_confetti_switch_on,
            &cha_confetti_name,
            &cha_confetti_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_sinelon_switch_on,
            &cha_sinelon_name,
            &cha_sinelon_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_bpm_switch_on,
            &cha_bpm_name,
            &cha_bpm_brightness,
            NULL
        }),
        HOMEKIT_SERVICE(LIGHTBULB, .primary=false, .characteristics=(homekit_characteristic_t*[]){
            &cha_juggle_switch_on,
            &cha_juggle_name,
            &cha_juggle_brightness,
            NULL
        }),
        NULL
    }),
//    HOMEKIT_ACCESSORY(.id=2, .category=homekit_accessory_category_lightbulb, .services=(homekit_service_t*[]) {
//        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
//            HOMEKIT_CHARACTERISTIC(NAME, "Rainbow Effect2"),
//            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "Jim!"),
//            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "Jenny-8675309"),
//            HOMEKIT_CHARACTERISTIC(MODEL, "ESP8266"),
//            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "1.0"),
//            HOMEKIT_CHARACTERISTIC(IDENTIFY, my_accessory_identify),
//            NULL
//        }),
//        HOMEKIT_SERVICE(LIGHTBULB, .primary=true, .characteristics=(homekit_characteristic_t*[]){
//            &cha_rainbow_switch_on,
//            &cha_rainbow_name,
//            &cha_rainbow_brightness,
//            NULL
//        }),
//        NULL
//    }),
    NULL
};

homekit_server_config_t homekit_config = {
        .accessories = accessories,
        .password = "123-12-123"
};
