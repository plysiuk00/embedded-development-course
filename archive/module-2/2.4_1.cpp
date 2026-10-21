#include <Arduino.h>

#define BUTTON_PIN 4

volatile uint32_t counter = 0;

void IRAM_ATTR handleInterrupt() {
    counter++;
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
    static uint32_t lastCounter = 0;

    if (counter != lastCounter) {
        Serial.println(counter);
        lastCounter = counter;
    }
}
