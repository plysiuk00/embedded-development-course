#include <Arduino.h>
const int BUTTON_PIN = 4;
const int LED_PIN = 2;

const unsigned long DEBOUNCE_TIME = 0;

int lastReading = LOW;
int stableState = LOW;

unsigned long lastChangeTime = 0;

void setup() {
    pinMode(BUTTON_PIN, INPUT);
    pinMode(LED_PIN, OUTPUT);

    digitalWrite(LED_PIN, HIGH);

    Serial.begin(115200);
}

void loop() {
    int reading = digitalRead(BUTTON_PIN);

    // Сигнал змінився
    if (reading != lastReading) {
        lastChangeTime = millis();
        lastReading = reading;
    }

    // Якщо сигнал стабільний 50 мс — приймаємо новий стан
    if (millis() - lastChangeTime >= DEBOUNCE_TIME) {
        if (reading != stableState) {
            stableState = reading;

            if (stableState == HIGH) {
                Serial.println("BUTTON PRESSED");
                digitalWrite(LED_PIN, LOW);
            } else {
                Serial.println("BUTTON RELEASED");
                digitalWrite(LED_PIN, HIGH);
            }
        }
    }
}