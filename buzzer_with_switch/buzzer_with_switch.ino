#include "pitches.h"

int buzzerPin = 9;
int switchPin = 12;

void setup() {
    Serial.begin(57600);
    pinMode(buzzerPin, OUTPUT);
    pinMode(switchPin, INPUT);
}

void telephone() {
    for (int x=0; x<10; x++) {
        tone(buzzerPin, NOTE_G6);
        delay(50);
        tone(buzzerPin, NOTE_GS6);
        delay(50);
    }
}

void allTheTones() {
    for (int x=31; x<=4978; x++) {
        tone(buzzerPin, x);
        delay(10);
    }
}

void loop() {
    if (digitalRead(switchPin) == HIGH) {
        Serial.println("switch is on");
        telephone();
        noTone(buzzerPin);
        delay(1000);
    }
    else {
        Serial.println("switch is off");
        delay(1000);
    }
}


