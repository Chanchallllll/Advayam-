/*
 * =============================================================================
 * TEMPERATURE CONTROLLED FIRE EFFECT - ESP-IDF v5+
 * =============================================================================
 *
 * LMT87 temperature sensor:
 *   GPIO 1 / ADC1 Channel 0
 *
 * RGB Matrix:
 *   GPIO 7
 *   25 WS2812 LEDs
 *
 * Behaviour:
 *   Below 30 C -> matrix OFF
 *   Above 30 C -> flame appears
 *   Higher temperature -> larger/stronger flame
 *
 * Uses the same LMT87 ADC conversion as the working temperature heatmap:
 *
 *     temperatureC = adc_raw * 0.02863;
 *
 * Matrix wiring is serpentine.
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "driver/gpio.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "led_strip.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define TEMP_ADC_CHANNEL    ADC_CHANNEL_0
#define TEMP_GPIO            1

#define POWER_PIN            16

#define MATRIX_BRIGHTNESS    25


/*
 * Flame settings.
 */

#define FLAME_START_TEMP     30.0
#define FLAME_MAX_TEMP       60.0


static const char *TAG = "TEMP_FIRE";


/*
 * =============================================================================
 * Hardware Handles
 * =============================================================================
 */

static led_strip_handle_t matrix = NULL;

static adc_oneshot_unit_handle_t adc1_handle = NULL;

static adc_cali_handle_t adc_cali_handle = NULL;


/*
 * =============================================================================
 * Fire Heat Buffer
 * =============================================================================
 */

static uint8_t heat[5][5];


/*
 * =============================================================================
 * Pixel Mapping
 * =============================================================================
 *
 * Physical matrix:
 *
 *   Row 0:  → → → → →
 *   Row 1:  ← ← ← ← ←
 *   Row 2:  → → → → →
 *   Row 3:  ← ← ← ← ←
 *   Row 4:  → → → → →
 *
 * Converts X/Y coordinates to the actual LED index.
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (x < 0 || x > 4 ||
        y < 0 || y > 4) {
        return -1;
    }

    if ((y % 2) == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


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
     * ESP-IDF curve fitting calibration.
     */

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };

    ret = adc_cali_create_scheme_curve_fitting(
        &cali_config,
        &handle
    );

    if (ret == ESP_OK) {

        ESP_LOGI(
            TAG,
            "ADC curve fitting calibration enabled"
        );

        calibrated = true;
    }

#endif


#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

    /*
     * Fall back to line fitting if curve fitting
     * is unavailable.
     */

    if (!calibrated) {

        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };

        ret = adc_cali_create_scheme_line_fitting(
            &cali_config,
            &handle
        );

        if (ret == ESP_OK) {

            ESP_LOGI(
                TAG,
                "ADC line fitting calibration enabled"
            );

            calibrated = true;
        }
    }

#endif


    *out_handle = handle;

    if (!calibrated) {

        ESP_LOGW(
            TAG,
            "ADC calibration unavailable"
        );
    }

    return calibrated;
}


/*
 * =============================================================================
 * Matrix Initialization
 * =============================================================================
 */

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,

        .led_pixel_format = LED_PIXEL_FORMAT_GRB,

        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        }
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        .resolution_hz = 10 * 1000 * 1000,

        .mem_block_symbols = 64,

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );


    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Set Pixel
 * =============================================================================
 */

static void set_pixel(
    int x,
    int y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    int index = pixel_xy(x, y);

    if (index < 0 ||
        index >= NUM_LEDS) {
        return;
    }


    /*
     * Apply global brightness.
     */

    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            index,
            r,
            g,
            b
        )
    );
}


/*
 * =============================================================================
 * Clear Matrix
 * =============================================================================
 */

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Fire Color
 * =============================================================================
 *
 * Input:
 *   0   = black
 *   255 = hottest
 *
 * Colour progression:
 *
 *   black
 *      ↓
 *   dark red
 *      ↓
 *   red
 *      ↓
 *   orange
 *      ↓
 *   yellow
 *      ↓
 *   white/yellow
 * =============================================================================
 */

static void heat_color(
    uint8_t temperature,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b
)
{
    uint16_t red = 0;
    uint16_t green = 0;
    uint16_t blue = 0;


    /*
     * 0 - 85:
     *
     * Black -> red
     */

    if (temperature < 85) {

        red = temperature * 3;

        green = 0;

        blue = 0;
    }


    /*
     * 85 - 170:
     *
     * Red -> orange/yellow
     */

    else if (temperature < 170) {

        red = 255;

        green =
            (temperature - 85) * 3;

        blue = 0;
    }


    /*
     * 170 - 255:
     *
     * Yellow -> white/yellow
     */

    else {

        red = 255;

        green = 255;

        blue =
            (temperature - 170) * 3;
    }


    /*
     * Clamp values.
     */

    if (red > 255)
        red = 255;

    if (green > 255)
        green = 255;

    if (blue > 255)
        blue = 255;


    *r = (uint8_t)red;
    *g = (uint8_t)green;
    *b = (uint8_t)blue;
}


/*
 * =============================================================================
 * Generate Fire
 * =============================================================================
 */

static void generate_fire(
    float temperatureC
)
{
    /*
     * Below 30 C:
     *
     * No flame.
     */

    if (temperatureC < FLAME_START_TEMP) {

        clear_matrix();

        return;
    }


    /*
     * Convert temperature into flame strength.
     *
     * 30 C -> approximately 60
     * 60 C -> 255
     */

    float normalized =
        (temperatureC - FLAME_START_TEMP) /
        (FLAME_MAX_TEMP - FLAME_START_TEMP);


    if (normalized < 0.0f)
        normalized = 0.0f;

    if (normalized > 1.0f)
        normalized = 1.0f;


    float strength =
        60.0f +
        normalized * 195.0f;


    /*
     * ---------------------------------------------------------
     * Cool existing heat.
     * ---------------------------------------------------------
     */

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            int cooling =
                rand() % 35;


            if (heat[y][x] > cooling) {

                heat[y][x] -= cooling;

            } else {

                heat[y][x] = 0;
            }
        }
    }


    /*
     * ---------------------------------------------------------
     * Move heat upward.
     * ---------------------------------------------------------
     *
     * Bottom row is y = 0.
     *
     * Heat moves toward y = 4.
     * ---------------------------------------------------------
     */

    for (int y = 4; y > 0; y--) {

        for (int x = 0; x < 5; x++) {

            int left =
                (x > 0) ? x - 1 : x;

            int right =
                (x < 4) ? x + 1 : x;


            heat[y][x] =
                (
                    heat[y - 1][x] +
                    heat[y - 1][left] +
                    heat[y - 1][right]
                ) / 3;
        }
    }


    /*
     * ---------------------------------------------------------
     * Generate new heat at the flame base.
     * ---------------------------------------------------------
     */

    for (int x = 0; x < 5; x++) {

        int variation =
            (rand() % 61) - 30;


        int new_heat =
            (int)strength + variation;


        if (new_heat < 0)
            new_heat = 0;

        if (new_heat > 255)
            new_heat = 255;


        heat[0][x] =
            (uint8_t)new_heat;
    }


    /*
     * ---------------------------------------------------------
     * Calculate flame height.
     * ---------------------------------------------------------
     *
     * 30 C -> 1 row
     * 60 C -> 5 rows
     * ---------------------------------------------------------
     */

    int flame_height =
        1 + (int)(
            normalized * 4.0f
        );


    if (flame_height < 1)
        flame_height = 1;

    if (flame_height > 5)
        flame_height = 5;


    /*
     * ---------------------------------------------------------
     * Draw flame.
     * ---------------------------------------------------------
     */

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    for (int y = 0;
         y < flame_height;
         y++) {

        for (int x = 0;
             x < 5;
             x++) {

            uint8_t r;
            uint8_t g;
            uint8_t b;


            heat_color(
                heat[y][x],
                &r,
                &g,
                &b
            );


            set_pixel(
                x,
                y,
                r,
                g,
                b
            );
        }
    }


    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
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
     * Configure ADC channel.
     *
     * GPIO 1 = ADC1_CHANNEL_0
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
     * Initialize calibration.
     *
     * This is useful for ADC diagnostics, but the temperature
     * conversion below intentionally uses the same raw ADC
     * formula as your working LMT87 heatmap program.
     */

    adc_calibration_init(
        ADC_UNIT_1,
        TEMP_ADC_CHANNEL,
        &adc_cali_handle
    );
}


/*
 * =============================================================================
 * Main
 * =============================================================================
 */

void app_main(void)
{
    /*
     * ---------------------------------------------------------
     * GPIO 16
     * ---------------------------------------------------------
     *
     * Same configuration as your working temperature program.
     */

    gpio_config_t power_config = {
        .pin_bit_mask =
            (1ULL << POWER_PIN),

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
        gpio_config(&power_config)
    );


    gpio_set_level(
        POWER_PIN,
        0
    );


    /*
     * ---------------------------------------------------------
     * Initialize matrix.
     * ---------------------------------------------------------
     */

    matrix_init();


    /*
     * ---------------------------------------------------------
     * Initialize ADC.
     * ---------------------------------------------------------
     */

    adc_init();


    /*
     * Seed random generator using ADC noise.
     */

    int random_seed = 0;

    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &random_seed
        )
    );


    srand(
        (unsigned int)random_seed
    );


    /*
     * Clear fire buffer.
     */

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            heat[y][x] = 0;
        }
    }


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Temperature Controlled Fire"
    );

    ESP_LOGI(
        TAG,
        "LMT87: GPIO %d / ADC1_CH0",
        TEMP_GPIO
    );

    ESP_LOGI(
        TAG,
        "Flame starts at %.1f C",
        FLAME_START_TEMP
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    /*
     * ---------------------------------------------------------
     * Main loop.
     * ---------------------------------------------------------
     */

    while (1) {

        int adc_raw = 0;


        /*
         * Read raw ADC.
         */

        esp_err_t ret =
            adc_oneshot_read(
                adc1_handle,
                TEMP_ADC_CHANNEL,
                &adc_raw
            );


        if (ret != ESP_OK) {

            ESP_LOGE(
                TAG,
                "ADC read failed: %s",
                esp_err_to_name(ret)
            );

            vTaskDelay(
                pdMS_TO_TICKS(500)
            );

            continue;
        }


        /*
         * =====================================================
         * IMPORTANT LMT87 CONVERSION
         * =====================================================
         *
         * This is deliberately the same conversion from your
         * working temperature heatmap program:
         *
         *     temperatureC = adc_raw * 0.02863
         *
         * Do NOT use the Arduino LMT87 library conversion here.
         */

        double temperatureC =
            adc_raw * 0.02861;


        /*
         * Log raw ADC as well so calibration can be checked.
         */

        ESP_LOGI(
            TAG,
            "ADC: %d | Temperature: %.1f C",
            adc_raw,
            temperatureC
        );


        /*
         * Generate the temperature-controlled flame.
         */

        generate_fire(
            (float)temperatureC
        );


        /*
         * Update every 100 ms.
         */

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }
}
