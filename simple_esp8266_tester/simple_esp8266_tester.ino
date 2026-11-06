#define LED_PIN 5

void setup() {
    Serial.begin(115200);
    delay(100);
    Serial.println("Hello there");
    pinMode(LED_PIN, OUTPUT);

}

void loop() {
    Serial.print("High... ");
    digitalWrite(LED_PIN, HIGH);
    delay(2000);
    Serial.println("Low");
    digitalWrite(LED_PIN, LOW);
    delay(2000);
}
