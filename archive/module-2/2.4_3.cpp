#include <Arduino.h>

#define BUTTON_PIN 4

volatile bool interruptFlag = false;
uint32_t counter = 0;

void IRAM_ATTR handleInterrupt() {
    interruptFlag = true;
}

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(BUTTON_PIN),
        handleInterrupt,
        FALLING
    );
}

void loop() {
    if (interruptFlag) {
        noInterrupts();
        interruptFlag = false;
        interrupts();

        if (digitalRead(BUTTON_PIN) == LOW) {
            counter++;
            Serial.println(counter);
        }
    }
}