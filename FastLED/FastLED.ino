#include <FastLED.h>

#define LED_PIN   7
#define DATA_PIN  7
#define NUM_LEDS  20

CRGB leds[NUM_LEDS];

void setup() {
//  FastLED.addLeds<WS2812, LED_PIN, GRB>(leds, NUM_LEDS);
//  leds[0] = CRGB::Blue;
//  FastLED.show();

  Serial.begin(57600);
  Serial.println("resetting");
  LEDS.addLeds<WS2812,DATA_PIN,RGB>(leds,NUM_LEDS);
  LEDS.setBrightness(84);
}

void fadeall() { for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } }

void loop() {
//  for (int x=0; x<NUM_LEDS; x++) {
//    leds[x] = CRGB::Red;
//    FastLED.show();
//    delay(40);
//  }
//
//  for (int x=NUM_LEDS - 1; x>=0; x--) {
//    leds[x] = CRGB(0, 0, 255);
//    FastLED.show();
//    delay(40);
//  }

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
  Serial.print("x");

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
}
