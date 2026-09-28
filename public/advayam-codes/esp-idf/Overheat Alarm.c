/*
 * =============================================================================
 * ACTIVE OVERHEAT ALARM - ESP-IDF v5+
 * =============================================================================
 *
 * Monitors an LMT87 temperature sensor connected to GPIO 1.
 *
 * Temperature >= 40 C:
 *     Buzzer GPIO 16 -> ON/OFF
 *     User LED GPIO 45 -> ON/OFF
 *
 * Temperature < 40 C:
 *     Buzzer -> OFF
 *     LED    -> OFF
 *
 * LMT87:
 *     GPIO 1
 *     ADC1 Channel 0
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
#include "esp_err.h"


/*
 * =============================================================================
 * GPIO Configuration
 * =============================================================================
 */

#define TEMP_SENSOR_PIN        1
#define TEMP_ADC_CHANNEL       ADC_CHANNEL_0

#define BUZZER_PIN             16
#define USER_LED               45


/*
 * =============================================================================
 * Temperature Configuration
 * =============================================================================
 */

#define TEMP_THRESHOLD          40.0


/*
 * =============================================================================
 * LMT87 Conversion
 * =============================================================================
 *
 * This is the conversion that was working with your board in the
 * previous LMT87 ESP-IDF program:
 *
 *     temperatureC = adc_raw * 0.03151
 *
 * The factor is specific to the LMT87/library setup used on this board.
 * =============================================================================
 */

#define LMT87_TEMP_FACTOR       0.03151


/*
 * =============================================================================
 * Alarm Timing
 * =============================================================================
 */

#define ALARM_ON_TIME_MS        200
#define ALARM_OFF_TIME_MS       200

#define NORMAL_READ_INTERVAL_MS 1000


static const char *TAG = "TEMP_ALARM";


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


    /*
     * -------------------------------------------------------------------------
     * Curve fitting calibration
     * -------------------------------------------------------------------------
     */

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Trying ADC curve fitting calibration"
        );


        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
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
    }

#endif


    /*
     * -------------------------------------------------------------------------
     * Line fitting calibration
     * -------------------------------------------------------------------------
     */

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Trying ADC line fitting calibration"
        );


        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
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


    *out_handle = handle;


    if (!calibrated) {

        ESP_LOGW(
            TAG,
            "ADC calibration unavailable; using raw ADC value"
        );
    }


    return calibrated;
}


/*
 * =============================================================================
 * GPIO Initialization
 * =============================================================================
 */

static void gpio_init(void)
{
    gpio_config_t output_config = {
        .pin_bit_mask =
            (1ULL << BUZZER_PIN) |
            (1ULL << USER_LED),

        .mode =
            GPIO_MODE_OUTPUT,

        .pull_up_en =
            GPIO_PULLUP_DISABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(&output_config)
    );


    /*
     * Make sure alarm devices start OFF.
     */

    gpio_set_level(
        BUZZER_PIN,
        0
    );

    gpio_set_level(
        USER_LED,
        0
    );
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
        .unit_id = ADC_UNIT_1,
    };


    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &adc1_handle
        )
    );


    /*
     * Configure GPIO 1 / ADC1 Channel 0.
     *
     * 12 dB attenuation gives the required input range for
     * the LMT87 voltage.
     */

    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };


    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &channel_config
        )
    );


    /*
     * Try to initialize calibration.
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
 */

static double read_temperature_c(void)
{
    int adc_raw = 0;


    /*
     * Read ADC.
     */

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &adc_raw
        )
    );


    /*
     * Keep the same temperature conversion used by the
     * working LMT87 heatmap program.
     */

    double temperatureC =
        (double)adc_raw *
        LMT87_TEMP_FACTOR;


    return temperatureC;
}


/*
 * =============================================================================
 * Alarm ON
 * =============================================================================
 */

static void alarm_on(void)
{
    gpio_set_level(
        BUZZER_PIN,
        1
    );

    gpio_set_level(
        USER_LED,
        1
    );
}


/*
 * =============================================================================
 * Alarm OFF
 * =============================================================================
 */

static void alarm_off(void)
{
    gpio_set_level(
        BUZZER_PIN,
        0
    );

    gpio_set_level(
        USER_LED,
        0
    );
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * -------------------------------------------------------------------------
     * Initialize GPIO
     * -------------------------------------------------------------------------
     */

    gpio_init();


    /*
     * -------------------------------------------------------------------------
     * Initialize ADC
     * -------------------------------------------------------------------------
     */

    adc_init();


    /*
     * -------------------------------------------------------------------------
     * Startup message
     * -------------------------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "       ACTIVE OVERHEAT ALARM"
    );

    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "LMT87 GPIO       : %d",
        TEMP_SENSOR_PIN
    );

    ESP_LOGI(
        TAG,
        "Buzzer GPIO      : %d",
        BUZZER_PIN
    );

    ESP_LOGI(
        TAG,
        "User LED GPIO    : %d",
        USER_LED
    );

    ESP_LOGI(
        TAG,
        "Temperature limit: %.1f C",
        TEMP_THRESHOLD
    );


    /*
     * -------------------------------------------------------------------------
     * Main loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        /*
         * Read temperature.
         */

        double temperatureC =
            read_temperature_c();


        /*
         * Print temperature.
         */

        ESP_LOGI(
            TAG,
            "Temperature: %.1f C",
            temperatureC
        );


        /*
         * =====================================================================
         * OVER TEMPERATURE
         * =====================================================================
         */

        if (temperatureC >= TEMP_THRESHOLD) {

            ESP_LOGW(
                TAG,
                "WARNING: OVER TEMPERATURE!"
            );


            /*
             * Alarm ON.
             */

            alarm_on();


            vTaskDelay(
                pdMS_TO_TICKS(
                    ALARM_ON_TIME_MS
                )
            );


            /*
             * Alarm OFF.
             */

            alarm_off();


            vTaskDelay(
                pdMS_TO_TICKS(
                    ALARM_OFF_TIME_MS
                )
            );
        }


        /*
         * =====================================================================
         * NORMAL TEMPERATURE
         * =====================================================================
         */

        else {

            /*
             * Ensure alarm is OFF.
             */

            alarm_off();


            /*
             * Check temperature once per second.
             */

            vTaskDelay(
                pdMS_TO_TICKS(
                    NORMAL_READ_INTERVAL_MS
                )
            );
        }
    }
}
