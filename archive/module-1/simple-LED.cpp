#include <Arduino.h>

// Якщо світлодіод не світиться, спробуйте змінити 2 на 13 або номер вашого GPIO
#define LED_PIN 5

void setup() {
    // Налаштовуємо пін як вихід живлення
    pinMode(LED_PIN, OUTPUT);
}

void loop() {
    // Вмикаємо світлодіод (подаємо 3.3V)
    digitalWrite(LED_PIN, HIGH);
    delay(1000); // Чекаємо 1 секунду

    // Вимикаємо світлодіод (подаємо 0V)
    digitalWrite(LED_PIN, LOW);
    delay(1000); // Чекаємо 1 секунду
}