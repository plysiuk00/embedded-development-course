#include <Arduino.h>

// ==================== TASK 1 ====================

enum class LedState {
    Off,
    On
};

class Led {
public:
    explicit Led(uint8_t pin) : pin_(pin) {}

    void init() {
        pinMode(pin_, OUTPUT);
        set(LedState::Off);
    }

    void set(LedState state) {
        state_ = state;

        if (state_ == LedState::On) {
            digitalWrite(pin_, HIGH);
        } else {
            digitalWrite(pin_, LOW);
        }
    }

private:
    uint8_t pin_;
    LedState state_ = LedState::Off;
};


// ==================== TASK 2 ====================

struct Config {
    static constexpr uint8_t LedPin = 4;
    static constexpr uint8_t ButtonPin = 0;

    static constexpr unsigned long BlinkInterval = 200;
    static constexpr unsigned long DebounceTime = 50;

    static constexpr unsigned long SerialBaudRate = 115200;
    static constexpr uint32_t MeasurementsCount = 100000;
};

Led led(Config::LedPin);


// ==================== TASK 4 ====================

// Button event flag
volatile bool buttonPressed = false;

// Interrupt Service Routine
void IRAM_ATTR buttonISR() {
    buttonPressed = true;
}


void setup() {
    led.init();

    Serial.begin(Config::SerialBaudRate);

    pinMode(Config::ButtonPin, INPUT_PULLUP);

    attachInterrupt(
        digitalPinToInterrupt(Config::ButtonPin),
        buttonISR,
        FALLING
    );
}


void loop() {

    // ==================== TASK 3 ====================

    const unsigned long startTime = micros();

    static unsigned long previousMillis = 0;
    static unsigned long lastButtonPress = 0;

    static uint32_t iterationCount = 0;
    static unsigned long totalTime = 0;


    // ==================== TASK 4 ====================

    static uint8_t mode = 0;

    const unsigned long currentMillis = millis();

    // Button event and debouncing are handled in loop()
    if (buttonPressed) {
        buttonPressed = false;

        if (currentMillis - lastButtonPress >= Config::DebounceTime) {
            lastButtonPress = currentMillis;

            // Blinking -> Always On -> Always Off -> Blinking
            mode++;

            if (mode > 2) {
                mode = 0;
            }
        }
    }


    // ==================== TASK 1 ====================

    // Non-blocking LED control
    if (mode == 0) {

        if (currentMillis - previousMillis >= Config::BlinkInterval) {
            previousMillis = currentMillis;

            static LedState blinkState = LedState::Off;

            blinkState = (blinkState == LedState::Off)
                             ? LedState::On
                             : LedState::Off;

            led.set(blinkState);
        }
    }
    else if (mode == 1) {
        led.set(LedState::On);
    }
    else {
        led.set(LedState::Off);
    }


    // ==================== TASK 3 ====================

    const unsigned long executionTime = micros() - startTime;

    totalTime += executionTime;
    iterationCount++;

    // Print average loop time every 100000 iterations
    if (iterationCount >= Config::MeasurementsCount) {

        const unsigned long averageTime =
            totalTime / iterationCount;

        Serial.print("Average loop time: ");
        Serial.print(averageTime);
        Serial.println(" us");

        totalTime = 0;
        iterationCount = 0;
    }
}