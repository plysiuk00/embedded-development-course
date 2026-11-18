//
// Created by Anton Plysiuk on 19.11.2026.
//

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "driver/ledc.h"

// GPIO configuration
#define POT_ADC_CHANNEL ADC_CHANNEL_0
#define SERVO_GPIO GPIO_NUM_18

// ADC calibration
// Example: 270-degree potentiometer and 180-degree servo
#define POT_ADC_MIN 0
#define POT_ADC_MAX 2730

// Servo configuration
#define SERVO_MIN_DEG 0
#define SERVO_MAX_DEG 180

#define SERVO_MIN_US 1000
#define SERVO_MAX_US 2000

#define PWM_FREQUENCY 50
#define PWM_RESOLUTION LEDC_TIMER_14_BIT

#define PWM_MAX_DUTY 16383
#define PWM_PERIOD_US 20000

// Clamp ADC value to the usable range
static int clamp_adc(int value)
{
    if (value < POT_ADC_MIN)
        return POT_ADC_MIN;

    if (value > POT_ADC_MAX)
        return POT_ADC_MAX;

    return value;
}

// Convert ADC value to servo angle
static int adc_to_angle(int adc)
{
    adc = clamp_adc(adc);

    return (adc - POT_ADC_MIN) *
           (SERVO_MAX_DEG - SERVO_MIN_DEG) /
           (POT_ADC_MAX - POT_ADC_MIN);
}

// Convert angle to PWM duty
static uint32_t angle_to_duty(int angle)
{
    uint32_t pulse_us =
        SERVO_MIN_US +
        (SERVO_MAX_US - SERVO_MIN_US) * angle / 180;

    return (uint32_t)(
        (uint64_t)pulse_us * (1 << 14) / PWM_PERIOD_US
    );
}

void app_main(void)
{
    // ADC initialization
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t adc_config = {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&adc_config, &adc_handle)
    );

    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            POT_ADC_CHANNEL,
            &channel_config
        )
    );

    // LEDC timer initialization
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    // LEDC channel initialization
    ledc_channel_config_t servo_channel = {
        .gpio_num = SERVO_GPIO,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(ledc_channel_config(&servo_channel));

    while (1)
    {
        int adc_value = 0;

        // Read potentiometer
        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                POT_ADC_CHANNEL,
                &adc_value
            )
        );

        // Calculate servo angle
        int angle = adc_to_angle(adc_value);

        // Generate PWM
        uint32_t duty = angle_to_duty(angle);

        ESP_ERROR_CHECK(
            ledc_set_duty(
                LEDC_LOW_SPEED_MODE,
                LEDC_CHANNEL_0,
                duty
            )
        );

        ESP_ERROR_CHECK(
            ledc_update_duty(
                LEDC_LOW_SPEED_MODE,
                LEDC_CHANNEL_0
            )
        );

        // Log angle from leftmost position
        printf("ADC: %d | Servo angle: %d degrees\n",
               adc_value, angle);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
