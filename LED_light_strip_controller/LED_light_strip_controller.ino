#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <FastLED.h>
#include "wifi_credentials.h"
#include "color_remote.h"

#define DEBUG 1

#define LED_PIN         5
#define NUM_LEDS        50
#define SERVICE_PORT    1234

WiFiServer server(SERVICE_PORT);
CRGB leds[NUM_LEDS];
CRGB currentColor = CRGB::Black;

// Stuff for ZeroConf/Bonjour
bool bHostDomainConfirmed = false;
char *pcHostDomain = NULL;
MDNSResponder::hMDNSService hMDNSService = NULL;
MDNSResponder::hMDNSServiceQuery hMDNSServiceQuery = NULL;
const char *serviceIdent = "ledicious";

uint8_t brightness = 44;

#define MAX_MSG_SIZE 256
typedef struct t_results {
     char msg[MAX_MSG_SIZE];
     int code;
} CmdResults;


void setup() {
    Serial.begin(9600);
    delay(100);
    Serial.println(F("Well, hello there..."));

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

    server.begin();

    MDNS.setHostProbeResultCallback(hostProbeCallback);
    if (!MDNSResponder::indexDomain(pcHostDomain, 0, "arduino_1") || !MDNS.begin(pcHostDomain)) { 
        Serial.println("Crud... mDNS did not start");
    }
    else {
        Serial.println("mDNS should be working");
    }

    FastLED.addLeds<WS2812,LED_PIN,GRB>(leds, NUM_LEDS);
    FastLED.setMaxPowerInVoltsAndMilliamps(5, 10000);
    fill_solid(leds, NUM_LEDS, currentColor);
    FastLED.setBrightness(brightness); 
    FastLED.show();

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

#define FX_trails       0
#define FX_totalEffects 1
#define FX_noEffect     255
uint8_t currentEffect = FX_noEffect;

void loop() {
    MDNS.update();
    
    WiFiClient client = server.available();
    if (client) {
        Serial.println("[client connected]");
        while (client.connected()) {
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
    
#if 0
    if (irrecv.decode(&irResults)) {
        uint8_t limit = 0;
        Serial.println(irResults.value, BIN);
        testValue = (irResults.value == REPEAT) ? lastValue : irResults.value;
        // Leave lights off if we're "off" and the button pressed is not the On button
        if (isOn == false && testValue != ON_BUTTON) {
            irrecv.resume();
            return;
        }
        switch(testValue) {
            case R_BUTTON:
                currentColor.red += 1;
                logColor(currentColor);
                fill_solid(leds, NUM_LEDS, currentColor);
                FastLED.show();                 
                break;
            case G_BUTTON:
                currentColor.green += 1;
                logColor(currentColor);
                fill_solid(leds, NUM_LEDS, currentColor);
                FastLED.show();                 
                break;
            case B_BUTTON:
                currentColor.blue += 1;
                fill_solid(leds, NUM_LEDS, currentColor);
                logColor(currentColor);
                FastLED.show();                 
                break;
            case ON_BUTTON:
                if (isOn != true) {
                    isOn = true;
                    fill_solid(leds, NUM_LEDS, currentColor);
                    FastLED.show();                 
                }
                else {
                    // repeated On button presses do effects!
                    currentEffect += 1;
                    if (currentEffect == FX_totalEffects) {
                        currentEffect = FX_noEffect;
                    }
                }
                break;
            case OFF_BUTTON:
                isOn = false;
                fill_solid(leds, NUM_LEDS, CRGB::Black);
                FastLED.show();
                break;
            case INCREASE_BRIGHTNESS:
                limit = 255 - brightness;
                brightness += (limit >= brightnessAdjustment) ? brightnessAdjustment : limit;
                FastLED.setBrightness(brightness);
                FastLED.show();
                break;
            case DECREASE_BRIGHTNESS:
                brightness -= (brightness >= brightnessAdjustment) ? brightnessAdjustment : brightness;
                FastLED.setBrightness(brightness);
                FastLED.show();
                break;
            case FLASH:
                break;
            case STROBE:
                break;
            case FADE:
                fadeall();
                FastLED.show();
                break;
            case SMOOTH:
                // do something
                break;
            default:
                CRGB c = colorForButton(testValue);
                Serial.print("whut: ");Serial.print(testValue, HEX);Serial.print("  ");Serial.println(c == CRGB(CRGB::Brown));
                // Brown is an error, don't use it
                if (c != CRGB(CRGB::Brown)) {
                    currentColor = c;
                    fill_solid(leds, NUM_LEDS, c);
                    FastLED.show();
                }
        }
        if (irResults.value != REPEAT) {
            lastValue = irResults.value;
        }
        irrecv.resume();
    }

    switch(currentEffect) {
        case FX_trails: trails(); break;
        default:
            break;
    }
    delay(50);
#endif
//    Serial.println("loop");
//    trails();
//    delay(50);

}

#define SET_OK  sprintf(r->msg, "ok")
CmdResults *commandProcessor(String cmd) {
    CmdResults *r = (CmdResults *)malloc(sizeof(CmdResults));
    bzero(r, sizeof(CmdResults));
    if (cmd == "ping") {
        sprintf(r->msg, "pong");
        r->code = 0;
    }
    else if (cmd == "currentConfig") {
        sprintf(r->msg, "currentConfig:0x%02X%02X%02X|none|%hhu", currentColor.red, currentColor.green, currentColor.blue, brightness);
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
        fill_solid(leds, NUM_LEDS, currentColor);
        FastLED.show();
    }
    else if (cmd.startsWith("effect:")) {
        r->code = 0;
        SET_OK;
        String effectName = cmd.substring(7);
        Serial.println(effectName);
    }
    else if (cmd.startsWith("brightness:")) {
        if (cmd.charAt(11) == '+') {
            uint8_t limit = 255 - brightness;
            brightness += (limit >= brightnessAdjustment) ? brightnessAdjustment : limit;
        }
        else {
            brightness -= (brightness >= brightnessAdjustment) ? brightnessAdjustment : brightness;
        }
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

CRGB colorForButton(unsigned long buttonValue) {
    CRGB c;
    switch(buttonValue) {
        case R_BUTTON:
            c = CRGB::Red;
            break;
        case G_BUTTON:
            c = CRGB::Green;
            break;
        case B_BUTTON:
            c = CRGB::Blue;
            break;
        case W_BUTTON:
            c = CRGB::White;
            break;
        case LIGHT_ORANGE:
            c = CRGB(0xFF4B0E);
            break;
        case LIGHT_GREEN:
            c = CRGB(0x64FE2E);
            break;
        case BLUE:
            c = CRGB::Blue;
            break;
        case ORANGE:
            c = CRGB(0xFF4500);
            break;
        case LIGHT_BLUE:
            c = CRGB(0x87CEFA);
            break;
        case PURPLE:
            c = CRGB(0x4B0082);
            break;
        case TAN:
            c = CRGB(0xF43F2D);
            break;
        case SEAFOAM:
            c = CRGB(0x4682B4);
            break;
        case MAROON:
            c = CRGB(0xC71585);
            break;
        case YELLOW:
            c = CRGB(0xFFD700);
            break;
        case JADE:
            c = CRGB(0x20B2AA);
            break;
        case PINK:
            c = CRGB(0xFF0060);
            break;

        default:
            c = CRGB::Brown;
            break;
    }
    return c;
}

void fadeall() { 
    for(int i = 0; i < NUM_LEDS; i++) { leds[i].nscale8(250); } 
}

void trails() {
    Serial.println("trails");
    static uint8_t hue = 0;
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
}

//
// ZeroConf/Bonjour things
//
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
