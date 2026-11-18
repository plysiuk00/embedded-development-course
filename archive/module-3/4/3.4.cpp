
#include <stdint.h>
#include <stdbool.h>

#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// GPIO configuration
#define BUZZER_PIN GPIO_NUM_17

#define LED_COUNT 5

static const gpio_num_t led_pins[LED_COUNT] = {
    GPIO_NUM_5,
    GPIO_NUM_6,
    GPIO_NUM_7,
    GPIO_NUM_15,
    GPIO_NUM_16
};

// LEDC configuration
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_RESOLUTION LEDC_TIMER_10_BIT

#define PWM_DUTY 512

// Player timing
#define TICK_US 50000

// Musical notes
#define REST     0
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_D6  1175

// Note structure
typedef struct {
    uint16_t frequency;
    uint8_t duration_ticks;
    uint8_t led_index;
} Note;

// LED mapping:
// LED 1: E
// LED 2: F
// LED 3: G
// LED 4: A, B
// LED 5: C, D

static const Note melody[] = {
    {NOTE_E5,  3, 0},
    {NOTE_G5,  3, 2},
    {NOTE_A5,  6, 3},
    {NOTE_A5,  3, 3},
    {NOTE_A5,  3, 3},
    {NOTE_B5,  6, 3},
    {NOTE_C6,  6, 4},

    {NOTE_C6,  3, 4},
    {NOTE_C6,  3, 4},
    {NOTE_D6,  6, 4},
    {NOTE_B5,  6, 3},
    {NOTE_A5,  6, 3},

    {NOTE_G5,  6, 2},
    {NOTE_E5,  6, 0},
    {NOTE_G5,  6, 2},
    {NOTE_A5, 12, 3},

    {REST,     3, 255},

    {NOTE_E5,  3, 0},
    {NOTE_G5,  3, 2},
    {NOTE_A5,  6, 3},
    {NOTE_A5,  3, 3},
    {NOTE_A5,  3, 3},
    {NOTE_B5,  6, 3},
    {NOTE_C6,  6, 4},

    {NOTE_D6,  6, 4},
    {NOTE_C6,  6, 4},
    {NOTE_B5,  6, 3},
    {NOTE_A5,  6, 3},
    {NOTE_G5,  6, 2},
    {NOTE_E5, 12, 0}
};

#define MELODY_LENGTH (sizeof(melody) / sizeof(melody[0]))

// Player state
static uint32_t current_note = 0;
static uint8_t elapsed_ticks = 0;
static int64_t last_tick_us = 0;

// Turn off all LEDs
static void leds_off(void)
{
    for (int i = 0; i < LED_COUNT; i++) {
        gpio_set_level(led_pins[i], 0);
    }
}

// Stop buzzer
static void buzzer_off(void)
{
    ESP_ERROR_CHECK(
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0)
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL)
    );
}

// Play a note
static void play_note(uint16_t frequency)
{
    if (frequency == REST) {
        buzzer_off();
        return;
    }

    ESP_ERROR_CHECK(
        ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency)
    );

    ESP_ERROR_CHECK(
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, PWM_DUTY)
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL)
    );
}

// Display current note
static void show_note(uint8_t led_index)
{
    leds_off();

    if (led_index < LED_COUNT) {
        gpio_set_level(led_pins[led_index], 1);
    }
}

// Start current note
static void start_note(void)
{
    const Note *note = &melody[current_note];

    play_note(note->frequency);
    show_note(note->led_index);
}

// Advance melody
static void player_update(void)
{
    elapsed_ticks++;

    if (elapsed_ticks >= melody[current_note].duration_ticks) {
        elapsed_ticks = 0;

        current_note++;

        if (current_note >= MELODY_LENGTH) {
            current_note = 0;
        }

        start_note();
    }
}

// Initialize hardware
static void hardware_init(void)
{
    for (int i = 0; i < LED_COUNT; i++) {
        ESP_ERROR_CHECK(gpio_reset_pin(led_pins[i]));
        ESP_ERROR_CHECK(
            gpio_set_direction(led_pins[i], GPIO_MODE_OUTPUT)
        );
    }

    leds_off();

    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_RESOLUTION,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}

// Application entry point
void app_main(void)
{
    hardware_init();

    current_note = 0;
    elapsed_ticks = 0;

    start_note();

    last_tick_us = esp_timer_get_time();

    while (1) {
        int64_t now = esp_timer_get_time();

        if (now - last_tick_us >= TICK_US) {
            last_tick_us += TICK_US;
            player_update();
        }

        taskYIELD();
    }
}
