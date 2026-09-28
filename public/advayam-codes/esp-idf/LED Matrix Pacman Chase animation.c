/*
 * =============================================================================
 * PACMAN-STYLE CHASE ANIMATION
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * A yellow Pacman pixel is chased by a red ghost pixel across
 * the 5x5 WS2812 RGB matrix.
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
 *
 * in the Arduino version.
 */
#define MATRIX_BRIGHTNESS    25


static const char *TAG = "PACMAN";

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
 * LED Strip Initialization
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
     * Start with the matrix turned off.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "5x5 RGB matrix initialized on GPIO %d",
        MATRIX_PIN
    );
}


/* =============================================================================
 * Set Pixel
 * =============================================================================
 *
 * Applies brightness scaling manually because ESP-IDF's led_strip component
 * does not provide Adafruit_NeoPixel's setBrightness() function.
 * =============================================================================
 */

static void set_pixel(
    int pixel,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    /*
     * Scale RGB values to approximately match:
     *
     *     matrix.setBrightness(25)
     */
    uint8_t scaled_red =
        (uint8_t)(
            (red * MATRIX_BRIGHTNESS) / 255
        );

    uint8_t scaled_green =
        (uint8_t)(
            (green * MATRIX_BRIGHTNESS) / 255
        );

    uint8_t scaled_blue =
        (uint8_t)(
            (blue * MATRIX_BRIGHTNESS) / 255
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
 * Draw Scene
 * =============================================================================
 */

static void draw_scene(
    int pacman,
    int ghost
)
{
    /*
     * Clear the previous frame.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    /*
     * Pacman:
     *
     * Yellow
     * RGB = 255, 200, 0
     *
     * Positioned on row 2.
     */
    set_pixel(
        pixel_xy(pacman, 2),
        255,
        200,
        0
    );


    /*
     * Ghost:
     *
     * Red
     * RGB = 255, 0, 0
     *
     * Positioned on row 2.
     */
    set_pixel(
        pixel_xy(ghost, 2),
        255,
        0,
        0
    );


    /*
     * Send frame to the LEDs.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
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
    ESP_LOGI(TAG, " Pacman-Style Chase Animation");
    ESP_LOGI(TAG, "==============================================");


    /*
     * Initialize the WS2812 matrix.
     */
    led_strip_init_board();


    ESP_LOGI(
        TAG,
        "Starting animation..."
    );


    while (1) {

        /* ---------------------------------------------------------------------
         * Move from left to right
         * ---------------------------------------------------------------------
         *
         * Arduino:
         *
         *     for (int x = 0; x < 5; x++)
         *     {
         *         int ghost = max(0, x - 2);
         *     }
         *
         */

        for (int x = 0; x < 5; x++) {

            int ghost =
                (x - 2 > 0)
                ? (x - 2)
                : 0;


            draw_scene(
                x,
                ghost
            );


            vTaskDelay(
                pdMS_TO_TICKS(250)
            );
        }


        /* ---------------------------------------------------------------------
         * Return from right to left
         * ---------------------------------------------------------------------
         *
         * Arduino:
         *
         *     for (int x = 4; x >= 0; x--)
         *     {
         *         int ghost = min(4, x + 2);
         *     }
         *
         */

        for (int x = 4; x >= 0; x--) {

            int ghost =
                (x + 2 < 4)
                ? (x + 2)
                : 4;


            draw_scene(
                x,
                ghost
            );


            vTaskDelay(
                pdMS_TO_TICKS(250)
            );
        }
    }
}
