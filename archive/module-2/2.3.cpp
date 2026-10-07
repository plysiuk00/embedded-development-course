#include <Arduino.h>

#define LED1 5
#define LED2 6
#define LED3 7

void setup() {
    pinMode(LED1, OUTPUT);
    pinMode(LED2, OUTPUT);
    pinMode(LED3, OUTPUT);
}

static uint32_t last_toggle1 = 0;
static uint32_t last_toggle2 = 0;
static uint32_t last_toggle3 = 0;

constexpr uint32_t intervalLed1 = 200;
constexpr uint32_t intervalLed2 = 500;
constexpr uint32_t intervalLed3 = 1000;

static void toggle_led(const int pin) {
    digitalWrite(pin, !digitalRead(pin));
}

static void update_led() {
    const uint32_t now = millis();

    if (now - last_toggle1 >= intervalLed1) {
        toggle_led(LED1);
        last_toggle1 = now;
    }

    if (now - last_toggle2 >= intervalLed2) {
        toggle_led(LED2);
        last_toggle2 = now;
    }

    if (now - last_toggle3 >= intervalLed3) {
        toggle_led(LED3);
        last_toggle3 = now;
    }
}

void loop() {
    update_led();
}