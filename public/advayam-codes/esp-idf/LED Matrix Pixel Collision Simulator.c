/*
 * =============================================================================
 * 5x5 RGB MATRIX - COLLISION / EXPLOSION ANIMATION
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Matrix:
 *     Data = GPIO 7
 *     LEDs = 25
 *
 * Animation:
 *
 *     Two white pixels move toward the center.
 *     They collide.
 *     A colorful explosion expands from the center.
 *     The explosion then fades out.
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_err.h"
#include "esp_log.h"

#include "led_strip.h"


/* =============================================================================
 * Hardware
 * =============================================================================
 */

#define MATRIX_PIN      7
#define NUM_LEDS        25

#define NORMAL_BRIGHTNESS   25


static const char *TAG = "COLLISION";

static led_strip_handle_t led_strip = NULL;


/* =============================================================================
 * Matrix Coordinate Mapping
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
 * Set Pixel With Brightness Scaling
 * =============================================================================
 *
 * ESP-IDF's led_strip component does not provide the equivalent of:
 *
 *     matrix.setBrightness()
 *
 * Therefore brightness is applied to the RGB values here.
 *
 * =============================================================================
 */

static void set_pixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue,
    uint8_t brightness
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }


    /*
     * Scale RGB values according to brightness.
     *
     * brightness:
     *
     *     0   = off
     *     255 = full brightness
     */
    uint8_t scaled_red =
        (uint8_t)(
            ((uint16_t)red * brightness) / 255
        );


    uint8_t scaled_green =
        (uint8_t)(
            ((uint16_t)green * brightness) / 255
        );


    uint8_t scaled_blue =
        (uint8_t)(
            ((uint16_t)blue * brightness) / 255
        );


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            pixel_xy(x, y),
            scaled_red,
            scaled_green,
            scaled_blue
        )
    );
}


/* =============================================================================
 * Convenience Function
 * =============================================================================
 *
 * Uses the normal matrix brightness of 25.
 *
 * =============================================================================
 */

static void set_pixel_normal(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    set_pixel(
        x,
        y,
        red,
        green,
        blue,
        NORMAL_BRIGHTNESS
    );
}


/* =============================================================================
 * Clear Matrix
 * =============================================================================
 */

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


/* =============================================================================
 * Refresh Matrix
 * =============================================================================
 */

static void show_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/* =============================================================================
 * Collision Animation
 * =============================================================================
 */

static void collision(void)
{
    /*
     * ============================================================
     * TWO WHITE PIXELS MOVING TOWARD THE CENTER
     * ============================================================
     */

    for (int step = 0; step < 3; step++) {

        clear_matrix();


        /*
         * Left pixel.
         */
        int left_x = step;


        /*
         * Right pixel.
         */
        int right_x = 4 - step;


        /*
         * Left white pixel.
         */
        set_pixel_normal(
            left_x,
            2,
            255,
            255,
            255
        );


        /*
         * Right white pixel.
         */
        set_pixel_normal(
            right_x,
            2,
            255,
            255,
            255
        );


        show_matrix();


        vTaskDelay(
            pdMS_TO_TICKS(120)
        );
    }


    /*
     * ============================================================
     * COLLISION
     * ============================================================
     */

    clear_matrix();


    /*
     * Bright white flash at the center.
     */
    set_pixel_normal(
        2,
        2,
        255,
        255,
        255
    );


    show_matrix();


    /*
     * Very short collision flash.
     */
    vTaskDelay(
        pdMS_TO_TICKS(30)
    );


    /*
     * Immediately start explosion.
     */
    // explosion();
    //
    // Defined below.
}


/* =============================================================================
 * Explosion
 * =============================================================================
 */

static void explosion(void)
{
    /*
     * Color palette.
     *
     * Same colors as the Arduino version.
     */

    const uint8_t colors[7][3] = {

        /* Red */
        {255,   0,   0},

        /* Orange */
        {255,  80,   0},

        /* Yellow */
        {255, 255,   0},

        /* Green */
        {  0, 255,   0},

        /* Blue */
        {  0, 150, 255},

        /* Purple */
        { 80,   0, 255},

        /* Pink */
        {255,   0, 150}
    };


    /*
     * ============================================================
     * EXPLOSION FRAME 1 - CENTER
     * ============================================================
     */

    clear_matrix();


    set_pixel_normal(
        2,
        2,
        255,
        255,
        255
    );


    show_matrix();


    vTaskDelay(
        pdMS_TO_TICKS(30)
    );


    /*
     * ============================================================
     * EXPLOSION FRAME 2
     * ============================================================
     */

    clear_matrix();


    /*
     * Center.
     */
    set_pixel_normal(
        2, 2,
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );


    /*
     * Direct neighbours.
     */

    set_pixel_normal(
        1, 2,
        colors[1][0],
        colors[1][1],
        colors[1][2]
    );


    set_pixel_normal(
        3, 2,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    set_pixel_normal(
        2, 1,
        colors[3][0],
        colors[3][1],
        colors[3][2]
    );


    set_pixel_normal(
        2, 3,
        colors[4][0],
        colors[4][1],
        colors[4][2]
    );


    show_matrix();


    vTaskDelay(
        pdMS_TO_TICKS(50)
    );


    /*
     * ============================================================
     * EXPLOSION FRAME 3
     * ============================================================
     */

    clear_matrix();


    /*
     * Center.
     */
    set_pixel_normal(
        2, 2,
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );


    /*
     * Inner ring.
     */

    set_pixel_normal(
        1, 2,
        colors[1][0],
        colors[1][1],
        colors[1][2]
    );


    set_pixel_normal(
        3, 2,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    set_pixel_normal(
        2, 1,
        colors[3][0],
        colors[3][1],
        colors[3][2]
    );


    set_pixel_normal(
        2, 3,
        colors[4][0],
        colors[4][1],
        colors[4][2]
    );


    /*
     * Diagonal particles.
     */

    set_pixel_normal(
        1, 1,
        colors[5][0],
        colors[5][1],
        colors[5][2]
    );


    set_pixel_normal(
        3, 1,
        colors[6][0],
        colors[6][1],
        colors[6][2]
    );


    set_pixel_normal(
        1, 3,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    set_pixel_normal(
        3, 3,
        colors[3][0],
        colors[3][1],
        colors[3][2]
    );


    show_matrix();


    vTaskDelay(
        pdMS_TO_TICKS(60)
    );


    /*
     * ============================================================
     * EXPLOSION FRAME 4 - MAXIMUM EXPANSION
     * ============================================================
     */

    clear_matrix();


    /*
     * Center.
     */

    set_pixel_normal(
        2, 2,
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );


    /*
     * Middle pixels.
     */

    set_pixel_normal(
        1, 2,
        colors[1][0],
        colors[1][1],
        colors[1][2]
    );


    set_pixel_normal(
        3, 2,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    set_pixel_normal(
        2, 1,
        colors[3][0],
        colors[3][1],
        colors[3][2]
    );


    set_pixel_normal(
        2, 3,
        colors[4][0],
        colors[4][1],
        colors[4][2]
    );


    /*
     * Diagonals.
     */

    set_pixel_normal(
        1, 1,
        colors[5][0],
        colors[5][1],
        colors[5][2]
    );


    set_pixel_normal(
        3, 1,
        colors[6][0],
        colors[6][1],
        colors[6][2]
    );


    set_pixel_normal(
        1, 3,
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );


    set_pixel_normal(
        3, 3,
        colors[1][0],
        colors[1][1],
        colors[1][2]
    );


    /*
     * Outer explosion.
     */

    set_pixel_normal(
        0, 2,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    set_pixel_normal(
        4, 2,
        colors[3][0],
        colors[3][1],
        colors[3][2]
    );


    set_pixel_normal(
        2, 0,
        colors[4][0],
        colors[4][1],
        colors[4][2]
    );


    set_pixel_normal(
        2, 4,
        colors[5][0],
        colors[5][1],
        colors[5][2]
    );


    /*
     * Corners.
     */

    set_pixel_normal(
        0, 0,
        colors[6][0],
        colors[6][1],
        colors[6][2]
    );


    set_pixel_normal(
        4, 0,
        colors[0][0],
        colors[0][1],
        colors[0][2]
    );


    set_pixel_normal(
        0, 4,
        colors[1][0],
        colors[1][1],
        colors[1][2]
    );


    set_pixel_normal(
        4, 4,
        colors[2][0],
        colors[2][1],
        colors[2][2]
    );


    show_matrix();


    vTaskDelay(
        pdMS_TO_TICKS(100)
    );


    /*
     * ============================================================
     * EXPLOSION FADES
     * ============================================================
     *
     * Arduino:
     *
     *     for (int brightness = 100;
     *          brightness > 0;
     *          brightness -= 20)
     *
     * ESP-IDF implementation:
     *
     * redraw the complete explosion at progressively lower
     * brightness levels.
     *
     * ============================================================
     */

    for (
        int brightness = 100;
        brightness > 0;
        brightness -= 20
    ) {

        clear_matrix();


        /*
         * Center.
         */

        set_pixel(
            2, 2,
            colors[0][0],
            colors[0][1],
            colors[0][2],
            brightness
        );


        /*
         * Middle pixels.
         */

        set_pixel(
            1, 2,
            colors[1][0],
            colors[1][1],
            colors[1][2],
            brightness
        );


        set_pixel(
            3, 2,
            colors[2][0],
            colors[2][1],
            colors[2][2],
            brightness
        );


        set_pixel(
            2, 1,
            colors[3][0],
            colors[3][1],
            colors[3][2],
            brightness
        );


        set_pixel(
            2, 3,
            colors[4][0],
            colors[4][1],
            colors[4][2],
            brightness
        );


        /*
         * Diagonals.
         */

        set_pixel(
            1, 1,
            colors[5][0],
            colors[5][1],
            colors[5][2],
            brightness
        );


        set_pixel(
            3, 1,
            colors[6][0],
            colors[6][1],
            colors[6][2],
            brightness
        );


        set_pixel(
            1, 3,
            colors[0][0],
            colors[0][1],
            colors[0][2],
            brightness
        );


        set_pixel(
            3, 3,
            colors[1][0],
            colors[1][1],
            colors[1][2],
            brightness
        );


        /*
         * Outer explosion.
         */

        set_pixel(
            0, 2,
            colors[2][0],
            colors[2][1],
            colors[2][2],
            brightness
        );


        set_pixel(
            4, 2,
            colors[3][0],
            colors[3][1],
            colors[3][2],
            brightness
        );


        set_pixel(
            2, 0,
            colors[4][0],
            colors[4][1],
            colors[4][2],
            brightness
        );


        set_pixel(
            2, 4,
            colors[5][0],
            colors[5][1],
            colors[5][2],
            brightness
        );


        /*
         * Corners.
         */

        set_pixel(
            0, 0,
            colors[6][0],
            colors[6][1],
            colors[6][2],
            brightness
        );


        set_pixel(
            4, 0,
            colors[0][0],
            colors[0][1],
            colors[0][2],
            brightness
        );


        set_pixel(
            0, 4,
            colors[1][0],
            colors[1][1],
            colors[1][2],
            brightness
        );


        set_pixel(
            4, 4,
            colors[2][0],
            colors[2][1],
            colors[2][2],
            brightness
        );


        show_matrix();


        vTaskDelay(
            pdMS_TO_TICKS(40)
        );
    }


    /*
     * Turn everything off.
     */
    clear_matrix();
    show_matrix();
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


    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "WS2812 matrix initialized"
    );
}


/* =============================================================================
 * Main
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " 5x5 Collision / Explosion Animation");
    ESP_LOGI(TAG, "==============================================");


    /*
     * Initialize matrix.
     */
    led_strip_init_board();


    while (1) {

        /*
         * Collision.
         */
        collision();


        /*
         * The original Arduino program calls explosion()
         * immediately after the collision flash.
         *
         * Do the same here.
         */
        explosion();


        /*
         * Pause before repeating.
         */
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}
