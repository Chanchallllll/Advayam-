/*
 * This program animates expanding square frames on the 5x5
 * RGB LED matrix, starting from the center and expanding
 * toward the outer boundary.
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
#define POWER_PIN          16
#define MATRIX_BRIGHTNESS  50 // Scale (0 - 255)

static const char *TAG = "square_animation";
static led_strip_handle_t led_strip;

// Serpentine wiring coordinate conversion
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Set one LED using x/y coordinates with color and brightness scaling
static void set_pixel_xy(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x >= 5 || y < 0 || y >= 5) {
        return;
    }

    // Apply global brightness scaling
    uint8_t r_val = (r * MATRIX_BRIGHTNESS) / 255;
    uint8_t g_val = (g * MATRIX_BRIGHTNESS) / 255;
    uint8_t b_val = (b * MATRIX_BRIGHTNESS) / 255;

    int idx = pixel_xy(x, y);
    led_strip_set_pixel(led_strip, idx, r_val, g_val, b_val);
}

// Draw one square frame
static void draw_square(int size, uint8_t r, uint8_t g, uint8_t b) {
    // Clear matrix first
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    int min_pos = 2 - size;
    int max_pos = 2 + size;

    // Top and bottom edges
    for (int x = min_pos; x <= max_pos; x++) {
        set_pixel_xy(x, min_pos, r, g, b);
        set_pixel_xy(x, max_pos, r, g, b);
    }

    // Left and right edges
    for (int y = min_pos; y <= max_pos; y++) {
        set_pixel_xy(min_pos, y, r, g, b);
        set_pixel_xy(max_pos, y, r, g, b);
    }

    led_strip_refresh(led_strip);
}

void app_main(void) {
    // 1. Configure Power Pin (GPIO 16) as output and keep LOW
    gpio_config_t power_conf = {
        .pin_bit_mask = (1ULL << POWER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&power_conf);
    gpio_set_level(POWER_PIN, 0);

    // 2. Configure LED Strip via RMT
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

    ESP_LOGI(TAG, "Square Animation Initialized.");

    // 3. Main Animation Loop
    while (1) {
        // Center pixel (Red)
        draw_square(0, 255, 0, 0);
        vTaskDelay(pdMS_TO_TICKS(300));

        // 3x3 square (Green)
        draw_square(1, 0, 255, 0);
        vTaskDelay(pdMS_TO_TICKS(300));

        // 5x5 square (Blue)
        draw_square(2, 0, 0, 255);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}