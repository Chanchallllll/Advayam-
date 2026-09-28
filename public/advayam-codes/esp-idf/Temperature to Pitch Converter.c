/*
 * =============================================================================
 * ESP-IDF v5+
 *
 * LMT87 Temperature -> Buzzer Frequency
 *
 * LMT87:
 *   OUT -> GPIO 1 / ADC1 Channel 0
 *
 * Buzzer:
 *   GPIO 16
 *
 * UART:
 *   115200 baud
 *
 * =============================================================================
 *
 * Temperature mapping:
 *
 *   20 C -> 200 Hz
 *   50 C -> 2000 Hz
 *
 * Temperatures below 20 C are clamped to 20 C.
 * Temperatures above 50 C are clamped to 50 C.
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

/* ---------------------------------------------------------------------------
 * LMT87
 * ------------------------------------------------------------------------- */

#define TEMP_PIN            GPIO_NUM_1
#define TEMP_ADC_CHANNEL    ADC_CHANNEL_0

#define ADC_ATTENUATION     ADC_ATTEN_DB_12
#define ADC_BITWIDTH        ADC_BITWIDTH_DEFAULT


/* ---------------------------------------------------------------------------
 * Buzzer
 * ------------------------------------------------------------------------- */

#define BUZZER_PIN          GPIO_NUM_16

#define BUZZER_LEDC_TIMER   LEDC_TIMER_0
#define BUZZER_LEDC_CHANNEL LEDC_CHANNEL_0

#define BUZZER_DUTY_RES     LEDC_TIMER_10_BIT

#define BUZZER_DUTY         512


/* ---------------------------------------------------------------------------
 * Temperature range
 * ------------------------------------------------------------------------- */

#define MIN_TEMP            20.0f
#define MAX_TEMP            50.0f


/* ---------------------------------------------------------------------------
 * Frequency range
 * ------------------------------------------------------------------------- */

#define MIN_FREQ            200
#define MAX_FREQ            2000


static const char *TAG = "TEMP_PITCH";


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
 * LMT87 ADC Initialization
 * =============================================================================
 */

static void lmt87_adc_init(void)
{
    /*
     * Create ADC1.
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
     * GPIO 1 / ADC1 Channel 0.
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
     *
     * Calibration is not used in the actual LMT87 temperature
     * calculation because the known-working conversion uses
     * the raw ADC value.
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
 * This is the CURRENT working LMT87 conversion:
 *
 *     temperatureC = adc_raw * 0.03151
 *
 * This replaces the Arduino:
 *
 *     -lmt87.getCDegree(...)
 *
 * =============================================================================
 */

static float read_lmt87_temperature(void)
{
    int adc_raw = 0;


    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &adc_raw
        )
    );


    float temperatureC =
        (float)adc_raw * 0.03151f;


    return temperatureC;
}


/*
 * =============================================================================
 * Map Temperature to Frequency
 * =============================================================================
 *
 * 20 C -> 200 Hz
 * 50 C -> 2000 Hz
 *
 * Temperature is clamped before mapping.
 * =============================================================================
 */

static int temperature_to_frequency(float temperatureC)
{
    /*
     * Clamp temperature.
     */
    if (temperatureC < MIN_TEMP) {
        temperatureC = MIN_TEMP;
    }


    if (temperatureC > MAX_TEMP) {
        temperatureC = MAX_TEMP;
    }


    /*
     * Linear interpolation.
     *
     * frequency =
     *
     * MIN_FREQ +
     *
     * ((temperature - MIN_TEMP)
     * /
     * (MAX_TEMP - MIN_TEMP))
     *
     * *
     *
     * (MAX_FREQ - MIN_FREQ)
     */
    float ratio =
        (temperatureC - MIN_TEMP) /
        (MAX_TEMP - MIN_TEMP);


    int frequency =
        MIN_FREQ +
        (int)(
            ratio *
            (MAX_FREQ - MIN_FREQ)
        );


    return frequency;
}


/*
 * =============================================================================
 * BUZZER / LEDC INITIALIZATION
 * =============================================================================
 *
 * Arduino:
 *
 *     tone(BUZZER_PIN, frequency);
 *
 * ESP-IDF:
 *
 *     LEDC generates the hardware PWM tone.
 *
 * The duty cycle is approximately 50%.
 * =============================================================================
 */

static void buzzer_init(void)
{
    /*
     * -------------------------------------------------------------------------
     * LEDC timer
     * -------------------------------------------------------------------------
     */

    ledc_timer_config_t timer_config = {
        .speed_mode =
            LEDC_LOW_SPEED_MODE,

        .timer_num =
            BUZZER_LEDC_TIMER,

        .duty_resolution =
            BUZZER_DUTY_RES,

        /*
         * Initial frequency.
         *
         * This will be changed dynamically.
         */
        .freq_hz =
            MIN_FREQ,

        .clk_cfg =
            LEDC_AUTO_CLK
    };


    ESP_ERROR_CHECK(
        ledc_timer_config(
            &timer_config
        )
    );


    /*
     * -------------------------------------------------------------------------
     * LEDC channel
     * -------------------------------------------------------------------------
     */

    ledc_channel_config_t channel_config = {
        .gpio_num =
            BUZZER_PIN,

        .speed_mode =
            LEDC_LOW_SPEED_MODE,

        .channel =
            BUZZER_LEDC_CHANNEL,

        .intr_type =
            LEDC_INTR_DISABLE,

        .timer_sel =
            BUZZER_LEDC_TIMER,

        .duty =
            0,

        .hpoint =
            0
    };


    ESP_ERROR_CHECK(
        ledc_channel_config(
            &channel_config
        )
    );


    /*
     * Start with buzzer OFF.
     */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            BUZZER_LEDC_CHANNEL,
            0
        )
    );


    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            BUZZER_LEDC_CHANNEL
        )
    );
}


/*
 * =============================================================================
 * Set Buzzer Frequency
 * =============================================================================
 */

static void buzzer_set_frequency(int frequency)
{
    /*
     * Change LEDC frequency.
     */
    ESP_ERROR_CHECK(
        ledc_set_freq(
            LEDC_LOW_SPEED_MODE,
            BUZZER_LEDC_TIMER,
            frequency
        )
    );


    /*
     * 50% duty cycle.
     */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            BUZZER_LEDC_CHANNEL,
            BUZZER_DUTY
        )
    );


    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            BUZZER_LEDC_CHANNEL
        )
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
     * Initialize LMT87 ADC.
     */
    lmt87_adc_init();


    /*
     * Initialize buzzer.
     */
    buzzer_init();


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Temperature-to-Pitch Converter"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "LMT87       : GPIO 1 / ADC1 CH0"
    );

    ESP_LOGI(
        TAG,
        "Buzzer      : GPIO 16"
    );

    ESP_LOGI(
        TAG,
        "Temperature : %.1f - %.1f C",
        MIN_TEMP,
        MAX_TEMP
    );

    ESP_LOGI(
        TAG,
        "Frequency   : %d - %d Hz",
        MIN_FREQ,
        MAX_FREQ
    );


    /*
     * =========================================================================
     * Main Loop
     * =========================================================================
     */

    while (1) {

        /*
         * Read LMT87.
         */
        float temperatureC =
            read_lmt87_temperature();


        /*
         * Convert temperature to frequency.
         */
        int frequency =
            temperature_to_frequency(
                temperatureC
            );


        /*
         * Update buzzer.
         */
        buzzer_set_frequency(
            frequency
        );


        /*
         * Display values.
         */
        ESP_LOGI(
            TAG,
            "Temperature: %.1f C | Frequency: %d Hz",
            temperatureC,
            frequency
        );


        /*
         * Same timing as the original Arduino program:
         *
         *     delay(250);
         */
        vTaskDelay(
            pdMS_TO_TICKS(250)
        );
    }
}
