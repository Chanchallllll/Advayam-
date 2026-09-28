#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "led_strip.h"
#include "esp_random.h"
#include "esp_log.h"

#define MATRIX_PIN  7
#define NUM_LEDS    25

#define MATRIX_WIDTH  5
#define MATRIX_HEIGHT 5

static const char *TAG = "LED_MATRIX";

static led_strip_handle_t led_strip;

//====================================================
// Convert X/Y coordinates to LED index
//====================================================
static int pixelXY(int x, int y)
{
    // Flip Y axis so the rain moves upward
    y = 4 - y;

    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

//====================================================
// Rain drop position for each column
//====================================================
static int drops[5];

//====================================================
// Set RGB color
//====================================================
static void set_pixel(int index, uint8_t r, uint8_t g, uint8_t b)
{
    if (index >= 0 && index < NUM_LEDS)
    {
        led_strip_set_pixel(led_strip, index, r, g, b);
    }
}

//====================================================
// Clear matrix
//====================================================
static void clear_matrix(void)
{
    led_strip_clear(led_strip);
}

//====================================================
// Main application
//====================================================
void app_main(void)
{
    //================================================
    // LED strip configuration
    //================================================
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
        .flags = {
            .with_dma = false,
        },
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip)
    );

    //================================================
    // Clear LEDs
    //================================================
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
    ESP_ERROR_CHECK(led_strip_refresh(led_strip));

    //================================================
    // Initialize random drop positions
    //================================================
    for (int x = 0; x < MATRIX_WIDTH; x++)
    {
        drops[x] = esp_random() % 5;
    }

    ESP_LOGI(TAG, "5x5 LED rain started");

    //================================================
    // Main Loop
    //================================================
    while (1)
    {
        clear_matrix();

        //============================================
        // Draw rain drops
        //============================================
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            int y = drops[x];

            // Bright head
            if (y >= 0 && y < MATRIX_HEIGHT)
            {
                set_pixel(
                    pixelXY(x, y),
                    0,      // R
                    255,    // G
                    0       // B
                );
            }

            // Dim trail behind the head
            if (y - 1 >= 0)
            {
                set_pixel(
                    pixelXY(x, y - 1),
                    0,      // R
                    80,     // G
                    0       // B
                );
            }

            // Move upward
            drops[x]++;

            // Restart from the bottom
            if (drops[x] > 6)
            {
                drops[x] = (esp_random() % 5) - 4;
            }
        }

        //============================================
        // Send data to LEDs
        //============================================
        ESP_ERROR_CHECK(led_strip_refresh(led_strip));

        // 150 ms delay
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
