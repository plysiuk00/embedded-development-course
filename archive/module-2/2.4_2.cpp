#include <Arduino.h>

#define BUTTON_PIN 4
#define DEBOUNCE_MS 50

volatile bool interruptFlag = false;
uint32_t counter = 0;
uint32_t lastInterruptTime = 0;

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

        uint32_t currentTime = millis();

        if (currentTime - lastInterruptTime >= DEBOUNCE_MS) {
            counter++;
            lastInterruptTime = currentTime;

            Serial.println(counter);
        }
    }
}