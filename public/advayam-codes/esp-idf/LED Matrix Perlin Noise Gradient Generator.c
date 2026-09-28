/*
 * =============================================================================
 * PROCEDURAL COLOR GRADIENT
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Generates smoothly changing procedural color gradients across
 * a 5x5 WS2812 RGB matrix using interpolated procedural noise.
 *
 * Matrix:
 *
 *     Data = GPIO 7
 *     LEDs = 25
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "led_strip.h"


/* =============================================================================
 * Hardware Configuration
 * =============================================================================
 */

#define MATRIX_PIN      7
#define NUM_LEDS        25

/*
 * Equivalent to:
 *
 *     matrix.setBrightness(25);
 */
#define MATRIX_BRIGHTNESS    25


static const char *TAG = "COLOR_GRADIENT";

static led_strip_handle_t led_strip = NULL;


/* =============================================================================
 * Convert X/Y Coordinates to LED Number
 * =============================================================================
 *
 * Physical matrix:
 *
 *     20 21 22 23 24
 *     19 18 17 16 15
 *     10 11 12 13 14
 *      9  8  7  6  5
 *      0  1  2  3  4
 *
 * Serpentine wiring.
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


/* =============================================================================
 * Smooth Step
 * =============================================================================
 *
 * Same as the Arduino:
 *
 *     t * t * (3 - 2 * t)
 *
 * Currently retained for compatibility with the original program.
 * =============================================================================
 */

static float smooth_step(float t)
{
    return t * t * (3.0f - 2.0f * t);
}


/* =============================================================================
 * Procedural 2D Noise
 * =============================================================================
 *
 * This is the same mathematical function as the Arduino version.
 *
 * Result:
 *
 *     0.0 ... 1.0
 *
 * =============================================================================
 */

static float noise_2d(float x, float y)
{
    float value =
        sinf(
            x * 1.7f +
            sinf(y * 1.3f)
        )
        *
        cosf(
            y * 1.9f +
            sinf(x * 0.8f)
        );


    return (value + 1.0f) / 2.0f;
}


/* =============================================================================
 * HSV to RGB
 * =============================================================================
 *
 * Replaces Adafruit_NeoPixel::ColorHSV().
 *
 * Inputs:
 *
 *     hue        = 0 ... 65535
 *     saturation = 0 ... 255
 *     value      = 0 ... 255
 *
 * Outputs:
 *
 *     red
 *     green
 *     blue
 *
 * =============================================================================
 */

static void hsv_to_rgb(
    uint16_t hue,
    uint8_t saturation,
    uint8_t value,
    uint8_t *red,
    uint8_t *green,
    uint8_t *blue
)
{
    /*
     * Fully desaturated color = grayscale.
     */
    if (saturation == 0) {

        *red = value;
        *green = value;
        *blue = value;

        return;
    }


    /*
     * Convert 16-bit hue into a 0-360 degree hue.
     */
    float h =
        ((float)hue / 65535.0f) * 360.0f;


    float s =
        (float)saturation / 255.0f;


    float v =
        (float)value / 255.0f;


    float c =
        v * s;


    float x =
        c *
        (1.0f -
         fabsf(
             fmodf(h / 60.0f, 2.0f) - 1.0f
         ));


    float m =
        v - c;


    float r1;
    float g1;
    float b1;


    if (h < 60.0f) {

        r1 = c;
        g1 = x;
        b1 = 0.0f;

    }
    else if (h < 120.0f) {

        r1 = x;
        g1 = c;
        b1 = 0.0f;

    }
    else if (h < 180.0f) {

        r1 = 0.0f;
        g1 = c;
        b1 = x;

    }
    else if (h < 240.0f) {

        r1 = 0.0f;
        g1 = x;
        b1 = c;

    }
    else if (h < 300.0f) {

        r1 = x;
        g1 = 0.0f;
        b1 = c;

    }
    else {

        r1 = c;
        g1 = 0.0f;
        b1 = x;
    }


    *red =
        (uint8_t)(
            (r1 + m) * 255.0f
        );


    *green =
        (uint8_t)(
            (g1 + m) * 255.0f
        );


    *blue =
        (uint8_t)(
            (b1 + m) * 255.0f
        );
}


/* =============================================================================
 * Set Pixel With Brightness Scaling
 * =============================================================================
 *
 * Adafruit_NeoPixel provides:
 *
 *     matrix.setBrightness(25)
 *
 * ESP-IDF led_strip does not have an equivalent global brightness setting,
 * so RGB values are scaled before being sent to the LEDs.
 *
 * =============================================================================
 */

static void set_pixel(
    int pixel,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    uint8_t scaled_red =
        (uint8_t)(
            ((uint16_t)red *
             MATRIX_BRIGHTNESS) / 255
        );


    uint8_t scaled_green =
        (uint8_t)(
            ((uint16_t)green *
             MATRIX_BRIGHTNESS) / 255
        );


    uint8_t scaled_blue =
        (uint8_t)(
            ((uint16_t)blue *
             MATRIX_BRIGHTNESS) / 255
        );


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            pixel,
            scaled_red,
            scaled_green,
            scaled_blue
        )
    );
}


/* =============================================================================
 * Initialize LED Strip
 * =============================================================================
 */

static void led_strip_init_board(void)
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

        /*
         * 10 MHz RMT resolution.
         */
        .resolution_hz = 10 * 1000 * 1000,

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


    /*
     * Clear matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "RGB matrix initialized on GPIO %d",
        MATRIX_PIN
    );
}


/* =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " Procedural Color Gradient");
    ESP_LOGI(TAG, "==============================================");


    /*
     * Initialize WS2812 matrix.
     */
    led_strip_init_board();


    /*
     * Equivalent to:
     *
     *     static float time = 0;
     *
     * in the Arduino program.
     */
    float time_value = 0.0f;


    while (1) {

        /* ---------------------------------------------------------------------
         * Generate the 5x5 frame
         * ---------------------------------------------------------------------
         */

        for (int y = 0; y < 5; y++) {

            for (int x = 0; x < 5; x++) {

                /*
                 * Generate procedural noise.
                 *
                 * Same calculation as Arduino:
                 *
                 *     noise2D(
                 *         x * 0.6 + time,
                 *         y * 0.6 + time * 0.7
                 *     );
                 */
                float n =
                    noise_2d(
                        x * 0.6f + time_value,
                        y * 0.6f + time_value * 0.7f
                    );


                /*
                 * Convert noise value:
                 *
                 *     0.0 ... 1.0
                 *
                 * into:
                 *
                 *     0 ... 65535
                 *
                 * matching ColorHSV().
                 */
                uint16_t hue =
                    (uint16_t)(
                        n * 65535.0f
                    );


                /*
                 * HSV:
                 *
                 * Saturation = 255
                 * Value      = 255
                 */
                uint8_t red;
                uint8_t green;
                uint8_t blue;


                hsv_to_rgb(
                    hue,
                    255,
                    255,
                    &red,
                    &green,
                    &blue
                );


                /*
                 * Write pixel using the serpentine mapping.
                 */
                set_pixel(
                    pixel_xy(x, y),
                    red,
                    green,
                    blue
                );
            }
        }


        /* ---------------------------------------------------------------------
         * Display frame
         * ---------------------------------------------------------------------
         */

        ESP_ERROR_CHECK(
            led_strip_refresh(led_strip)
        );


        /* ---------------------------------------------------------------------
         * Advance animation
         * ---------------------------------------------------------------------
         *
         * Same as:
         *
         *     time += 0.05;
         *
         */

        time_value += 0.05f;


        /* ---------------------------------------------------------------------
         * Animation frame rate
         * ---------------------------------------------------------------------
         *
         * Original:
         *
         *     delay(50);
         *
         */

        vTaskDelay(
            pdMS_TO_TICKS(50)
        );
    }
}
