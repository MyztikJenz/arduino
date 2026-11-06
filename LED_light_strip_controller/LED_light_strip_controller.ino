#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FastLED.h>
#include <EEPROM.h>
#include <arduino_homekit_server.h>
#include "wifi_credentials.h"

// 
// LED Light Strip controller doodad
//
// A Note: EEPROM management for ESP devices is different than just normal Arduino. There's begin/commit/end and it appears 
//         that they blit the entire size given to begin() each time commit() is called. It's a bit weird.
//
////////////////////////////////////////////////////////////////////////////////////////////////
#define DEBUG 1
#define LOG_EFFECT 0 // Set to 1 to see effect names as they're called. It's going to slow things down though...

#define LED_PIN         5
#define POWER_RELAY_PIN 14
#define PAIR_MODE_PIN   16
#define NUM_LEDS        10

#define RUN_WEB_SERVER  0
#if RUN_WEB_SERVER
#define SERVICE_PORT    1234
WiFiServer server(SERVICE_PORT);
#endif


CRGB leds[NUM_LEDS];
CRGB currentColor = CRGB::White;
CHSV currentHSVColor = CHSV(HUE_BLUE, 255, 255);
#define MAX_CURRENT_EFFECT_LENGTH   64
char currentEffect[MAX_CURRENT_EFFECT_LENGTH] = "none";
uint8_t brightness = 128;
uint8_t effectHue = 0; // rotating base color used by effects
void (*currentEffectFunc)(void) = NULL;
#define RESET_STORED_VALUES 0   // Set to 1 to reset EEPROM values to the standard (defined above)

// Stuff for ZeroConf/Bonjour
#if RUN_WEB_SERVER
bool bHostDomainConfirmed = false;
char *pcHostDomain = NULL;
MDNSResponder::hMDNSService hMDNSService = NULL;
MDNSResponder::hMDNSServiceQuery hMDNSServiceQuery = NULL;
const char *serviceIdent = "ledicious";
#endif

// EEPROM addresses
int addr_offset          = 100;               // Someone (probably the HomeKit stuff) is scribbling in the lower memory addresses
int addr_savedColor      = addr_offset + 0;   // 3 bytes
int addr_savedBrightness = addr_offset + 3;   // 1 byte
int addr_savedEffect     = addr_offset + 4;   // MAX_CURRENT_EFFECT_LENGTH bytes (64)

// HomeKit characteristics defined in lightstrip_accessory.c
extern "C" homekit_server_config_t homekit_config;
extern "C" homekit_characteristic_t cha_switch_on;
extern "C" homekit_characteristic_t cha_brightness;
extern "C" homekit_characteristic_t cha_saturation;
extern "C" homekit_characteristic_t cha_hue;

extern "C" homekit_characteristic_t cha_rainbow_switch_on;
extern "C" homekit_characteristic_t cha_rainbow_brightness;
static bool inPairingMode = false;  // Hold the pairing button during restart to enter pairing mode (deletes existing pairs, overclocks CPU)
#define REPORT_HOMEKIT_USAGE    0   // Set to 1 to see periodic HomeKit stats

// The message that's returned from Command processing when connected via the App
#if RUN_WEB_SERVER
#define MAX_MSG_SIZE 256
typedef struct t_results {
     char msg[MAX_MSG_SIZE];
     int code;
} CmdResults;
#endif

void setup() {
    Serial.begin(9600);
    delay(100);
    Serial.println(F("Well, hello there..."));

    pinMode(POWER_RELAY_PIN, OUTPUT);
    digitalWrite(POWER_RELAY_PIN, LOW);
    cha_switch_on.value.bool_value = false;

    pinMode(PAIR_MODE_PIN, INPUT_PULLUP);
    
    inPairingMode = (digitalRead(PAIR_MODE_PIN) == HIGH);
    
    EEPROM.begin(1024);

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

#if RUN_WEB_SERVER
    server.begin();

    MDNS.setHostProbeResultCallback(hostProbeCallback);
    if (!MDNSResponder::indexDomain(pcHostDomain, 0, "arduino_1") || !MDNS.begin(pcHostDomain)) { 
        Serial.println("Crud... mDNS did not start");
    }
    else {
        Serial.println("mDNS should be working");
    }
#endif

#if RESET_STORED_VALUES > 0
    storeCurrentColor();
    storeBrightness();
    storeEffect();
#else
    // Load any values from memory
    loadCurrentColor();
    loadBrightness();
    loadEffect();
#endif
    setEffectFuncPtr();

    FastLED.addLeds<WS2812,LED_PIN,GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 1000);
    fill_solid(leds, NUM_LEDS, currentHSVColor);
    FastLED.setBrightness(brightness);

    my_homekit_setup();
}

unsigned long lastValue = 0;
unsigned long testValue = 0;
uint8_t brightnessAdjustment = 5;
char str[128];
bool isOn = true;

void logColor(CRGB c) {
    sprintf(str, "Current color: 0x%02X%02X%02X", c.red, c.green, c.blue);
    Serial.println(str);
}

void loadCurrentColor() { mem_currentColor(false); }
void storeCurrentColor() { mem_currentColor(true); }
void mem_currentColor(bool saving) {
    uint8_t hue = EEPROM.read(addr_savedColor);
    uint8_t saturation = EEPROM.read(addr_savedColor+1);
//    uint8_t b = EEPROM.read(addr_savedColor+2);

    if (saving) {
        bool commit = false;
        if (hue != currentHSVColor.hue)               { EEPROM.write(addr_savedColor, currentHSVColor.hue); commit = true; }
        if (saturation != currentHSVColor.saturation) { EEPROM.write(addr_savedColor+1, currentHSVColor.saturation); commit = true; }
//        if (b != currentColor.blue)  { EEPROM.write(addr_savedColor+2, currentColor.blue); commit = true; }

        if (commit) {
            EEPROM.commit();
        }

        uint8_t _h = EEPROM.read(addr_savedColor);
        uint8_t _s = EEPROM.read(addr_savedColor+1);
        Serial.printf("memcheck color: commit:(%s) hue(%hhu) saturation(%hhu)\n", commit ? "true" : "false", _h, _s);
    }
    else {
#if DEBUG
        Serial.printf("Loaded color hue(%hhu) saturation(%hhu) from memory\n", hue, saturation);
#endif
        currentHSVColor = CHSV(hue, saturation, brightness);
    }
}

void loadBrightness() { mem_brightness(false); }
void storeBrightness() { mem_brightness(true); }
void mem_brightness(bool saving) {
    uint8_t b = EEPROM.read(addr_savedBrightness);
    if (saving) {
        if (b != brightness) { 
            EEPROM.write(addr_savedBrightness, brightness);
            EEPROM.commit();
        }

        uint8_t x = EEPROM.read(addr_savedBrightness);
        Serial.printf("memcheck brightness: %hhu\n", x);
    }
    else {
#if DEBUG
        Serial.printf("Loaded brightness %hhu from memory\n", b);
#endif
        brightness = b;
    }
}

void loadEffect() { mem_effect(false); }
void storeEffect() { mem_effect(true); }
void mem_effect(bool saving) {
    char e[MAX_CURRENT_EFFECT_LENGTH];
    bzero(e, MAX_CURRENT_EFFECT_LENGTH);
    EEPROM.get(addr_savedEffect, e);
    if (saving) {
        if (strncmp(currentEffect, e, MAX_CURRENT_EFFECT_LENGTH) != 0) {
            EEPROM.put(addr_savedEffect, currentEffect);
            EEPROM.commit();
        }

//        bzero(e, MAX_CURRENT_EFFECT_LENGTH);
//        EEPROM.get(addr_savedEffect, e);
//        Serial.printf("memcheck effect name: [%s]\n", e);
    }
    else {
#if DEBUG
        Serial.printf("Loaded currentEffect %s from memory\n", e);
#endif
        strncpy(currentEffect, e, MAX_CURRENT_EFFECT_LENGTH);
    }
}

void updateEffects() {
    EVERY_N_MILLISECONDS(20) {
        effectHue++;
    }
    if (currentEffectFunc != NULL) {
        (*currentEffectFunc)();
    }
    else {
        if (cha_switch_on.value.bool_value == true) {
            FastLED.show();
        }
    }
}

void loop() {
#if RUN_WEB_SERVER
    MDNS.update();
#endif
    my_homekit_loop();
    
#if RUN_WEB_SERVER
    WiFiClient client = server.available();
    if (client) {
        Serial.println("[client connected]");
        while (client.connected()) {
            updateEffects();
            if (client.available()) {
                String cLine = client.readStringUntil('\r');
                cLine.trim();
#if DEBUG
                Serial.printf("received command: %s\n", cLine.c_str());
#endif
                CmdResults *r = commandProcessor(cLine);

#if DEBUG
                Serial.printf("code: %d msg: %s\n", r->code, r->msg);
#endif
                
                if (r->code == 1) { // end of commands from the client 
                    client.println(">thanks for coming!");
                    break;
                }
                else {
                    client.printf(">%s\n", r->msg);
//                    client.print(">");client.println(r->msg);
                }
                free(r);
            }
        }
        delay(10);
        client.stop();
        Serial.println("[client disconnected]");
    }
#endif

    // If no client is connected, we're just handling the lights.
    updateEffects();
    
    delay(50);
}

void setEffectFuncPtr() {
    if (strcmp(currentEffect, "none") == 0)                 { currentEffectFunc = NULL; }
    else if (strcmp(currentEffect, "trails") == 0)          { currentEffectFunc = &trails; }
    else if (strcmp(currentEffect, "rainbow") == 0)         { currentEffectFunc = &rainbow; } 
    else if (strcmp(currentEffect, "rainbowGlitter") == 0)  { currentEffectFunc = &rainbowWithGlitter; } 
    else if (strcmp(currentEffect, "confetti") == 0)        { currentEffectFunc = &confetti; } 
    else if (strcmp(currentEffect, "sinelon") == 0)         { currentEffectFunc = &sinelon; } 
    else if (strcmp(currentEffect, "bpm") == 0)             { currentEffectFunc = &bpm; } 
    else if (strcmp(currentEffect, "juggle") == 0)          { currentEffectFunc = &juggle; }
    else { Serial.printf("Unknown effect [%s] in setEffectFuncPtr\n", currentEffect); }
}

#if RUN_WEB_SERVER
#define SET_OK  sprintf(r->msg, "ok")
CmdResults *commandProcessor(String cmd) {
    CmdResults *r = (CmdResults *)malloc(sizeof(CmdResults));
    bzero(r, sizeof(CmdResults));
    if (cmd == "ping") {
        sprintf(r->msg, "pong");
        r->code = 0;
    }
    else if (cmd == "currentConfig") {
        sprintf(r->msg, "currentConfig:0x%02X%02X%02X|%s|%hhu", currentColor.red, currentColor.green, currentColor.blue, currentEffect, brightness);
        r->code = 0;
    }
    else if (cmd == "") { // End the connection
//        r->code = 1;
    }
    else if (cmd.startsWith("setColor:")) {
        r->code = 0;
        SET_OK;
        String hexColor = cmd.substring(9);
        int n = (int)strtol(hexColor.c_str(), NULL, 16);
        currentColor = CRGB(n);
        logColor(currentColor);
        storeCurrentColor();
        fill_solid(leds, NUM_LEDS, currentColor);
        FastLED.show();
    }
    else if (cmd.startsWith("effect:")) {
        r->code = 0;
        String effectName = cmd.substring(7);
        Serial.println(effectName);
        bzero(currentEffect, MAX_CURRENT_EFFECT_LENGTH);
        sprintf(currentEffect, effectName.c_str());
        storeEffect();
        setEffectFuncPtr();
        SET_OK;        
    }
    else if (cmd.startsWith("brightness:")) {
        if (cmd.charAt(11) == '+') {
            uint8_t limit = 255 - brightness;
            brightness += (limit >= brightnessAdjustment) ? brightnessAdjustment : limit;
        }
        else if (cmd.charAt(11) == '-') {
            brightness -= (brightness >= brightnessAdjustment) ? brightnessAdjustment : brightness;
        }
        else {
            String newBrightness = cmd.substring(11);
            int n = (int)strtol(newBrightness.c_str(), NULL, 10);
            brightness = (uint8_t)min(max(n, 0), 255);
        }
        storeBrightness();
        FastLED.setBrightness(brightness);
        FastLED.show();

        r->code = 0;
        SET_OK;
    }
    else {
        sprintf(r->msg, "Unknown command: [%s]", cmd.c_str());
        r->code = 999;
    }

    return r;
}
#endif

//
// HomeKit
//
///////////////////////////////////////////////////////////////////////////
#if REPORT_HOMEKIT_USAGE
static uint32_t next_heap_millis = 0;
#endif

void my_homekit_loop() {
    arduino_homekit_loop();
#if REPORT_HOMEKIT_USAGE
    const uint32_t t = millis();
    if (t > next_heap_millis) {
        // show heap info every 5 seconds
        next_heap_millis = t + 5 * 1000;
        Serial.printf("Free heap: %d, HomeKit clients: %d\n", ESP.getFreeHeap(), arduino_homekit_connected_clients_count());
    }
#endif
}

void my_homekit_setup() {
    if (inPairingMode) {
        Serial.println("### Restting HomeKit Storage ###");
        homekit_storage_reset();
    }

    cha_hue.setter = cha_hue_setter;
    cha_switch_on.setter = cha_switch_on_setter;
    cha_brightness.setter = cha_brightness_setter;
    cha_saturation.setter = cha_saturation_setter;

    cha_rainbow_switch_on.setter = cha_rainbow_switch_on_setter;
    cha_rainbow_brightness.setter = cha_rainbow_brightness_setter;

    // Set the HomeKit Characteristics to their defaults as restored from memory. HomeKit uses this to update it's UI.
    cha_brightness.value.int_value = (brightness * 100 / 255);
    cha_saturation.value.float_value = (float)(currentHSVColor.saturation * 100.0 / 255.0);
    cha_hue.value.int_value = (uint8_t)(currentHSVColor.hue * 360.0 / 255.0);

    arduino_homekit_setup(&homekit_config);

    // Finally figured out why the LEDs were just not working right. `arduino_homekit_setup` sets the clock speed to 160Mhz and
    // that messes everything up. Resetting back to 80Mhz gets everyone working again (although the HomeKit routines do appear
    // much slower now).
    if (!inPairingMode) {
        system_update_cpu_freq(SYS_CPU_80MHZ);
    }
}

void cha_switch_on_setter(const homekit_value_t value) {
    bool onValue = value.bool_value;
    cha_switch_on.value.bool_value = onValue;
#if DEBUG
    Serial.printf("Switch: %s\n", onValue ? "ON" : "OFF");
#endif

    // If an effect is enabled, turn it off
    if (strcmp(currentEffect, "none") != 0) {
        strncpy(currentEffect, "none", MAX_CURRENT_EFFECT_LENGTH);
    }

    digitalWrite(POWER_RELAY_PIN, onValue ? HIGH : LOW);
}

// Brightness is 0-100
void cha_brightness_setter(const homekit_value_t value) {
    int b = value.int_value;
#if DEBUG
    Serial.printf("brightness: %d\n", b);
#endif

    cha_brightness.value.int_value = b;
    brightness = (uint8_t)ceilf(b * 255.0 / 100.0);
    storeBrightness();
    currentHSVColor.value = brightness;
    FastLED.setBrightness(brightness);
}

// Hue and saturation always come in pairs (hue is currently first). Probably could get away with just calling fill_solid() in both callbacks,
// but we can probably do better than that just in case the order flips down the road someday.
// Hmmm... this assumption of the hue/saturation pairs may not be accurate. I've seen saturation change without hue.
typedef struct {
    unsigned int hue_called : 1;
    unsigned int saturation_called : 1;
} CallbackRegister;
CallbackRegister cbr;

void _colorSetter() {
    if (cbr.hue_called && cbr.saturation_called) {
        cbr.hue_called = false;
        cbr.saturation_called = false;

        Serial.println("updating color...");
        storeCurrentColor();
        fill_solid(leds, NUM_LEDS, currentHSVColor);
    }
}

// Saturation is 0-100
// And for some reason, this comes in as a float
void cha_saturation_setter(const homekit_value_t value) {
    float s = value.float_value;
#if DEBUG
    Serial.printf("saturation: %f\n", s);
#endif

    cbr.saturation_called = true;
    cha_saturation.value.float_value = s;
    uint8_t newSaturation = (uint8_t)ceilf(s * 255.0 / 100.0);
    currentHSVColor.saturation = newSaturation;
    _colorSetter();
}

// Hue is 0-360
void cha_hue_setter(const homekit_value_t value) {
    int h = (int)value.float_value;
#if DEBUG
    Serial.printf("hue: %d\n", h);
#endif

    cbr.hue_called = true;
    cha_hue.value.int_value = h;
    uint8_t newHue = (uint8_t)(h * 255 / 360);
    currentHSVColor.hue = newHue;
    _colorSetter();
}


// Effects test
void cha_rainbow_switch_on_setter(const homekit_value_t value) {
    bool onValue = value.bool_value;
    cha_rainbow_switch_on.value.bool_value = onValue;
    digitalWrite(POWER_RELAY_PIN, onValue ? HIGH : LOW);

    strncpy(currentEffect, onValue ? "rainbow" : "none", MAX_CURRENT_EFFECT_LENGTH);
    setEffectFuncPtr();
}

// Brightness is 0-100
void cha_rainbow_brightness_setter(const homekit_value_t value) {
    int b = value.int_value;

    cha_rainbow_brightness.value.int_value = b;
    uint8_t effectBrightness = (uint8_t)ceilf(b * 255.0 / 100.0);
    FastLED.setBrightness(effectBrightness);
}


//
// Effects
//
// Reminder - All effects must handle calling FastLED.show() themselves.
///////////////////////////////////////////////////////////////////////////
void fadeall() { 
    for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } 
}

void __logEffect(const char *funcName) {
#if LOG_EFFECT
    Serial.printf("%s\n", funcName);
#endif
    
}

void trails() {
    __logEffect(__func__);
    // First slide the led in one direction
    for(int i = 0; i < NUM_LEDS; i++) {
        // Set the i'th led to red 
        leds[i] = CHSV(effectHue, 255, 255);
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
        leds[i] = CHSV(effectHue, 255, 255);
        // Show the leds
        FastLED.show();
        // now that we've shown the leds, reset the i'th led to black
        // leds[i] = CRGB::Black;
        fadeall();
        // Wait a little bit before we loop around and do it again
        delay(10);
    }
}

void rainbow() {
    __logEffect(__func__);
    fill_rainbow(leds, NUM_LEDS, effectHue, 7);
    FastLED.show();
}

void rainbowWithGlitter() {
    __logEffect(__func__);
    rainbow();
    _addGlitter(80);
}

void _addGlitter(fract8 chanceOfGlitter) {
    if( random8() < chanceOfGlitter) {
        leds[ random16(NUM_LEDS) ] += CRGB::White;
    }
}

void confetti() {
    __logEffect(__func__);
    // random colored speckles that blink in and fade smoothly
    fadeToBlackBy(leds, NUM_LEDS, 10);
    int pos = random16(NUM_LEDS);
    leds[pos] += CHSV(effectHue + random8(64), 200, 255);
}

void sinelon() {
    __logEffect(__func__);
    // a colored dot sweeping back and forth, with fading trails
    fadeToBlackBy(leds, NUM_LEDS, 20);
    int pos = beatsin16(13, 0, NUM_LEDS-1);
    leds[pos] += CHSV(effectHue, 255, 192);
}

void bpm() {
    __logEffect(__func__);
    // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
    uint8_t BeatsPerMinute = 62;
    CRGBPalette16 palette = PartyColors_p;
    uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
    for(int i=0; i<NUM_LEDS; i++) { //9948
        leds[i] = ColorFromPalette(palette, effectHue+(i*2), beat-effectHue+(i*10));
    }
}

void juggle() {
    __logEffect(__func__);
    // eight colored dots, weaving in and out of sync with each other
    fadeToBlackBy(leds, NUM_LEDS, 20);
    byte dothue = 0;
    for(int i=0; i<8; i++) {
        leds[beatsin16(i+7, 0, NUM_LEDS-1 )] |= CHSV(dothue, 200, 255);
        dothue += 32;
    }
}


//
// ZeroConf/Bonjour things
//
///////////////////////////////////////////////////////////////////////////
#if RUN_WEB_SERVER
void serviceProbeResult(String p_pcServiceName, const MDNSResponder::hMDNSService p_hMDNSService, bool p_bProbeResult) {
    (void) p_hMDNSService;
    Serial.printf("MDNSServiceProbeResultCallback: Service '%s' probe %s\n", p_pcServiceName.c_str(), (p_bProbeResult ? "succeeded." : "failed!"));
}


void hostProbeCallback(String p_pcDomainName, bool p_bProbeResult) {

  Serial.printf("MDNSHostProbeResultCallback: Host domain '%s.local' is %s\n", p_pcDomainName.c_str(), (p_bProbeResult ? "free" : "already USED!"));

  if (true == p_bProbeResult) {
    // Set station hostname
//    setStationHostname(pcHostDomain);

    if (!bHostDomainConfirmed) {
      // Hostname free -> setup clock service
      bHostDomainConfirmed = true;

      if (!hMDNSService) {
        // Add a 'http.tcp' service to port 'SERVICE_PORT', using the host domain as instance domain
        hMDNSService = MDNS.addService(0, serviceIdent, "tcp", SERVICE_PORT);
        if (hMDNSService) {
              MDNS.setServiceProbeResultCallback(hMDNSService, serviceProbeResult);
        
              // Add some '_http._tcp' protocol specific MDNS service TXT items
              // See: http://www.dns-sd.org/txtrecords.html#http
//            MDNS.addServiceTxt(hMDNSService, "user", "");
//            MDNS.addServiceTxt(hMDNSService, "password", "");
//            MDNS.addServiceTxt(hMDNSService, "path", "/");
        }
        
        // Install dynamic 'http.tcp' service query
        if (!hMDNSServiceQuery) {
            hMDNSServiceQuery = MDNS.installServiceQuery(serviceIdent, "tcp", MDNSServiceQueryCallback);
            if (hMDNSServiceQuery) {
                Serial.printf("MDNSProbeResultCallback: Service query for '%s.tcp' services installed.\n", serviceIdent);
            } else {
                Serial.printf("MDNSProbeResultCallback: FAILED to install service query for 'http.tcp' services!\n");
            }
        }
      }
    }
  }
  else 
  {
    // Change hostname, use '-' as divider between base name and index
    if (MDNSResponder::indexDomain(pcHostDomain, "-", 0)) {
        MDNS.setHostname(pcHostDomain);
    } else {
        Serial.println("MDNSProbeResultCallback: FAILED to update hostname!");
    }
  }
}


void MDNSServiceQueryCallback(MDNSResponder::MDNSServiceInfo serviceInfo, MDNSResponder::AnswerType answerType, bool p_bSetContent) {
  String answerInfo;
  switch (answerType) {
    case MDNSResponder::AnswerType::ServiceDomain :
      answerInfo = "ServiceDomain " + String(serviceInfo.serviceDomain());
      break;
    case MDNSResponder::AnswerType::HostDomainAndPort :
      answerInfo = "HostDomainAndPort " + String(serviceInfo.hostDomain()) + ":" + String(serviceInfo.hostPort());
      break;
    case MDNSResponder::AnswerType::IP4Address :
      answerInfo = "IP4Address ";
      for (IPAddress ip : serviceInfo.IP4Adresses()) {
        answerInfo += "- " + ip.toString();
      };
      break;
    case MDNSResponder::AnswerType::Txt :
      answerInfo = "TXT " + String(serviceInfo.strKeyValue());
      for (auto kv : serviceInfo.keyValues()) {
        answerInfo += "\nkv : " + String(kv.first) + " : " + String(kv.second);
      }
      break;
    default :
      answerInfo = "Unknown Answertype";
  }
  Serial.printf("Answer %s %s\n", answerInfo.c_str(), p_bSetContent ? "Modified" : "Deleted");
}
#endif
