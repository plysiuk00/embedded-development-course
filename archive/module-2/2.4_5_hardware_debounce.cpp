#include <Arduino.h>

#define BUTTON_PIN 4

constexpr uint32_t PollInterval = 5;
constexpr uint32_t DebounceTime = 50;

uint32_t counter = 0;
uint32_t lastPollTime = 0;
uint32_t stateChangeTime = 0;

bool lastReading = HIGH;
bool stableState = HIGH;

void setup() {
    Serial.begin(115200);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
}

void loop() {
    uint32_t currentTime = millis();

    if (currentTime - lastPollTime >= PollInterval) {
        lastPollTime = currentTime;

        bool reading = digitalRead(BUTTON_PIN);

        if (reading != lastReading) {
            lastReading = reading;
            stateChangeTime = currentTime;
        }

        if (currentTime - stateChangeTime >= DebounceTime) {
            if (reading != stableState) {
                stableState = reading;

                // Реагуємо лише на натискання
                if (stableState == LOW) {
                    counter++;
                    Serial.println(counter);
                }
            }
        }
    }
}