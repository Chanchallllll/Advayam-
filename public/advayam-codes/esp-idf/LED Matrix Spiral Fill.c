/*
 * =============================================================================
 * 5x5 RGB MATRIX - CLOCKWISE SPIRAL
 * =============================================================================
 *
 * Native ESP-IDF 6.0.2 version.
 *
 * Hardware:
 *   WS2812 / NeoPixel data -> GPIO 7
 *   Matrix                 -> 5x5 = 25 LEDs
 *
 * The LEDs illuminate one-by-one in a clockwise spiral,
 * starting at the top-left and moving toward the center.
 *
 * Matrix wiring:
 *
 *   20 21 22 23 24
 *   19 18 17 16 15
 *   10 11 12 13 14
 *    9  8  7  6  5
 *    0  1  2  3  4
 *
 * ESP-IDF 6.0.2
 * ESP32-S3
 */

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "led_strip_rmt.h"

#include "esp_log.h"
#include "esp_err.h"

#define MATRIX_PIN      7
#define NUM_LEDS        25

#define MATRIX_BRIGHTNESS 25

static const char *TAG = "SPIRAL";

/*
 * ---------------------------------------------------------------------------
 * LED strip
 * ---------------------------------------------------------------------------
 */

static led_strip_handle_t led_strip;


/*
 * ---------------------------------------------------------------------------
 * Convert X/Y coordinates to physical NeoPixel index.
 *
 * Matrix is wired in a serpentine pattern:
 *
 *   20 21 22 23 24
 *   19 18 17 16 15
 *   10 11 12 13 14
 *    9  8  7  6  5
 *    0  1  2  3  4
 * ---------------------------------------------------------------------------
 */

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


/*
 * ---------------------------------------------------------------------------
 * Set RGB pixel
 *
 * Adafruit_NeoPixel normally applies brightness internally.
 *
 * Here we apply the brightness ourselves because ESP-IDF's led_strip
 * driver expects raw RGB values.
 * ---------------------------------------------------------------------------
 */

static void set_pixel_xy(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }

    /*
     * Convert Arduino-style brightness (0-255)
     * to the requested matrix brightness.
     */
    red =
        ((uint16_t)red * MATRIX_BRIGHTNESS) / 255;

    green =
        ((uint16_t)green * MATRIX_BRIGHTNESS) / 255;

    blue =
        ((uint16_t)blue * MATRIX_BRIGHTNESS) / 255;

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            pixel_xy(x, y),
            red,
            green,
            blue
        )
    );
}


/*
 * ---------------------------------------------------------------------------
 * Clear matrix
 * ---------------------------------------------------------------------------
 */

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/*
 * ---------------------------------------------------------------------------
 * Show matrix
 * ---------------------------------------------------------------------------
 */

static void show_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/*
 * ---------------------------------------------------------------------------
 * Initialize WS2812 / NeoPixel
 * ---------------------------------------------------------------------------
 */

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );

    clear_matrix();

    ESP_LOGI(
        TAG,
        "5x5 RGB matrix initialized"
    );
}


/*
 * ---------------------------------------------------------------------------
 * Spiral animation
 * ---------------------------------------------------------------------------
 *
 * Clockwise spiral:
 *
 *   → → → → →
 *           ↓
 *   ← ← ←   ↓
 *   ↑   →   ↓
 *   ↑   ↓ ← ←
 *   ↑   ← ← ←
 *
 * Starting at the top-left.
 * ---------------------------------------------------------------------------
 */

static void spiral_animation(void)
{
    int left = 0;
    int right = 4;
    int top = 0;
    int bottom = 4;

    while (left <= right &&
           top <= bottom) {

        /*
         * ---------------------------------------------------------------
         * Top row: left -> right
         * ---------------------------------------------------------------
         */

        for (int x = left; x <= right; x++) {

            set_pixel_xy(
                x,
                top,
                0,
                150,
                255
            );

            show_matrix();

            vTaskDelay(
                pdMS_TO_TICKS(100)
            );
        }

        top++;


        /*
         * ---------------------------------------------------------------
         * Right column: top -> bottom
         * ---------------------------------------------------------------
         */

        for (int y = top; y <= bottom; y++) {

            set_pixel_xy(
                right,
                y,
                0,
                150,
                255
            );

            show_matrix();

            vTaskDelay(
                pdMS_TO_TICKS(100)
            );
        }

        right--;


        /*
         * ---------------------------------------------------------------
         * Bottom row: right -> left
         * ---------------------------------------------------------------
         */

        if (top <= bottom) {

            for (int x = right; x >= left; x--) {

                set_pixel_xy(
                    x,
                    bottom,
                    0,
                    150,
                    255
                );

                show_matrix();

                vTaskDelay(
                    pdMS_TO_TICKS(100)
                );
            }

            bottom--;
        }


        /*
         * ---------------------------------------------------------------
         * Left column: bottom -> top
         * ---------------------------------------------------------------
         */

        if (left <= right) {

            for (int y = bottom; y >= top; y--) {

                set_pixel_xy(
                    left,
                    y,
                    0,
                    150,
                    255
                );

                show_matrix();

                vTaskDelay(
                    pdMS_TO_TICKS(100)
                );
            }

            left++;
        }
    }
}


/*
 * ---------------------------------------------------------------------------
 * app_main
 * ---------------------------------------------------------------------------
 */

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting 5x5 clockwise spiral"
    );

    matrix_init();

    while (1) {

        /*
         * Start each animation with a completely
         * blank matrix.
         */
        clear_matrix();

        /*
         * Illuminate the LEDs one-by-one
         * in a clockwise spiral.
         */
        spiral_animation();

        /*
         * Pause after completing the spiral.
         */
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}
