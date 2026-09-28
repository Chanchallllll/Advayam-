/*
  Matrix Column Scanner

  Each button press moves a solid vertical bar
  one column to the right.

  After the fifth column, it returns to the first.

  Includes software button debouncing.
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
#define POWER_PIN          16
#define BUTTON_PIN         17
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)
#define DEBOUNCE_TIME_MS   50

static const char *TAG = "column_scanner";
static led_strip_handle_t led_strip;

// Serpentine wiring coordinate conversion
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Draw the vertical bar column
static void draw_column(int col) {
    // Clear matrix
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    // Apply brightness scaling to color (R: 0, G: 150, B: 255)
    uint32_t r = (0 * MATRIX_BRIGHTNESS) / 255;
    uint32_t g = (150 * MATRIX_BRIGHTNESS) / 255;
    uint32_t b = (255 * MATRIX_BRIGHTNESS) / 255;

    for (int y = 0; y < 5; y++) {
        int idx = pixel_xy(col, y);
        led_strip_set_pixel(led_strip, idx, r, g, b);
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

    // 2. Configure Button Pin (GPIO 17) as input with pull-up
    gpio_config_t button_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&button_conf);

    // 3. Configure LED Strip via RMT
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

    int column = 0;
    int last_button_reading = 1; // HIGH (due to pull-up)
    int button_state = 1;
    uint32_t last_debounce_time = 0;

    draw_column(column);

    ESP_LOGI(TAG, "Column Scanner Initialized.");
    ESP_LOGI(TAG, "Press button to move. Current Column: %d", column + 1);

    // 4. Main Loop
    while (1) {
        int reading = gpio_get_level(BUTTON_PIN);
        uint32_t current_time_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

        // Check if raw button reading changed
        if (reading != last_button_reading) {
            last_debounce_time = current_time_ms;
        }

        // If the reading has stayed stable longer than debounce time
        if ((current_time_ms - last_debounce_time) > DEBOUNCE_TIME_MS) {
            if (reading != button_state) {
                button_state = reading;

                // Button pressed (active LOW with pull-up)
                if (button_state == 0) {
                    column++;
                    if (column >= 5) {
                        column = 0;
                    }

                    draw_column(column);
                    ESP_LOGI(TAG, "Column: %d", column + 1);
                }
            }
        }

        last_button_reading = reading;
        vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
    }
}