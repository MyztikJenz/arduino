// Colors are the wires on the perfboard setup
const int buttonPin = 9;    // white
const int clockPin = 10;    // yellow
const int latchPin = 11;    // blue
const int outputPin = 12;   // green

const int debugLevel = 0;

#define NUM_LEDS    4

int effectTimer = 0;

// Remember to wire the lowest layer to layer1, which is the first of the four bits
// on the shift register
//const uint8_t layer1 = (1 << 0);
//const uint8_t layer2 = (1 << 1);
//const uint8_t layer3 = (1 << 2);
//const uint8_t layer4 = (1 << 3);

uint8_t cube[4][4];

void setup() {
    Serial.begin(9600);

    pinMode(buttonPin, INPUT_PULLUP);
    pinMode(latchPin, OUTPUT);
    pinMode(outputPin, OUTPUT);
    pinMode(clockPin, OUTPUT);

    pinMode(LED_BUILTIN, OUTPUT);

//    uint8_t layer = B00001110; // Only the first four bits matter
//    uint16_t values = (B10000000 << 8) + (B11110000); 
//
//    shiftOut(outputPin, clockPin, MSBFIRST, layer1);
//    shiftOut(outputPin, clockPin, MSBFIRST, (values >> 8));
//    shiftOut(outputPin, clockPin, MSBFIRST, values);
//
//    digitalWrite(latchPin, LOW);    
//    digitalWrite(latchPin, HIGH);
//    digitalWrite(latchPin, LOW);
//
//    Serial.println("done");
    effectTimer = 0;

    clearCube();
//    setVoxel(0, 0, 0);
//    setVoxel(1, 0, 0);
//    setVoxel(0, 0, 1);
//    renderCube();
    printCube();
}

#define PATTERN_boxThrob    0
#define PATTERN_layerCycle  1
#define PATTERN_test        2
#define MAX_PATTERNS        2

typedef enum t_effects {
    boxThrob = 0,
    layerCycle,
    chaserEffect,
    allOnThenOff,

    totalNumberOfEffects
} Effect;

Effect currentEffect = layerCycle;

int lastButtonPressed = HIGH;
void loop() {
    int currentButton = digitalRead(buttonPin);
    if (currentButton != lastButtonPressed) {
        if (currentButton == LOW) {
            currentEffect = (currentEffect + 1) % MAX_PATTERNS;
            Serial.println(currentEffect);
        }
        lastButtonPressed = currentButton;
    }

    switch(currentEffect) {
        case boxThrob: throbCube(); break;
        case layerCycle: cycleLightLayers(); break;
        case chaserEffect: chaser(); break;
        case allOnThenOff: allOnOffEffect(); break;
//        case PATTERN_test: testPattern(); break;

        default: throbCube();
        
    }
    
    renderCube();
}

void chaser() {
    testPattern();
}

bool allOn = true;
void allOnOffEffect() {
    if (allOn) {
        allOn = false;
//        for (uint8_t layer=0; layer<NUM_LEDS; layer++) {
//            lightLayer(layer, false);
//        }
        digitalWrite(LED_BUILTIN, HIGH);
        lightEntireCube(2);
    }
    else {
        allOn = true;
        clearCube();
        digitalWrite(LED_BUILTIN, LOW);
        delay(2000);
    }
}

uint8_t currentLayer = 0;
bool layersAreHorizontal = true;
bool layerDirectionForward = true;
void cycleLightLayers() {
    effectTimer++;
    if (effectTimer > 100) {
        effectTimer = 0;
        lightLayer(layerDirectionForward ? currentLayer++ : currentLayer--, layersAreHorizontal);
        if (currentLayer >= NUM_LEDS && layerDirectionForward) {
            currentLayer = NUM_LEDS-2;
            layerDirectionForward = !layerDirectionForward;
        }
        else if (currentLayer == 255 && !layerDirectionForward) {
            currentLayer = 0;
            layerDirectionForward = !layerDirectionForward;
            layersAreHorizontal = !layersAreHorizontal;
        }
    }
    
}

void lightLayer(uint8_t tgtLayer, bool horizontal) {
    clearCube();
    if (horizontal == true) {
        for (uint8_t row=0; row<NUM_LEDS; row++) {
            cube[tgtLayer][row] = 0xF;
        }
    }
    else {
        // for vertical layers, layer 1 is leftmost
        for (uint8_t layer=0; layer<NUM_LEDS; layer++) {
            for (uint8_t row=0; row<NUM_LEDS; row++) {
                setVoxel(tgtLayer, layer, row);
            }
        }
    }
}

// Our cube has (0,0,0) in the front, lower left. This is because the wiring has the LSB at that point.
// While putting zero in the back, upper left would make my UIKit-addled brain happier, it makes
// the logic complicated for little gain. A future version of this could put zero there, but the wiring would 
// need to support it.
// cube[layer][row] = slide, or cube[y][z] = x
// Rows are perpendicular to the front, and move positively towards the back
void setVoxel(uint8_t x, uint8_t y, uint8_t z) {
    cube[y][z] |= (1 << x);
}

uint8_t cubeSize = 1;
bool cubeExpanding = true;
void throbCube() {
    effectTimer++;
    if (effectTimer > 200) {
        effectTimer = 0;
        if (cubeExpanding) {
            cubeSize++;
        }
        else {
            cubeSize--;
        }
        clearCube();
        drawCube(0, 0, 0, cubeSize);

        if (cubeSize == 4) {
            cubeExpanding = false;
        }
        else if (cubeSize == 2) {
            cubeExpanding = true;
        }
    }
}

void drawCube(uint8_t x, uint8_t y, uint8_t z, uint8_t s) {
    for (uint8_t i = 0; i < s; i++) {
        // Sets front left corner
        setVoxel(x, y + i, z);
        setVoxel(x + i, y, z);
        setVoxel(x, y, z + i);

        // Sets the back right corner
        setVoxel(x + s - 1, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z + s - 1);
        setVoxel(x + s - 1, y + s - 1, z + i);

        // There's gaps in some rows, these fill them in
        setVoxel(x + s - 1, y + i, z);
        setVoxel(x, y + i, z + s - 1);
        setVoxel(x + i, y + s - 1, z);
        setVoxel(x + i, y, z + s - 1);
        setVoxel(x + s - 1, y, z + i);
        setVoxel(x, y + s - 1, z + i);
    } 
}

void clearCube() {
    for (uint8_t layer=0; layer<NUM_LEDS; layer++) {
        for (uint8_t row=0; row<NUM_LEDS; row++) {
            cube[layer][row] = 0x0;
        }
    }
}

void renderCube() {
    for (uint8_t layer=0; layer<4; layer++) {
        uint16_t leds = 0;
        for (uint8_t row=0; row<4; row++) {
            uint8_t values = cube[layer][row];
            // Remember, rows are perpendicular to the front and move positively towards the back.
            leds |= (values & 0xF) << (4 * row);
        }
        renderCubeAtLayer((1 << layer), leds);
    }
}

void printCube() {
    char str[25];
    for (uint8_t layer=0; layer<4; layer++) {
        Serial.print("Layer");Serial.println(layer);
        for (uint8_t row=0; row<4; row++) {
            uint8_t values = cube[layer][row];
            sprintf(str, "row: %d  [ 0x%02x ]", row, values & 0xF);
            Serial.println(str);
        }
    }    
}

void testPattern() {
    uint8_t currentLayer = 0;
    uint16_t currentLED  = 0;

    while(true) {
        renderCubeAtLayer((1 << currentLayer), (1 << currentLED));
        currentLED = (currentLED + 1) % 16;
        if (currentLED == 0) {
            currentLayer = (currentLayer + 1) % 4;
        }

        delay(100);
    }
}

void lightEntireCube(int holdForSeconds) {
    uint8_t currentLayer = 0;
    uint16_t currentLED  = 0;
    unsigned long startTime = millis();
    unsigned long endTime = startTime;
    unsigned long msToHold = 1000 * holdForSeconds;

    while ((endTime - startTime) < msToHold) {
        renderCubeAtLayer((1 << currentLayer), (1 << currentLED));
        currentLED = (currentLED + 1) % 16;
        if (currentLED == 0) {
            currentLayer = (currentLayer + 1) % 4;
        }
        endTime = millis();
    }
}

const char *numberToBin(uint32_t n) {
    static char str[37];
    int spaceCtr = 0;
    str[0] = '\0';
    char *p = str;
    for (uint32_t x=(1UL<<31); x>0; x>>=1) {
        *p = ((n & x) == x) ? '1' : '0';
        p++;
        spaceCtr++;
        if (spaceCtr == 8) {
            spaceCtr = 0;
            *p = ' ';
            p++;
        }
    }
    *p = '\0';
    return str;
}

void renderCubeAtLayer(uint8_t layer, uint16_t led) {
    if (debugLevel >= 2) {
        // This is painfully slow... it'll break any chance for persistence of vision to work
        char str[128];
        uint32_t bitstream = layer;
        const char *bsStr = numberToBin((bitstream << 16) | led);
        sprintf(str, "current layer: %d led: %3d bitstream: %s", layer, led, bsStr);
        Serial.println(str);
    }
    
    shiftOut(outputPin, clockPin, MSBFIRST, layer);
    shiftOut(outputPin, clockPin, MSBFIRST, (led >> 8));
    shiftOut(outputPin, clockPin, MSBFIRST, led);

    digitalWrite(latchPin, LOW);    
    digitalWrite(latchPin, HIGH);
    digitalWrite(latchPin, LOW);
}
