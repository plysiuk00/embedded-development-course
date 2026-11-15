
#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/adc_oneshot.h"
#include "esp_err.h"

// GPIO configuration
#define LDR_ADC_CHANNEL ADC_CHANNEL_0  // GPIO 1
#define LED_PIN GPIO_NUM_15

// SMA configuration
#define SMA_SIZE 10

// Hysteresis thresholds
#define DARK_THRESHOLD 1200
#define LIGHT_THRESHOLD 1800

// Sampling interval
#define SAMPLE_INTERVAL_MS 200

static adc_oneshot_unit_handle_t adc_handle;

static int samples[SMA_SIZE];
static int sample_index = 0;
static int sample_sum = 0;

static bool led_state = false;


// Initialize ADC
void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&unit_config, &adc_handle)
    );

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            LDR_ADC_CHANNEL,
            &channel_config
        )
    );
}


// Initialize LED
void led_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&config));

    gpio_set_level(LED_PIN, 0);
}


// Read ADC
int adc_read(void)
{
    int value = 0;

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc_handle,
            LDR_ADC_CHANNEL,
            &value
        )
    );

    return value;
}


// Calculate Simple Moving Average
int calculate_sma(int new_value)
{
    // Fill the buffer with the first measurement
    // to avoid averaging with zero values.
    if (sample_index == 0 && sample_sum == 0) {
        for (int i = 0; i < SMA_SIZE; i++) {
            samples[i] = new_value;
        }

        sample_sum = new_value * SMA_SIZE;
    }
    else {
        sample_sum -= samples[sample_index];

        samples[sample_index] = new_value;

        sample_sum += new_value;
    }

    sample_index = (sample_index + 1) % SMA_SIZE;

    return sample_sum / SMA_SIZE;
}


// Control LED using hysteresis
void update_led(int filtered_value)
{
    if (filtered_value < DARK_THRESHOLD) {
        led_state = true;
    }
    else if (filtered_value > LIGHT_THRESHOLD) {
        led_state = false;
    }

    gpio_set_level(LED_PIN, led_state);
}


void app_main(void)
{
    adc_init();
    led_init();

    printf("ESP32 SMA started\n");

    while (1) {
        int raw_value = adc_read();

        int filtered_value = calculate_sma(raw_value);

        update_led(filtered_value);

        printf(
            "ADC: %d | SMA: %d | LED: %s\n",
            raw_value,
            filtered_value,
            led_state ? "ON" : "OFF"
        );

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }
}
