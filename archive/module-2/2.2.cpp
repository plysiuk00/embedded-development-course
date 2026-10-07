#include <Arduino.h>

#define MOTOR_PIN 4
#define BUTTON_PIN 18

int level = 0;

void setup() {
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);

    ledcSetup(0, 1000, 8);
    ledcAttachPin(MOTOR_PIN, 0);

    ledcWrite(0, 0);

    Serial.println("System started");
    Serial.println("Motor: OFF");
}

void loop() {
    if (digitalRead(BUTTON_PIN) == LOW) {
        level++;

        if (level > 3) {
            level = 0;
        }

        if (level == 0) {
            ledcWrite(0, 0);
            Serial.println("Button pressed -> Motor OFF");
        } else if (level == 1) {
            ledcWrite(0, 84); // ~33%
            Serial.println("Button pressed -> Motor 33%");
        } else if (level == 2) {
            ledcWrite(0, 128); // ~50%
            Serial.println("Button pressed -> Motor 50%");
        } else if (level == 3) {
            ledcWrite(0, 255); // 100%
            Serial.println("Button pressed -> Motor 100%");
        }

        // Чекаємо, поки кнопку відпустять
        while (digitalRead(BUTTON_PIN) == LOW) {
            delay(10);
        }

        delay(50);
    }
}
