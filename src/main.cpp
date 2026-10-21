
#include <Arduino.h>
#include "driver/gpio.h"
#include "esp_task_wdt.h"

constexpr gpio_num_t FAN_PIN = GPIO_NUM_4;
constexpr uint8_t GREEN_LED_PIN = 6;
constexpr uint8_t RED_LED_PIN = 7;

constexpr uint32_t WORK_TIME_SECONDS = 3;
constexpr uint32_t PERIOD_SECONDS = 10;
constexpr uint32_t WATCHDOG_TIMEOUT_SECONDS = 5;

hw_timer_t* timer = nullptr;
volatile uint32_t elapsedSeconds = 0;

constexpr uint8_t EVENT_FAN_OFF = 1;
constexpr uint8_t EVENT_NEW_CYCLE = 2;

volatile uint8_t pendingEvents = 0;
portMUX_TYPE eventMux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR onTimer() {
    elapsedSeconds++;

    if (elapsedSeconds == WORK_TIME_SECONDS) {
        gpio_set_level(FAN_PIN, 0);
        gpio_set_level((gpio_num_t)GREEN_LED_PIN, 0);
        gpio_set_level((gpio_num_t)RED_LED_PIN, 1);

        portENTER_CRITICAL_ISR(&eventMux);
        pendingEvents |= EVENT_FAN_OFF;
        portEXIT_CRITICAL_ISR(&eventMux);
    }

    if (elapsedSeconds >= PERIOD_SECONDS) {
        elapsedSeconds = 0;
        gpio_set_level(FAN_PIN, 1);
        gpio_set_level((gpio_num_t)GREEN_LED_PIN, 1);
        gpio_set_level((gpio_num_t)RED_LED_PIN, 0);

        portENTER_CRITICAL_ISR(&eventMux);
        pendingEvents |= EVENT_NEW_CYCLE;
        portEXIT_CRITICAL_ISR(&eventMux);
    }
}

void setup() {
    Serial.begin(115200);

    pinMode((uint8_t)FAN_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    pinMode(RED_LED_PIN, OUTPUT);

    gpio_set_level(FAN_PIN, 1);
    digitalWrite(GREEN_LED_PIN, HIGH);
    digitalWrite(RED_LED_PIN, LOW);

    Serial.println("System started");
    Serial.println("Fan: ON");
    Serial.println("Green LED: ON");

    esp_task_wdt_init(WATCHDOG_TIMEOUT_SECONDS, true);
    esp_task_wdt_add(NULL);

    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 1000000, true);
    timerAlarmEnable(timer);
}

void loop() {
    uint8_t events;

    portENTER_CRITICAL(&eventMux);
    events = pendingEvents;
    pendingEvents = 0;
    portEXIT_CRITICAL(&eventMux);

    if (events & EVENT_FAN_OFF) {
        Serial.println("Fan: OFF");
        Serial.println("Green LED: OFF");
        Serial.println("Red LED: ON");
    }

    if (events & EVENT_NEW_CYCLE) {
        Serial.println("New cycle started");
        Serial.println("Fan: ON");
        Serial.println("Green LED: ON");
        Serial.println("Red LED: OFF");
    }

    esp_task_wdt_reset();
}
