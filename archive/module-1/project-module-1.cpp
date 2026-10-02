#include <Arduino.h>

// Призначення виводів GPIO
const int GPIO_IN = 1;
const int GPIO_OUT = 4;

// Визначення порогів освітленості з гістерезисом
const int THRESHOLD_DARK = 1600; // Нижній поріг: ADC < 1200 -> ТЕМНО (увімкнути реле)
const int THRESHOLD_LIGHT = 2000; // Верхній поріг: ADC > 2000 -> СВІТЛО (вимкнути реле)

void setup() {
    Serial.begin(115200);

    pinMode(GPIO_IN, INPUT);
    pinMode(GPIO_OUT, OUTPUT);

    digitalWrite(GPIO_OUT, LOW);

    Serial.println("Систему запущено.");
}

void loop() {
    int adcValue = analogRead(GPIO_IN);

    if (adcValue < THRESHOLD_DARK) {
        // Темно -> Подати HIGH на GPIO Out
        digitalWrite(GPIO_OUT, HIGH);
        Serial.print("ADC: ");
        Serial.print(adcValue);
        Serial.println(" | Стан: ТЕМНО -> GPIO Out = HIGH (Реле УВІМКНЕНО)");
    } else if (adcValue > THRESHOLD_LIGHT) {
        // Світло -> Подати LOW на GPIO Out
        digitalWrite(GPIO_OUT, LOW);
        Serial.print("ADC: ");
        Serial.print(adcValue);
        Serial.println(" | Стан: СВІТЛО -> GPIO Out = LOW (Реле ВИМКНЕНО)");
    } else {
        // Значення між порогами -> НІЧОГО НЕ ЗМІНЮВАТИ
        Serial.print("ADC: ");
        Serial.print(adcValue);
        Serial.println(" | Стан: ЗОНА ГІСТЕРЕЗИСУ (стан реле без змін)");
    }

    delay(300);
}
