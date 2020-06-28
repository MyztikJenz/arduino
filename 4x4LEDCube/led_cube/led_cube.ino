const int clockPin = 8;
const int latchPin = 9;
const int outputPin = 11;

const int debugLevel = 0;

// Remember to wire the lowest layer to layer1, which is the first of the four bits
// on the shift register
const uint8_t layer1 = (1 << 0);
const uint8_t layer2 = (1 << 1);
const uint8_t layer3 = (1 << 2);
const uint8_t layer4 = (1 << 3);

uint8_t cube[4][4];

void setup() {
    Serial.begin(9600);

    pinMode(latchPin, OUTPUT);
    pinMode(outputPin, OUTPUT);
    pinMode(clockPin, OUTPUT);

    uint8_t layer = B00001110; // Only the first four bits matter
    uint16_t values = (B10000000 << 8) + (B11110000); 

    shiftOut(outputPin, clockPin, MSBFIRST, layer1);
    shiftOut(outputPin, clockPin, MSBFIRST, (values >> 8));
    shiftOut(outputPin, clockPin, MSBFIRST, values);

    digitalWrite(latchPin, LOW);    
    digitalWrite(latchPin, HIGH);
    digitalWrite(latchPin, LOW);

    Serial.println("done");

    clearCube();
    setVoxel(0, 0, 0);
    setVoxel(1, 0, 0);
    setVoxel(0, 0, 1);
//    renderCube();
    printCube();
}

void loop() {
//    testPattern();

//    setVoxel(0, 0, 0);
    renderCube();
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

void clearCube() {
    for (uint8_t layer=0; layer<4; layer++) {
        for (uint8_t row=0; row<4; row++) {
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

        delay(500);
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
