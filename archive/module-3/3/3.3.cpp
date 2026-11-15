
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/ledc.h"

#define MOTOR_GPIO GPIO_NUM_4
#define LED_GPIO GPIO_NUM_18

#define POT_CHANNEL ADC_CHANNEL_0  // GPIO1

#define PWM_FREQ 20000
#define PWM_RESOLUTION LEDC_TIMER_10_BIT
#define PWM_MAX 1023

void app_main(void)
{
    // 1. Налаштування ADC для потенціометра
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(adc_oneshot_new_unit(&adc_config, &adc_handle));

    adc_oneshot_chan_cfg_t adc_channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };

    ESP_ERROR_CHECK(adc_oneshot_config_channel(
        adc_handle,
        POT_CHANNEL,
        &adc_channel_config
    ));

    // 2. Налаштування PWM-таймера
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQ,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    // 3. PWM-канал мотора
    ledc_channel_config_t motor_channel = {
        .gpio_num = MOTOR_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&motor_channel));

    // 4. PWM-канал LED
    ledc_channel_config_t led_channel = {
        .gpio_num = LED_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_1,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&led_channel));

    int adc_value;

    while (1) {
        // 5. Зчитування потенціометра
        ESP_ERROR_CHECK(adc_oneshot_read(
            adc_handle,
            POT_CHANNEL,
            &adc_value
        ));

        // 6. Перетворення ADC у PWM
        int duty = (adc_value * PWM_MAX) / 4095;

        // 7. Керування швидкістю мотора
        ESP_ERROR_CHECK(ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            duty
        ));

        ESP_ERROR_CHECK(ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        ));

        // 8. Керування яскравістю LED
        ESP_ERROR_CHECK(ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_1,
            duty
        ));

        ESP_ERROR_CHECK(ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_1
        ));

        printf("ADC: %d | PWM: %d\n", adc_value, duty);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
