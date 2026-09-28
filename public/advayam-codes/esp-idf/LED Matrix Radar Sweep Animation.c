#include <stdio.h>
#include <math.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/rmt_tx.h"
#include "led_strip.h"
#include "esp_err.h"

// --------------------------------------------------
// Configuration
// --------------------------------------------------

#define MATRIX_PIN  7
#define NUM_LEDS    25

#define MATRIX_SIZE 5

// --------------------------------------------------
// LED strip
// --------------------------------------------------

static led_strip_handle_t matrix;

// Same brightness array as the Arduino version
static uint8_t brightness[5][5];

// --------------------------------------------------
// Convert X/Y coordinate to physical LED index
// Serpentine / zig-zag layout
// --------------------------------------------------

static int pixelXY(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

// --------------------------------------------------
// Initialize WS2812 / NeoPixel strip
// --------------------------------------------------

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );

    ESP_ERROR_CHECK(led_strip_clear(matrix));
    ESP_ERROR_CHECK(led_strip_refresh(matrix));
}

// --------------------------------------------------
// Main radar animation
// --------------------------------------------------

static void radar_task(void *arg)
{
    float angle = 0;

    while (1)
    {
        // ------------------------------------------
        // Fade previous trail
        // ------------------------------------------

        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                brightness[y][x] =
                    brightness[y][x] > 25
                    ? brightness[y][x] - 25
                    : 0;
            }
        }

        // ------------------------------------------
        // Calculate radar endpoint
        // ------------------------------------------

        int x =
            2 + roundf(cosf(angle) * 2);

        int y =
            2 + roundf(sinf(angle) * 2);

        // ------------------------------------------
        // Light pixels from center to endpoint
        // ------------------------------------------

        for (int step = 0; step <= 10; step++)
        {
            int px =
                2 + roundf(
                    (x - 2) * step / 10.0f
                );

            int py =
                2 + roundf(
                    (y - 2) * step / 10.0f
                );

            if (px >= 0 && px < 5 &&
                py >= 0 && py < 5)
            {
                brightness[py][px] = 100;
            }
        }

        // ------------------------------------------
        // Clear LED strip
        // ------------------------------------------

        ESP_ERROR_CHECK(led_strip_clear(matrix));

        // ------------------------------------------
        // Draw 5x5 matrix
        //
        // Arduino:
        // matrix.Color(0, brightness, 0)
        //
        // So this remains GREEN only.
        // ------------------------------------------

        for (int yy = 0; yy < 5; yy++)
        {
            for (int xx = 0; xx < 5; xx++)
            {
                int index = pixelXY(xx, yy);

                uint8_t green = brightness[yy][xx];

                // led_strip_set_pixel uses RGB
                ESP_ERROR_CHECK(
                    led_strip_set_pixel(
                        matrix,
                        index,
                        0,          // Red
                        green,      // Green
                        0           // Blue
                    )
                );
            }
        }

        // ------------------------------------------
        // Send data to WS2812
        // ------------------------------------------

        ESP_ERROR_CHECK(led_strip_refresh(matrix));

        // ------------------------------------------
        // Same as:
        // angle += 0.2;
        // delay(60);
        // ------------------------------------------

        angle += 0.2f;

        vTaskDelay(pdMS_TO_TICKS(60));
    }
}

// --------------------------------------------------
// ESP-IDF entry point
// --------------------------------------------------

void app_main(void)
{
    matrix_init();

    xTaskCreate(
        radar_task,
        "radar_task",
        4096,
        NULL,
        5,
        NULL
    );
}
