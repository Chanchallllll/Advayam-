/*
 * =============================================================================
 * LMT87 TEMPERATURE FACE - ESP-IDF v5+
 * =============================================================================
 *
 * Displays happy, neutral, or sad/frowning faces depending on temperature.
 *
 * Temperature:
 *
 *   < 15 C       -> NEUTRAL
 *   15 - 28 C    -> HAPPY
 *   28 - 35 C    -> NEUTRAL
 *   >= 35 C      -> FROWNING
 *
 * The face is displayed upside down.
 *
 * LMT87:
 *   Temperature ADC = GPIO 1
 *   ADC1 Channel 0
 *
 * RGB Matrix:
 *   Data = GPIO 7
 *   Pixels = 25
 *   WS2812 / NeoPixel
 *   GRB
 *
 * Power:
 *   GPIO 16 = LOW
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

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

#define MATRIX_PIN              7
#define NUM_LEDS                25

#define TEMP_PIN                1

#define POWER_PIN               16

/*
 * GPIO 1 = ADC1 Channel 0
 */
#define TEMP_ADC_CHANNEL        ADC_CHANNEL_0

#define MATRIX_BRIGHTNESS       25


/*
 * =============================================================================
 * Temperature thresholds
 * =============================================================================
 */

#define HAPPY_MIN_TEMP          15.0
#define HAPPY_MAX_TEMP          28.0

#define FROWN_TEMP              35.0


static const char *TAG = "TEMP_FACE";


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static led_strip_handle_t led_strip = NULL;

static adc_oneshot_unit_handle_t adc1_handle = NULL;

static adc_cali_handle_t adc_cali_handle = NULL;


/*
 * =============================================================================
 * Matrix Coordinate Mapping
 * =============================================================================
 *
 * Physical matrix:
 *
 *   Row 0:  -> -> -> -> ->
 *   Row 1:  <- <- <- <- <-
 *   Row 2:  -> -> -> -> ->
 *   Row 3:  <- <- <- <- <-
 *   Row 4:  -> -> -> -> ->
 *
 * The face itself is rotated 180 degrees when displayed.
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
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

    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Using ADC curve fitting calibration"
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
        }
    }

#endif


#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Using ADC line fitting calibration"
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
        }
    }

#endif


    *out_handle = handle;


    if (calibrated) {

        ESP_LOGI(
            TAG,
            "ADC calibration enabled"
        );

    } else {

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

        .resolution_hz =
            10 * 1000 * 1000,

        .mem_block_symbols = 64,

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );


    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
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
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {

        return;
    }


    int index =
        pixel_xy(x, y);


    /*
     * Apply global brightness.
     */
    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            index,
            r,
            g,
            b
        )
    );
}


/*
 * =============================================================================
 * Face Patterns
 * =============================================================================
 *
 * 5x5 patterns.
 *
 * Bit 4 = leftmost pixel
 * Bit 0 = rightmost pixel
 *
 * =============================================================================
 */

static const uint8_t faces[3][5] =
{
    /*
     * =========================================================
     * HAPPY
     * =========================================================
     *
     *  . # . # .
     *  . . . . .
     *  . . . . .
     *  # . . . #
     *  . # # # .
     *
     */
    {
        0b01010,
        0b00000,
        0b00000,
        0b10001,
        0b01110
    },


    /*
     * =========================================================
     * NEUTRAL
     * =========================================================
     *
     *  . # . # .
     *  . . . . .
     *  . . . . .
     *  . . . . .
     *  # # # # #
     *
     */
    {
        0b01010,
        0b00000,
        0b00000,
        0b00000,
        0b11111
    },


    /*
     * =========================================================
     * FROWNING
     * =========================================================
     *
     *  . # . # .
     *  . . . . .
     *  . . . . .
     *  . # # # .
     *  # . . . #
     *
     */
    {
        0b01010,
        0b00000,
        0b00000,
        0b01110,
        0b10001
    }
};


/*
 * =============================================================================
 * Draw Face
 * =============================================================================
 *
 * The face is rotated 180 degrees.
 *
 * =============================================================================
 */

static void draw_face(int face)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            /*
             * Check whether this pixel is part
             * of the face pattern.
             */
            if (faces[face][y] &
                (1 << (4 - x))) {


                /*
                 * Rotate 180 degrees.
                 */
                int flipped_x =
                    4 - x;

                int flipped_y =
                    4 - y;


                /*
                 * Yellow face.
                 */
                set_pixel(
                    flipped_x,
                    flipped_y,

                    255,    // Red
                    190,    // Green
                    0       // Blue
                );
            }
        }
    }


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/*
 * =============================================================================
 * Temperature Reading
 * =============================================================================
 *
 * IMPORTANT:
 *
 * This uses the exact conversion used by your working
 * LMT87 heatmap program:
 *
 *     temperatureC = adc_raw * 0.03151;
 *
 * Do NOT convert the calibrated millivolt value here.
 *
 * The LMT87 library/reference code you provided is
 * effectively using the ADC reading with this conversion
 * factor for your hardware configuration.
 *
 * =============================================================================
 */

static double read_temperature(void)
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
     * LMT87 conversion used by your working program.
     */
    double temperatureC =
        adc_raw * 0.03151;


    ESP_LOGI(
        TAG,
        "ADC raw: %d | Temperature: %.1f C",
        adc_raw,
        temperatureC
    );


    return temperatureC;
}


/*
 * =============================================================================
 * Determine Face
 * =============================================================================
 */

static int temperature_to_face(double temperatureC)
{
    /*
     * Too cold:
     * neutral
     */
    if (temperatureC < HAPPY_MIN_TEMP) {

        return 1;
    }


    /*
     * Comfortable:
     * happy
     */
    else if (temperatureC < HAPPY_MAX_TEMP) {

        return 0;
    }


    /*
     * Warm:
     * neutral
     */
    else if (temperatureC < FROWN_TEMP) {

        return 1;
    }


    /*
     * Hot:
     * frowning
     */
    else {

        return 2;
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
     * =========================================================
     * GPIO 16
     * =========================================================
     *
     * Keep this LOW, matching your working LMT87 program.
     */
    gpio_config_t power_conf = {

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
        gpio_config(&power_conf)
    );


    ESP_ERROR_CHECK(
        gpio_set_level(
            POWER_PIN,
            0
        )
    );


    /*
     * =========================================================
     * Initialize RGB matrix
     * =========================================================
     */

    matrix_init();


    /*
     * =========================================================
     * Initialize ADC1
     * =========================================================
     */

    adc_oneshot_unit_init_cfg_t init_config1 = {

        .unit_id =
            ADC_UNIT_1,

        .ulp_mode =
            ADC_ULP_MODE_DISABLE,
    };


    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config1,
            &adc1_handle
        )
    );


    /*
     * Configure GPIO 1 / ADC1 Channel 0.
     */
    adc_oneshot_chan_cfg_t adc_config = {

        .atten =
            ADC_ATTEN_DB_12,

        .bitwidth =
            ADC_BITWIDTH_DEFAULT,
    };


    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            TEMP_ADC_CHANNEL,
            &adc_config
        )
    );


    /*
     * =========================================================
     * ADC Calibration
     * =========================================================
     *
     * Calibration is initialized so the ADC setup matches
     * the working reference program.
     *
     * The temperature calculation itself still uses the
     * raw ADC value *0.03151;
     */

    adc_calibration_init(
        ADC_UNIT_1,
        TEMP_ADC_CHANNEL,
        &adc_cali_handle
    );


    /*
     * Initial display.
     */
    draw_face(1);


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       LMT87 TEMPERATURE FACE"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Temperature ADC: GPIO %d",
        TEMP_PIN
    );

    ESP_LOGI(
        TAG,
        "Matrix: GPIO %d",
        MATRIX_PIN
    );


    /*
     * =========================================================
     * Main Loop
     * =========================================================
     */

    while (1) {

        /*
         * Read temperature.
         */
        double temperatureC =
            read_temperature();


        /*
         * Determine appropriate face.
         */
        int face =
            temperature_to_face(
                temperatureC
            );


        /*
         * Draw face.
         */
        draw_face(face);


        /*
         * Debug information.
         */
        const char *face_name;

        if (face == 0) {

            face_name = "HAPPY";

        } else if (face == 1) {

            face_name = "NEUTRAL";

        } else {

            face_name = "FROWNING";
        }


        ESP_LOGI(
            TAG,
            "Temperature: %.1f C | Face: %s",
            temperatureC,
            face_name
        );


        /*
         * Update once per second.
         */
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}
