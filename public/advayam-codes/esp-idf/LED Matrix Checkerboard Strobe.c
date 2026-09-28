/*
 * This program alternates two complementary colors across the
 * 5x5 RGB matrix in a checkerboard pattern, creating a strobe
 * effect.
 */
 #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  15 // Scale (0 - 255)

static const char *TAG = "checkerboard_strobe";
static led_strip_handle_t led_strip;

static int pixel_xy(int x, int y) {
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

static void draw_checkerboard(bool inverted) {
    // Clear matrix first
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            bool first_color = ((x + y) % 2 == 0);

            if (inverted)
                first_color = !first_color;

            uint8_t r, g, b;
            if (first_color) {
                r = 255;
                g = 0;
                b = 100;
            } else {
                r = 0;
                g = 255;
                b = 255;
            }

            // Apply brightness scaling
            r = (r * MATRIX_BRIGHTNESS) / 255;
            g = (g * MATRIX_BRIGHTNESS) / 255;
            b = (b * MATRIX_BRIGHTNESS) / 255;

            int idx = pixel_xy(x, y);
            led_strip_set_pixel(led_strip, idx, r, g, b);
        }
    }

    led_strip_refresh(led_strip);
}

void app_main(void) {
    // Configure LED Strip via RMT
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);

    ESP_LOGI(TAG, "Checkerboard Strobe initialized.");

    // Main Loop
    while (1) {
        // First checkerboard arrangement
        draw_checkerboard(false);
        vTaskDelay(pdMS_TO_TICKS(300));

        // Swap the colors
        draw_checkerboard(true);
        vTaskDelay(pdMS_TO_TICKS(300));
    }
}