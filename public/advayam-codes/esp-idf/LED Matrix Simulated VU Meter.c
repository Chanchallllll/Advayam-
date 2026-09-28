/*
 * ESP-IDF version
 *
 * Simulates a five-bar audio VU meter on a 5x5
 * RGB matrix.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_err.h"
#include "esp_random.h"


#define MATRIX_PIN  7
#define NUM_LEDS    25

// Brightness = 15% of maximum
#define BRIGHTNESS_PERCENT 15

#define MATRIX_WIDTH  5
#define MATRIX_HEIGHT 5


static led_strip_handle_t matrix;

static int barss[5];


//====================================================
// Convert X/Y coordinates to LED index
//====================================================

static int pixelXY(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}


//====================================================
// Set RGB color with 15% brightness
//====================================================

static void setPixelColor(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    /*
     * Scale RGB values to 15% brightness.
     *
     * Example:
     *   255 -> 38
     *   80  -> 12
     *   0   -> 0
     */
    r = ((uint16_t)r * BRIGHTNESS_PERCENT) / 100;
    g = ((uint16_t)g * BRIGHTNESS_PERCENT) / 100;
    b = ((uint16_t)b * BRIGHTNESS_PERCENT) / 100;

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


//====================================================
// Draw Upside-Down VU Meter
//====================================================

static void drawMeter(void)
{
    /*
     * Equivalent to:
     *
     * matrix.clear();
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    for (int x = 0; x < 5; x++)
    {
        for (int level = 0; level < barss[x]; level++)
        {
            /*
             * Grow from TOP to BOTTOM
             */
            int y = level;

            if (level < 3)
            {
                /*
                 * Green
                 *
                 * Arduino:
                 * Color(0, 255, 0)
                 */
                setPixelColor(
                    pixelXY(x, y),
                    0,
                    255,
                    0
                );
            }
            else
            {
                /*
                 * Orange
                 *
                 * Arduino:
                 * Color(255, 80, 0)
                 */
                setPixelColor(
                    pixelXY(x, y),
                    255,
                    80,
                    0
                );
            }
        }
    }


    /*
     * Equivalent to:
     *
     * matrix.show();
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//====================================================
// Initialize RGB matrix
//====================================================

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,

        .led_model = LED_MODEL_WS2812,

        /*
         * NeoPixel:
         * NEO_GRB + NEO_KHZ800
         */
        .color_component_format =
            LED_STRIP_COLOR_COMPONENT_FMT_GRB,

        .flags = {
            .invert_out = false,
        },
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        /*
         * 10 MHz gives 100 ns resolution.
         */
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
            &matrix
        )
    );


    /*
     * Start with all LEDs off.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//====================================================
// Setup
//====================================================

void app_main(void)
{
    /*
     * Initialize the matrix.
     */
    matrix_init();


    /*
     * Equivalent to:
     *
     * randomSeed(analogRead(1));
     *
     * ESP32 has a hardware random-number generator,
     * so esp_random() provides a better seed.
     */
    srand((unsigned int)esp_random());


    /*
     * Equivalent to:
     *
     * for (int i = 0; i < 5; i++)
     *     barss[i] = 0;
     */
    for (int i = 0; i < 5; i++)
    {
        barss[i] = 0;
    }


    //================================================
    // Main Loop
    //================================================

    while (1)
    {
        for (int i = 0; i < 5; i++)
        {
            /*
             * Equivalent to:
             *
             * if (random(100) < 40)
             */
            if ((rand() % 100) < 40)
            {
                /*
                 * Arduino:
                 *
                 * random(1, 6)
                 *
                 * gives 1, 2, 3, 4, or 5.
                 */
                barss[i] =
                    (rand() % 5) + 1;
            }

            /*
             * Decay
             */
            else if (barss[i] > 0)
            {
                barss[i]--;
            }
        }


        /*
         * Draw the current meter.
         */
        drawMeter();


        /*
         * Equivalent to:
         *
         * delay(120);
         */
        vTaskDelay(
            pdMS_TO_TICKS(120)
        );
    }
}
