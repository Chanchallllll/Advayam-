/*
 * This program creates a moving diagonal rainbow wave across
 * the 5x5 RGB LED matrix connected to GPIO 7.
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
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

static const char *TAG = "rainbow_wave";
static led_strip_handle_t led_strip;

// Convert coordinates to the serpentine wiring
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Simple HSV to RGB conversion (Saturation = 255, Value = 255)
static void hsv_to_rgb(uint16_t hue16, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t hue = hue16 >> 8; // Scale down 16-bit hue to 0-255
    uint8_t sector = hue / 43;
    uint8_t offset = (hue % 43) * 6;

    uint8_t p = 0;
    uint8_t q = 255 - offset;
    uint8_t t = offset;

    switch (sector) {
        case 0:  *r = 255; *g = t;   *b = p;   break;
        case 1:  *r = q;   *g = 255; *b = p;   break;
        case 2:  *r = p;   *g = 255; *b = t;   break;
        case 3:  *r = p;   *g = q;   *b = 255; break;
        case 4:  *r = t;   *g = p;   *b = 255; break;
        default: *r = 255; *g = p;   *b = q;   break;
    }
}

// Set a pixel using coordinates with brightness scaling
static void set_pixel_xy(int x, int y, uint16_t hue16) {
    if (x < 0 || x >= 5 || y < 0 || y >= 5) {
        return;
    }

    uint8_t r, g, b;
    hsv_to_rgb(hue16, &r, &g, &b);

    // Apply global brightness scaling
    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;

    int idx = pixel_xy(x, y);
    led_strip_set_pixel(led_strip, idx, r, g, b);
}

void app_main(void) {
    // 1. Configure LED Strip via RMT
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

    ESP_LOGI(TAG, "Diagonal Rainbow Wave Initialized.");

    uint8_t offset = 0;

    // 2. Main Loop
    while (1) {
        // Clear matrix before drawing frame
        for (int i = 0; i < NUM_LEDS; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }

        // Create diagonal hue movement
        for (int y = 0; y < 5; y++) {
            for (int x = 0; x < 5; x++) {
                uint8_t hue = offset + ((x + y) * 25);
                set_pixel_xy(x, y, (uint16_t)hue * 256);
            }
        }

        led_strip_refresh(led_strip);

        offset += 4;
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}