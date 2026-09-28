/*
  Stopwatch on Matrix

  This program uses a push button to start and stop a stopwatch.
  The LED matrix progressively fills with one LED for each
  elapsed second.

  After 25 seconds, the display cycles through the matrix again
  while the stopwatch continues counting.

  The elapsed time is also displayed on the Serial Monitor.
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
#define BUTTON_PIN         17
#define POWER_PIN          16
#define MATRIX_BRIGHTNESS  50 // Scale (0 - 255)

static const char *TAG = "stopwatch";
static led_strip_handle_t led_strip;

static bool running = false;
static bool last_button = true; // HIGH with pull-up

static uint64_t start_ms = 0;
static uint64_t elapsed_before_pause = 0;
static unsigned long last_print = 0;

// Draw the stopwatch progress on the matrix
static void draw_stopwatch(unsigned long seconds) {
    // Clear matrix
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    int count = seconds % 25;

    // Fill one LED for each elapsed second with blue (0, 150, 255) scaled
    uint8_t blue_r = (0 * MATRIX_BRIGHTNESS) / 255;
    uint8_t blue_g = (150 * MATRIX_BRIGHTNESS) / 255;
    uint8_t blue_b = (255 * MATRIX_BRIGHTNESS) / 255;

    for (int i = 0; i < count; i++) {
        led_strip_set_pixel(led_strip, i, blue_r, blue_g, blue_b);
    }

    // Show current position in orange (255, 80, 0) scaled
    if (count < 25) {
        uint8_t orange_r = (255 * MATRIX_BRIGHTNESS) / 255;
        uint8_t orange_g = (80 * MATRIX_BRIGHTNESS) / 255;
        uint8_t orange_b = (0 * MATRIX_BRIGHTNESS) / 255;
        led_strip_set_pixel(led_strip, count, orange_r, orange_g, orange_b);
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

    // 2. Configure Button Pin (GPIO 17) with internal pull-up
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

    ESP_LOGI(TAG, "Stopwatch Initialized.");
    ESP_LOGI(TAG, "Button = Start / Stop");

    // 4. Main Event Loop
    while (1) {
        int button_level = gpio_get_level(BUTTON_PIN);
        bool button = (button_level == 1);

        // Detect a new button press (HIGH to LOW transition)
        if (last_button && !button) {
            uint64_t current_ms = esp_timer_get_time() / 1000;
            if (!running) {
                start_ms = current_ms;
                running = true;
                ESP_LOGI(TAG, "START");
            } else {
                elapsed_before_pause += current_ms - start_ms;
                running = false;
                ESP_LOGI(TAG, "STOP");
            }
            vTaskDelay(pdMS_TO_TICKS(100)); // Basic debounce
        }

        last_button = button;

        uint64_t elapsed;
        uint64_t current_ms = esp_timer_get_time() / 1000;

        if (running) {
            elapsed = elapsed_before_pause + (current_ms - start_ms);
        } else {
            elapsed = elapsed_before_pause;
        }

        unsigned long seconds = (unsigned long)(elapsed / 1000);

        draw_stopwatch(seconds);

        if (seconds != last_print) {
            ESP_LOGI(TAG, "Elapsed: %lu seconds", seconds);
            last_print = seconds;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}