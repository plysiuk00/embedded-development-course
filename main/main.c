
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_err.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

// ADC configuration
#define ADC_CHANNEL ADC_CHANNEL_0
#define ADC_ATTEN ADC_ATTEN_DB_12
#define ADC_BITWIDTH ADC_BITWIDTH_12

#define READ_INTERVAL_MS 500
#define MEASUREMENTS_COUNT 10000

#define ADC_MAX_VALUE 4095.0f
#define ADC_REFERENCE_MV 3300.0f

void app_main(void)
{
    // 1. Initialize ADC1
    adc_oneshot_unit_handle_t adc_handle;

    adc_oneshot_unit_init_cfg_t unit_config = {
        .unit_id = ADC_UNIT_1,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(&unit_config, &adc_handle)
    );

    // 2. Configure ADC channel
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc_handle,
            ADC_CHANNEL,
            &channel_config
        )
    );

    // 3. Initialize ADC calibration
    adc_cali_handle_t cali_handle;

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = ADC_UNIT_1,
        .chan = ADC_CHANNEL,
        .atten = ADC_ATTEN,
        .bitwidth = ADC_BITWIDTH,
    };

    ESP_ERROR_CHECK(
        adc_cali_create_scheme_curve_fitting(
            &cali_config,
            &cali_handle
        )
    );

    // 4. Print measurement table
    printf("\n");
    printf("RAW\tU_manual(mV)\tU_cali(mV)\tError(%%)\n");
    printf("------------------------------------------------\n");

    // 5. Perform multiple measurements
    for (int i = 0; i < MEASUREMENTS_COUNT; i++) {

        int raw_value = 0;
        int calibrated_voltage = 0;

        // Read RAW ADC value
        ESP_ERROR_CHECK(
            adc_oneshot_read(
                adc_handle,
                ADC_CHANNEL,
                &raw_value
            )
        );

        // Calculate voltage manually
        float manual_voltage =
            (raw_value / ADC_MAX_VALUE) * ADC_REFERENCE_MV;

        // Read calibrated voltage
        ESP_ERROR_CHECK(
            adc_cali_raw_to_voltage(
                cali_handle,
                raw_value,
                &calibrated_voltage
            )
        );

        // Calculate error percentage
        float difference = manual_voltage - calibrated_voltage;

        if (difference < 0) {
            difference = -difference;
        }

        float error = 0;

        if (calibrated_voltage != 0) {
            error = (difference / calibrated_voltage) * 100.0f;
        }

        // Print measurement
        printf(
            "%d\t%.1f\t\t%d\t\t%.2f\n",
            raw_value,
            manual_voltage,
            calibrated_voltage,
            error
        );

        vTaskDelay(pdMS_TO_TICKS(READ_INTERVAL_MS));
    }

    // 6. Delete calibration resources
    ESP_ERROR_CHECK(
        adc_cali_delete_scheme_curve_fitting(cali_handle)
    );

    ESP_ERROR_CHECK(
        adc_oneshot_del_unit(adc_handle)
    );
}
