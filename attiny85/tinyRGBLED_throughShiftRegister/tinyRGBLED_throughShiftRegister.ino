// This really benefits from running at 16Mhz, instead of 1Mhz
// You can set the speed by just selecting it from the Tools menu and then
// choosing to burn the bootloader. No need to do things from the command line.
#define RCLK    2
#define SRCLK   1
#define OE      0
#define DS      3
#define BUTTON  4

#define clockPin    SRCLK
#define latchPin    RCLK
#define enablePin   OE
#define dataPin     DS

unsigned long last_t = 0;
unsigned long accumulator = 0;
float potValue = 0;
uint8_t brightness_value = 0;

void orange(unsigned long);
void blue(unsigned long);
void white(unsigned long);
void pulseBlue(unsigned long);
void lightsOff(unsigned long);
#define MAX_FUNCTION_CNT 5

void (*f[MAX_FUNCTION_CNT])(unsigned long t) {orange, blue, white, pulseBlue, lightsOff};
void setup() {
    pinMode(clockPin, OUTPUT);    
    pinMode(latchPin, OUTPUT);    
    pinMode(enablePin, OUTPUT);    
    pinMode(dataPin, OUTPUT);
//    pinMode(POT, INPUT);
    pinMode(BUTTON, INPUT_PULLUP);

// http://www.technoblogy.com/show?LE0
//    GTCCR = 1<<PWM1B;
//    TCCR1 = 7<<CS10;

//    digitalWrite(enablePin, LOW);
    last_t = micros();
}

byte lastColor = 0;
void
writeColor(byte c) {
    if (c == lastColor) {
        return;
    }
    lastColor = c;
    digitalWrite(latchPin, LOW);
    // We have the same LEDs on two shift registers. So send this twice so the second 595
    // gets the right values
    shiftOut(dataPin, clockPin, MSBFIRST, c);
    shiftOut(dataPin, clockPin, MSBFIRST, c);
    digitalWrite(latchPin, HIGH);
}

void
purple(unsigned long msSec) {
    static uint16_t period = 1000; // µs
    static float duty = .25; 
    float onTime = period * duty;
    uint16_t slot = msSec % period;

    byte color = 0b00001001;
    
    if (slot <= onTime) {
        bitWrite(color, 2, 1);
        bitWrite(color, 5, 1);
    }
    writeColor(color);
}

void orange(unsigned long msSec) {
    analogWrite(enablePin, 0);
    static uint16_t period = 1000; // µs
    static float duty = .95; 
//    float duty = potValue / 1024.0;
    float onTime = period * duty;
    uint16_t slot = msSec % period;

    byte color = 0b00000001;
    
    if (slot <= onTime) {
//        bitWrite(color, 1, 1);
        writeColor(0b00001001);
    }
    else {
        writeColor(0b00011011);
    }
//    writeColor(color);
}

unsigned long b_accum = 0;
bool acsending = true;
void
brightness(unsigned long t) {
    b_accum = b_accum + (t - last_t);
    if (b_accum > 8000) {
        if (acsending) {
            brightness_value += 1;
            if (brightness_value == 255) {
                acsending = false;
            }
        }
        else {
            brightness_value -= 1;
            if (brightness_value == 0) {
                acsending = true;
            }
        }
        b_accum = 0;
        analogWrite(enablePin, brightness_value);
    }
}

void blue(unsigned long t) {
    analogWrite(enablePin, 0);
    byte color = 0b00100100;
    writeColor(color);
}

void
white(unsigned long t) {
    analogWrite(enablePin, 0);
    byte color = 0b00111111;
    writeColor(color);
}

void 
pulseBlue(unsigned long t) {
    brightness(t);
    // We can't just call blue(), it resets the brightness
    byte color = 0b00100100;
    writeColor(color);

}

void
lightsOff(unsigned long t) {
    analogWrite(enablePin, 255);
}



int lastButtonPressed = HIGH;
int f_idx = 0;
void loop() {
    unsigned long t = micros();

    int currentButton = digitalRead(BUTTON);
    if (currentButton != lastButtonPressed) {
        if (currentButton == LOW) {
            f_idx += 1;
            if (f_idx >= MAX_FUNCTION_CNT) {
                f_idx = 0;
            }
        }
        lastButtonPressed = currentButton;
    }

    (*f[f_idx])(t);

    accumulator = accumulator + (t - last_t);
    last_t = t;
}
