/*
 * =============================================================================
 * LMT87 TEMPERATURE HIGH/LOW LATCH - ESP-IDF v5+
 * =============================================================================
 *
 * Monitors temperature using an LMT87 connected to GPIO 1.
 *
 * Keeps track of:
 *
 *   - Current temperature
 *   - Minimum temperature since startup
 *   - Maximum temperature since startup
 *
 * New maximum:
 *   GPIO 45 -> one long blink
 *
 * New minimum:
 *   GPIO 45 -> two short blinks
 *
 * The minimum and maximum are stored in RAM and reset when the board
 * restarts.
 *
 * LMT87:
 *   GPIO 1
 *   ADC1 Channel 0
 *
 * LED:
 *   GPIO 45
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define TEMP_SENSOR_PIN     GPIO_NUM_1
#define TEMP_ADC_CHANNEL    ADC_CHANNEL_0

#define USER_LED             GPIO_NUM_45

/*
 * ADC configuration.
 *
 * This matches the working LMT87 program.
 */
#define ADC_ATTENUATION      ADC_ATTEN_DB_12
#define ADC_BITWIDTH         ADC_BITWIDTH_DEFAULT


static const char *TAG = "TEMP_HIGH_LOW";


/*
 * =============================================================================
 * ADC Handles
 * =============================================================================
 */

static adc_oneshot_unit_handle_t adc1_handle = NULL;

static adc_cali_handle_t adc_cali_handle = NULL;

static bool adc_calibrated = false;


/*
 * =============================================================================
 * ADC Calibration
 * =============================================================================
 */

static bool adc_calibration_init(
    adc_unit_t unit,
    adc_channel_t channel,
    adc_cali_handle_t *out_handle
)
{
    adc_cali_handle_t handle = NULL;

    esp_err_t ret = ESP_FAIL;

    bool calibrated = false;


#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

    /*
     * Try Curve Fitting calibration first.
     */
    ESP_LOGI(
        TAG,
        "Trying ADC curve fitting calibration..."
    );

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BITWIDTH
    };

    ret =
        adc_cali_create_scheme_curve_fitting(
            &cali_config,
            &handle
        );

    if (ret == ESP_OK) {

        calibrated = true;

        ESP_LOGI(
            TAG,
            "ADC curve fitting calibration enabled"
        );
    }

#endif


#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

    /*
     * If curve fitting isn't available, try line fitting.
     */
    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Trying ADC line fitting calibration..."
        );

        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTENUATION,
            .bitwidth = ADC_BITWIDTH
        };

        ret =
            adc_cali_create_scheme_line_fitting(
                &cali_config,
                &handle
            );

        if (ret == ESP_OK) {

            calibrated = true;

            ESP_LOGI(
                TAG,
                "ADC line fitting calibration enabled"
            );
        }
    }

#endif


    if (!calibrated) {

        ESP_LOGW(
            TAG,
            "ADC calibration unavailable"
        );
    }


    *out_handle = handle;

    return calibrated;
}


/*
 * =============================================================================
 * ADC Initialization
 * =============================================================================
 */

static void adc_init(void)
{
    /*
     * Create ADC1 unit.
     */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1
    };

    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &adc1_handle
        )
    );


    /*
     * Configure GPIO 1 / ADC1 Channel 0.
     */
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BITWIDTH
    };

    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &channel_config
        )
    );


    /*
     * Initialize calibration.
     */
    adc_calibrated =
        adc_calibration_init(
            ADC_UNIT_1,
            TEMP_ADC_CHANNEL,
            &adc_cali_handle
        );
}


/*
 * =============================================================================
 * Read LMT87 Temperature
 * =============================================================================
 *
 * This uses the same conversion that you confirmed works correctly
 * with your board:
 *
 *     temperatureC = adc_raw * 0.03151
 *
 * The calibration handle is initialized above, but the LMT87 conversion
 * itself intentionally uses the raw ADC value because this is the
 * conversion from your known-working program.
 * =============================================================================
 */

static double read_temperature_c(void)
{
    int adc_raw = 0;


    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &adc_raw
        )
    );


    /*
     * LMT87 conversion.
     *
     * This is the same formula used in your working
     * temperature heatmap program.
     */
    double temperatureC =
        adc_raw * 0.03151;


    return temperatureC;
}


/*
 * =============================================================================
 * LED Initialization
 * =============================================================================
 */

static void led_init(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask = (1ULL << USER_LED),

        .mode = GPIO_MODE_OUTPUT,

        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(
        gpio_config(&led_config)
    );


    /*
     * Make sure LED starts OFF.
     */
    gpio_set_level(
        USER_LED,
        0
    );
}


/*
 * =============================================================================
 * New Maximum Indication
 * =============================================================================
 *
 * One long blink.
 * =============================================================================
 */

static void indicate_maximum(void)
{
    /*
     * LED ON.
     */
    gpio_set_level(
        USER_LED,
        1
    );


    /*
     * Long blink: 300 ms.
     */
    vTaskDelay(
        pdMS_TO_TICKS(300)
    );


    /*
     * LED OFF.
     */
    gpio_set_level(
        USER_LED,
        0
    );
}


/*
 * =============================================================================
 * New Minimum Indication
 * =============================================================================
 *
 * Two short blinks.
 * =============================================================================
 */

static void indicate_minimum(void)
{
    for (int i = 0; i < 2; i++) {

        /*
         * LED ON.
         */
        gpio_set_level(
            USER_LED,
            1
        );


        /*
         * Short blink.
         */
        vTaskDelay(
            pdMS_TO_TICKS(100)
        );


        /*
         * LED OFF.
         */
        gpio_set_level(
            USER_LED,
            0
        );


        /*
         * Pause between flashes.
         */
        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * Initialize LED.
     */
    led_init();


    /*
     * Initialize ADC.
     */
    adc_init();


    ESP_LOGI(
        TAG,
        "--------------------------------"
    );

    ESP_LOGI(
        TAG,
        "LMT87 Temperature High/Low Latch"
    );

    ESP_LOGI(
        TAG,
        "--------------------------------"
    );

    ESP_LOGI(
        TAG,
        "LMT87: GPIO 1 / ADC1 Channel 0"
    );

    ESP_LOGI(
        TAG,
        "LED: GPIO 45"
    );


    /*
     * These variables exist for the entire runtime of the
     * application and are reset when the ESP32 restarts.
     */
    bool first_reading = true;

    double minimum_temperature = 0.0;

    double maximum_temperature = 0.0;


    /*
     * =========================================================================
     * Main Loop
     * =========================================================================
     */

    while (1) {

        /*
         * Read current temperature.
         */
        double temperatureC =
            read_temperature_c();


        /*
         * ================================================================
         * FIRST READING
         * ================================================================
         */

        if (first_reading) {

            /*
             * First reading becomes both minimum and maximum.
             */
            minimum_temperature =
                temperatureC;

            maximum_temperature =
                temperatureC;

            first_reading = false;


            ESP_LOGI(
                TAG,
                "Initial temperature recorded."
            );
        }


        /*
         * ================================================================
         * NEW MAXIMUM
         * ================================================================
         */

        else if (
            temperatureC >
            maximum_temperature
        ) {

            maximum_temperature =
                temperatureC;


            ESP_LOGI(
                TAG,
                "NEW HIGH TEMPERATURE!"
            );


            indicate_maximum();
        }


        /*
         * ================================================================
         * NEW MINIMUM
         * ================================================================
         */

        else if (
            temperatureC <
            minimum_temperature
        ) {

            minimum_temperature =
                temperatureC;


            ESP_LOGI(
                TAG,
                "NEW LOW TEMPERATURE!"
            );


            indicate_minimum();
        }


        /*
         * ================================================================
         * DISPLAY TEMPERATURE DATA
         * ================================================================
         */

        ESP_LOGI(
            TAG,
            "Current : %.1f C",
            temperatureC
        );

        ESP_LOGI(
            TAG,
            "Minimum : %.1f C",
            minimum_temperature
        );

        ESP_LOGI(
            TAG,
            "Maximum : %.1f C",
            maximum_temperature
        );


        /*
         * Wait 2 seconds before the next measurement.
         */
        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}

