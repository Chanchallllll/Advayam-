/*
 * This program mixes Red, Green, and Blue components on the
 * RGB matrix using the push button on GPIO 17.
 *
 * Short presses cycle through 8 brightness levels.
 * A long press confirms the selected component and moves to
 * the next color component.
 */
 #include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_timer.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)
#define BUTTON_PIN         17
#define LONG_PRESS_MS      1000

static const char *TAG = "rgb_color_mixer";

static led_strip_handle_t led_strip;

static int red_level = 0;
static int green_level = 0;
static int blue_level = 0;
static int selected_color = 0;

static bool last_button_state = true; // HIGH with pull-up
static uint64_t press_start = 0;

// Convert 0-7 level to 0-255
static int level_to_value(int level) {
    return level * 255 / 7;
}

static void update_matrix(void) {
    int r = level_to_value(red_level);
    int g = level_to_value(green_level);
    int b = level_to_value(blue_level);

    // Apply brightness scaling
    uint8_t r_val = (r * MATRIX_BRIGHTNESS) / 255;
    uint8_t g_val = (g * MATRIX_BRIGHTNESS) / 255;
    uint8_t b_val = (b * MATRIX_BRIGHTNESS) / 255;

    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, r_val, g_val, b_val);
    }
    led_strip_refresh(led_strip);
}

void app_main(void) {
    // 1. Configure Button Pin (GPIO 17) with internal pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

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

    ESP_LOGI(TAG, "RGB Color Mixer Initialized.");
    update_matrix();

    // 3. Main Event Loop
    while (1) {
        int button_level = gpio_get_level(BUTTON_PIN);
        bool button_state = (button_level == 1);

        // Button pressed (HIGH to LOW transition)
        if (last_button_state && !button_state) {
            press_start = esp_timer_get_time() / 1000;
        }

        // Button released (LOW to HIGH transition)
        if (!last_button_state && button_state) {
            uint64_t duration = (esp_timer_get_time() / 1000) - press_start;

            if (duration >= LONG_PRESS_MS) {
                // Confirm and move to next component
                selected_color++;
                if (selected_color > 2) {
                    selected_color = 0;
                }
                ESP_LOGI(TAG, "Selected Color Component: %d", selected_color);
            } else {
                // Cycle selected component level
                if (selected_color == 0) {
                    red_level++;
                    if (red_level > 7) red_level = 0;
                    ESP_LOGI(TAG, "Red Level: %d", red_level);
                } else if (selected_color == 1) {
                    green_level++;
                    if (green_level > 7) green_level = 0;
                    ESP_LOGI(TAG, "Green Level: %d", green_level);
                } else {
                    blue_level++;
                    if (blue_level > 7) blue_level = 0;
                    ESP_LOGI(TAG, "Blue Level: %d", blue_level);
                }
            }

            update_matrix();
            vTaskDelay(pdMS_TO_TICKS(30));
        }

        last_button_state = button_state;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}