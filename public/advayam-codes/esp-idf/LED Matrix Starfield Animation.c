/*
 * ============================================================
 * 5x5 RGB Matrix Starfield - ESP-IDF
 * ============================================================
 *
 * White pixels = normal stars
 * Warm yellow  = rare sunset stars
 *
 * Stars fade smoothly in and out.
 * Stars are spaced apart to avoid clustering.
 *
 * ESP32-S3
 * Matrix DATA = GPIO 7
 * Matrix size = 5x5
 * ============================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_random.h"

#include "led_strip.h"
#include "led_strip_rmt.h"

//====================================================
// Hardware
//====================================================

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define MAX_STARS 6

// Faster update interval
#define FRAME_DELAY_MS 20

static const char *TAG = "STARFIELD";

//====================================================
// LED Strip
//====================================================

static led_strip_handle_t matrix;

//====================================================
// Star Structure
//====================================================

typedef struct
{
    int x;
    int y;

    float brightness;

    float fade_amount;

    bool fading_in;

    bool active;

    bool sunset;

} Star;

static Star stars[MAX_STARS];

//====================================================
// Random Number Helper
//====================================================

static int random_int(int min, int max)
{
    return min +
           (int)(esp_random() %
                 (uint32_t)(max - min));
}

//====================================================
// Pixel Mapping
//====================================================
//
// Serpentine wiring:
//
// Row 0:  0  1  2  3  4
// Row 1:  9  8  7  6  5
// Row 2: 10 11 12 13 14
// Row 3: 19 18 17 16 15
// Row 4: 20 21 22 23 24
//
//====================================================

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

//====================================================
// Check Position
//====================================================

static bool position_available(int x, int y)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        if (!stars[i].active)
        {
            continue;
        }

        int dx = abs(x - stars[i].x);
        int dy = abs(y - stars[i].y);

        // Keep stars separated
        if (dx <= 1 && dy <= 1)
        {
            return false;
        }
    }

    return true;
}

//====================================================
// Find Random Position
//====================================================

static bool find_position(int *x, int *y)
{
    for (int attempt = 0; attempt < 50; attempt++)
    {
        int test_x = random_int(0, 5);
        int test_y = random_int(0, 5);

        if (position_available(test_x, test_y))
        {
            *x = test_x;
            *y = test_y;

            return true;
        }
    }

    return false;
}

//====================================================
// Create Star
//====================================================

static void create_star(int index)
{
    int x;
    int y;

    if (!find_position(&x, &y))
    {
        return;
    }

    stars[index].x = x;
    stars[index].y = y;

    // Start completely dark
    stars[index].brightness = 0.0f;

    // Small brightness increments
    stars[index].fade_amount =
        random_int(8, 16) / 10.0f;

    stars[index].fading_in = true;

    stars[index].active = true;

    // Approximately 10% sunset stars
    stars[index].sunset =
        (random_int(0, 100) < 10);
}

//====================================================
// Remove Star
//====================================================

static void remove_star(int index)
{
    stars[index].active = false;
}

//====================================================
// Smooth Fade
//====================================================
//
// Smoothstep easing:
//
// Starts slowly
// Speeds up in the middle
// Slows down at the end
//
//====================================================

static float smooth_fade(float x)
{
    return x * x * (3.0f - 2.0f * x);
}

//====================================================
// Update Stars
//====================================================

static void update_stars(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        //================================================
        // Create new star
        //================================================

        if (!stars[i].active)
        {
            if (random_int(0, 100) < 9)
            {
                create_star(i);
            }

            continue;
        }

        //================================================
        // Fade In
        //================================================

        if (stars[i].fading_in)
        {
            stars[i].brightness +=
                stars[i].fade_amount;

            if (stars[i].brightness >= 255.0f)
            {
                stars[i].brightness = 255.0f;

                stars[i].fading_in = false;
            }
        }

        //================================================
        // Fade Out
        //================================================

        else
        {
            stars[i].brightness -=
                stars[i].fade_amount;

            if (stars[i].brightness <= 0.0f)
            {
                stars[i].brightness = 0.0f;

                remove_star(i);
            }
        }
    }
}

//====================================================
// Draw Stars
//====================================================

static void draw_stars(void)
{
    // Clear matrix
    led_strip_clear(matrix);

    for (int i = 0; i < MAX_STARS; i++)
    {
        if (!stars[i].active)
        {
            continue;
        }

        float normalized =
            stars[i].brightness / 255.0f;

        // Apply smooth easing
        float smooth =
            smooth_fade(normalized);

        int brightness =
            (int)(smooth * 255.0f);

        int index =
            pixel_xy(
                stars[i].x,
                stars[i].y
            );

        uint32_t red;
        uint32_t green;
        uint32_t blue;

        //================================================
        // White Star
        //================================================

        if (!stars[i].sunset)
        {
            red = brightness;
            green = brightness;
            blue = brightness;
        }

        //================================================
        // Sunset Yellow
        //================================================

        else
        {
            red = brightness;
            green = (int)(brightness * 0.55f);
            blue = 0;
        }

        led_strip_set_pixel(
            matrix,
            index,
            red,
            green,
            blue
        );
    }

    // Send data to matrix
    led_strip_refresh(matrix);
}

//====================================================
// Initialize Matrix
//====================================================

static void setup_matrix(void)
{
    led_strip_config_t strip_config =
    {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,

        .led_model = LED_MODEL_WS2812,

        .flags =
        {
            .invert_out = false
        }
    };

    led_strip_rmt_config_t rmt_config =
    {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,

        .mem_block_symbols = 64,

        .flags =
        {
            .with_dma = false
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

    ESP_LOGI(TAG, "5x5 RGB matrix initialized");
}

//====================================================
// Initialize Stars
//====================================================

static void setup_stars(void)
{
    for (int i = 0; i < MAX_STARS; i++)
    {
        stars[i].active = false;
    }

    // Initial scattered stars
    for (int i = 0; i < 3; i++)
    {
        create_star(i);
    }
}

//====================================================
// Main
//====================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting 5x5 RGB Matrix Starfield"
    );

    setup_matrix();

    setup_stars();

    while (1)
    {
        update_stars();

        draw_stars();

        vTaskDelay(
            pdMS_TO_TICKS(FRAME_DELAY_MS)
        );
    }
}
