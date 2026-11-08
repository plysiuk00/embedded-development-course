#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/gpio.h"

// TASK 1: Pin configuration
#define LDR_CHANNEL ADC_CHANNEL_0
#define LED_PIN GPIO_NUM_15

// TASK 2: SMA configuration
#define SMA_WINDOW_SIZE 10

// TASK 3: Hysteresis thresholds
#define DARK_THRESHOLD 1200
#define LIGHT_THRESHOLD 1800

#define SAMPLE_INTERVAL_MS 200

void app_main(void)
{
    // TASK 1: ADC oneshot initialization
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&adc_config, &adc_handle)
    );

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            LDR_CHANNEL,
            &channel_config
        )
    );

    // TASK 1: LED initialization
    ESP_ERROR_CHECK(gpio_reset_pin(LED_PIN));
    ESP_ERROR_CHECK(gpio_set_direction(LED_PIN, GPIO_MODE_OUTPUT));
    ESP_ERROR_CHECK(gpio_set_level(LED_PIN, 0));

    // TASK 2: SMA buffer initialization
    int samples[SMA_WINDOW_SIZE] = {0};
    int sample_index = 0;
    int sample_count = 0;
    int sum = 0;

    // TASK 3: LED state
    bool led_state = false;

    printf("LDR + SMA + Hysteresis started\n");

    while (1)
    {
        // TASK 1: Read ADC value
        int adc_value = 0;

        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                LDR_CHANNEL,
                &adc_value
            )
        );

        // TASK 2: Update SMA buffer
        if (sample_count == SMA_WINDOW_SIZE)
        {
            sum -= samples[sample_index];
        }
        else
        {
            sample_count++;
        }

        samples[sample_index] = adc_value;
        sum += adc_value;

        sample_index = (sample_index + 1) % SMA_WINDOW_SIZE;

        // TASK 2: Calculate SMA
        int sma_value = sum / sample_count;

        // TASK 3: Hysteresis control
        if (sma_value < DARK_THRESHOLD)
        {
            led_state = true;
        }
        else if (sma_value > LIGHT_THRESHOLD)
        {
            led_state = false;
        }

        // TASK 3: Update LED
        ESP_ERROR_CHECK(
            gpio_set_level(LED_PIN, led_state)
        );

        // TASK 4: Print measurements
        printf(
            "ADC: %d | SMA: %d | LED: %s\n",
            adc_value,
            sma_value,
            led_state ? "ON" : "OFF"
        );

        vTaskDelay(pdMS_TO_TICKS(SAMPLE_INTERVAL_MS));
    }
}