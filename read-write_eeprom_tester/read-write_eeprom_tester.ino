#define addressLineDataOut          A0  // Pin 14/DS on first 595
#define addressLineLatchPin         A1  // Pin 12/ST_CP (storage register)
#define addressLineShiftClockPin    2   // Pin 11/SH_CP (shift register)

#define eepromWriteEnable           3   // Pin 21/WE(active low) on AT28C16
#define eepromOutputEnable          4   // Pin 20/OE(active low)
#define eepromChipEnable            5   // Pin 18/CE(active low)

#define dataOutDataOut              6   // Pin 14/DS on third 595
#define dataOutOutputEnable         7   // Pin 13/OE(active low)
#define dataOutLatchPin             8   // Pin 12/ST_CP (storage register)
#define dataOutShiftClockPin        9   // Pin 11/SH_CP (shift register)

#define dataInClockInhibit          10  // Pin 15/CE/Clock enable(active LOW) CLK INH on 165
#define dataInDataIn                11  // Pin 9/Qh
#define dataInShiftLoadPin          12  // Pin 1/SH/LD/PL/Parallel load (LOW from parallel)
#define dataInClockPin              13  // Pin 2/CLK/CP/Clock Pulse


void setup() {
    Serial.begin(9600);
    
    pinMode(addressLineDataOut, OUTPUT);
    pinMode(addressLineLatchPin, OUTPUT);
    pinMode(addressLineShiftClockPin, OUTPUT);

    pinMode(eepromWriteEnable, OUTPUT);
    pinMode(eepromOutputEnable, OUTPUT);
    pinMode(eepromChipEnable, OUTPUT);

    pinMode(dataOutDataOut, OUTPUT);
    pinMode(dataOutOutputEnable, OUTPUT);
    pinMode(dataOutLatchPin, OUTPUT);
    pinMode(dataOutShiftClockPin, OUTPUT);

    pinMode(dataInClockInhibit, OUTPUT);
    pinMode(dataInDataIn, INPUT);
    pinMode(dataInShiftLoadPin, OUTPUT);
    pinMode(dataInClockPin, OUTPUT);

    // Disable everyone on startup, just to be sure
    // Disable the ROM
    digitalWrite(eepromChipEnable, HIGH);
    // Disable the data-write 595
    digitalWrite(dataOutOutputEnable, HIGH);
    // Disable read of 165
    digitalWrite(dataInClockInhibit, HIGH);

//    printContents();
//    delay(10000);
//    saveValueToROM(0x55, 0);
//    byte r = readValueFromROM(0);
//    Serial.print("address 0: ");
//    Serial.println(r, HEX);
    
//    _setAddress(0x55);
//    Serial.println("address set to 0x55");
//    delay(10000000);

//    Serial.print("Erasing to zero...");
//    for(int x=0; x<2048; x++) {
//        saveValueToROM(0x00, x);
//    }
//    Serial.println(" done");
    saveValueToROM(0x08, 2047);
    printContents();
    delay(4000000);
}

void printContents() {
    for (int x=0; x<2048; x+=16) {
        byte data[16];
        for(int s=0; s<16; s++) {
            data[s] = readValueFromROM(x + s);
        }
        char buf[80];
        sprintf(buf, "%03x: %02x %02x %02x %02x %02x %02x %02x %02x   %02x %02x %02x %02x %02x %02x %02x %02x",
                     x, data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7],
                        data[8], data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
        Serial.println(buf);
    }
}


void _setAddress(int address) {
    // Put an address on the line. The address 595s have their OE pins hardwired enabled.
    // No need to toggle them on/off
    shiftOut(addressLineDataOut, addressLineShiftClockPin, MSBFIRST, (address >> 8));
    shiftOut(addressLineDataOut, addressLineShiftClockPin, MSBFIRST, address);
    digitalWrite(addressLineLatchPin, LOW);
    digitalWrite(addressLineLatchPin, HIGH);
    digitalWrite(addressLineLatchPin, LOW);
}

void saveValueToROM(byte value, int address) {
    // Disable read of 165, ensure no bus contention
    digitalWrite(dataInClockInhibit, HIGH);
    
    _setAddress(address);
        
    // Put some data on the line for that address
    digitalWrite(dataOutOutputEnable, LOW); // enable to drive the bus
    digitalWrite(dataOutLatchPin, LOW);
    shiftOut(dataOutDataOut, dataOutShiftClockPin, MSBFIRST, value);
    digitalWrite(dataOutLatchPin, HIGH);
    
    // Pulse the memory to write the data
    digitalWrite(eepromOutputEnable, HIGH);
    digitalWrite(eepromWriteEnable, HIGH);
    digitalWrite(eepromChipEnable, HIGH);
    digitalWrite(eepromChipEnable, LOW);
    digitalWrite(eepromWriteEnable, LOW);
    delay(10);
    digitalWrite(eepromWriteEnable, HIGH);
    digitalWrite(eepromChipEnable, HIGH);
    digitalWrite(dataOutOutputEnable, HIGH); // ensure the 595 isn't trying to write when the 165 is reading
}

byte readValueFromROM(int address) {
    // Disable the data-write 595, ensure no bus contention
    digitalWrite(dataOutOutputEnable, HIGH);

    _setAddress(address);

    digitalWrite(eepromChipEnable, LOW);
    digitalWrite(eepromOutputEnable, LOW);

    digitalWrite(dataInShiftLoadPin, LOW);
    delayMicroseconds(5);
    digitalWrite(dataInShiftLoadPin, HIGH);
    delayMicroseconds(5);
    
    digitalWrite(dataInClockPin, HIGH);
    digitalWrite(dataInClockInhibit, LOW);
    byte incoming = shiftIn(dataInDataIn, dataInClockPin, MSBFIRST);
    digitalWrite(dataInClockInhibit, HIGH);

    digitalWrite(eepromOutputEnable, HIGH);
    digitalWrite(eepromChipEnable, HIGH);

    return incoming;
}

typedef struct test_t {
    int address;
    byte value;
} TestValues;

TestValues tests[5] = {
    {0, 42},
    {1, 47},
    {255, 8},
    {1024, 100},
    {2000, 0xff}
};

void loop() {

    // Put an address on the line
//    digitalWrite(addressLineLatchPin, LOW);
//    int memAddr = 1;
//    shiftOut(addressLineDataOut, addressLineShiftClockPin, MSBFIRST, (memAddr >> 8));
//    shiftOut(addressLineDataOut, addressLineShiftClockPin, MSBFIRST, memAddr);
//    digitalWrite(addressLineLatchPin, HIGH);
//    
//    // Put some data on the line for that address
//    digitalWrite(dataOutLatchPin, LOW);
//    byte value = 112;
//    shiftOut(dataOutDataOut, dataOutShiftClockPin, MSBFIRST, value);
//    digitalWrite(dataOutLatchPin, HIGH);
//    digitalWrite(dataOutOutputEnable, LOW); // enable to drive the bus
//    
//    // Pulse the memory to write the data
//    digitalWrite(eepromChipEnable, LOW);
//    digitalWrite(eepromWriteEnable, LOW);
//    delay(20);
//    digitalWrite(eepromWriteEnable, HIGH);
//    digitalWrite(dataOutOutputEnable, LOW); // ensure the 595 isn't trying to write when the 165 is reading
    
    // Now to read from the address on the line
//    digitalWrite(dataInShiftLoadPin, LOW);
//    delayMicroseconds(5);
//    digitalWrite(dataInShiftLoadPin, HIGH);
//    delayMicroseconds(5);
//    
//    digitalWrite(dataInClockPin, HIGH);
//    digitalWrite(dataInClockInhibit, LOW);
//    byte incoming = shiftIn(dataInDataIn, dataInClockPin, MSBFIRST);
//    digitalWrite(dataInClockInhibit, HIGH)


    for(int x=0; x<sizeof(tests) / sizeof(TestValues); x++) {
        TestValues t = tests[x];
        int value = readValueFromROM(t.address);
        Serial.print(x);
        Serial.print(": ");
        Serial.print(t.address);
        Serial.print(" = ");
        Serial.println(value);
    }

    Serial.println("writing values...");
    for(int x=0; x<sizeof(tests) / sizeof(TestValues); x++) {
        TestValues t = tests[x];
        saveValueToROM(t.value, t.address);
    }
    delay(1000);
    Serial.println("done!");

//    Serial.print("wrote value ");
//    Serial.print(value);
//    Serial.print(" to address ");
//    Serial.print(memAddr, HEX);
//    Serial.print(". Read back value: ");
//    Serial.println(incoming);

    delay(6000000);
}
