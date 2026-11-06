#include <FastLED.h>

#define DATA_PIN  7
#define NUM_LEDS  240

CRGB leds[NUM_LEDS];


void setup() {
  FastLED.addLeds<WS2812B,DATA_PIN,GRB>(leds,NUM_LEDS);
  Serial.begin(56700);
  Serial.println("test");
  FastLED.clear();
}

void loop() {
  int potVal = analogRead(A0);
//  Serial.println(String(potVal));
//  delay(500);`

  int led = int(floor(potVal / 1023.0 * NUM_LEDS));

  fadeToBlackBy(leds, NUM_LEDS, 20);
  // Determine a hue based on the led we're on
  leds[led] = CHSV(int(float(led) / float(NUM_LEDS) * 255.0), 255, 192);
//  leds[led] = CRGB::Purple;
//  Serial.println(String(float(led) / float(NUM_LEDS)));
  FastLED.show();
  delay(5);
}
