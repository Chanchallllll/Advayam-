/*
 * This program creates a flickering fire effect on the 5x5
 * RGB LED matrix using a simulated heat map.
 *
 * Heat is converted into red, orange, and yellow colors.
 */
 #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_random.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define POWER_PIN          16
#define MATRIX_BRIGHTNESS  15 // Scale (0 - 255)

static const char *TAG = "fire_effect";
static led_strip_handle_t led_strip;

static uint8_t heat[5][5];

// Serpentine wiring coordinate conversion
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Convert heat value to fire color with global brightness scaling
static void heat_color(uint8_t temperature, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint32_t base_r = 0, base_g = 0, base_b = 0;

    if (temperature < 85) {
        base_r = temperature * 3;
        base_g = 0;
        base_b = 0;
    } else if (temperature < 170) {
        uint8_t green = (temperature - 85) * 3;
        base_r = 255;
        base_g = green;
        base_b = 0;
    } else {
        base_r = 255;
        base_g = 255;
        base_b = (temperature - 170) * 3;
    }

    // Apply global brightness scaling
    *r = (base_r * MATRIX_BRIGHTNESS) / 255;
    *g = (base_g * MATRIX_BRIGHTNESS) / 255;
    *b = (base_b * MATRIX_BRIGHTNESS) / 255;
}

// Update the simulated heat
static void update_heat(void) {
    // Cool the flames
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int cooling = esp_random() % 40;
            heat[y][x] = (heat[y][x] > cooling) ? (heat[y][x] - cooling) : 0;
        }
    }

    // Move heat upward
    for (int y = 4; y > 0; y--) {
        for (int x = 0; x < 5; x++) {
            int left_x = (x > 0) ? (x - 1) : 0;
            int right_x = (x < 4) ? (x + 1) : 4;

            heat[y][x] = (heat[y - 1][x] + heat[y - 1][left_x] + heat[y - 1][right_x]) / 3;
        }
    }

    // Add new heat at the bottom (y = 0)
    for (int x = 0; x < 5; x++) {
        heat[0][x] = 160 + (esp_random() % 96); // random value between 160 and 255
    }
}

// Draw the fire
static void draw_fire(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            uint8_t r, g, b;
            heat_color(heat[y][x], &r, &g, &b);
            int idx = pixel_xy(x, y);
            led_strip_set_pixel(led_strip, idx, r, g, b);
        }
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

    // Initialize heat map array
    memset(heat, 0, sizeof(heat));

    ESP_LOGI(TAG, "Fire Effect Initialized.");

    // 3. Main Fire Loop
    while (1) {
        update_heat();
        draw_fire();
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}