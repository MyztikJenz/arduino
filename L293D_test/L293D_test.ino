#define port1A  9
#define port2A  10

void setup() {
    Serial.begin(9600);
    pinMode(port1A, OUTPUT);
    pinMode(port2A, OUTPUT);  
}

void loop() {
    digitalWrite(port1A, HIGH);
    digitalWrite(port2A, LOW);
    delay(2000);

    motorStop();
    delay(1000);

    digitalWrite(port1A, LOW);
    digitalWrite(port2A, HIGH);
    delay(2000);

    motorStop();
    delay(1000);
}

void motorStop() {
    digitalWrite(port1A, LOW);
    digitalWrite(port2A, LOW);
}
