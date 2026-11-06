#include <FastLED.h>

#define LED_PIN   5
#define DATA_PIN  5
#define POWER_RELAY_PIN 14
#define NUM_LEDS  10

CRGB leds[NUM_LEDS];

void setup() {
    Serial.begin(9600);
    Serial.println("resetting");
    LEDS.addLeds<WS2812,DATA_PIN,GRB>(leds,NUM_LEDS);
    fill_solid(leds, NUM_LEDS, CHSV(HUE_BLUE, 255, 255));
    LEDS.setBrightness(84);
    
    pinMode(POWER_RELAY_PIN, OUTPUT);
    digitalWrite(POWER_RELAY_PIN, LOW);
    delay(2000);
    digitalWrite(POWER_RELAY_PIN, HIGH);
    FastLED.show();
    delay(1000);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

uint8_t ctr = 0;
void loop() {
    Serial.println("loop");
//    fill_solid(leds, NUM_LEDS, CHSV(HUE_BLUE, 255, 255));
    FastLED.show();
    return;
    static uint8_t hue = 0;
    Serial.print("x");
    // First slide the led in one direction
    for(int i = 0; i < NUM_LEDS; i++) {
        // Set the i'th led to red 
        leds[i] = CHSV(hue++, 255, 255);
        // Show the leds
        FastLED.show(); 
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
        // Wait a little bit before we loop around and do it again
        delay(10);
    }

    // Now go in the other direction.  
    for(int i = (NUM_LEDS)-1; i >= 0; i--) {
        // Set the i'th led to red 
        leds[i] = CHSV(hue++, 255, 255);
        // Show the leds
        FastLED.show();
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
        // Wait a little bit before we loop around and do it again
        delay(10);
    }
    ctr++;
    if (ctr == 0) {
        Serial.println("");
    }
}
