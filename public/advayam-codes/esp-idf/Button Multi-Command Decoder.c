/*
 * This program detects single, double, and triple button clicks
 * using a button connected to GPIO 17.
 *
 * Depending on the number of clicks, the LED connected to GPIO 45
 * blinks the same number of times:
 *   - Single click  -> 1 blink
 *   - Double click  -> 2 blinks
 *   - Triple click  -> 3 blinks
 *
 * The program uses a short debounce delay to prevent accidental
 * multiple detections caused by mechanical button bouncing.
 * It also waits for a short period after a click to determine
 * whether another click follows.
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN     17
#define USER_LED       45
#define EXTRA_PIN      16

#define DEBOUNCE_TIME  30
#define CLICK_TIMEOUT  400

static const char *TAG = "click_detector";

void blinkLED(int count) {
    for (int i = 0; i < count; i++) {
        gpio_set_level(USER_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(USER_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void app_main(void) {
    // 1. Configure GPIOs
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    gpio_config_t out_conf = {
        .pin_bit_mask = (1ULL << USER_LED) | (1ULL << EXTRA_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&out_conf);

    gpio_set_level(USER_LED, 0);
    gpio_set_level(EXTRA_PIN, 0);

    ESP_LOGI(TAG, "Multi-Click Detector Initialized.");

    bool last_button_state = true; // HIGH (pull-up)
    int click_count = 0;
    uint64_t last_click_time = 0;

    // 2. Main Event Loop
    while (1) {
        bool button_state = (gpio_get_level(BUTTON_PIN) == 1);
        uint64_t current_time_ms = esp_timer_get_time() / 1000;

        // Detect button press (HIGH to LOW transition)
        if (last_button_state && !button_state) {
            vTaskDelay(pdMS_TO_TICKS(DEBOUNCE_TIME));

            // Confirm button press
            if (gpio_get_level(BUTTON_PIN) == 0) {
                click_count++;
                last_click_time = current_time_ms;

                // Wait for release
                while (gpio_get_level(BUTTON_PIN) == 0) {
                    vTaskDelay(pdMS_TO_TICKS(1));
                }
            }
        }

        last_button_state = button_state;

        // Wait for additional clicks timeout
        if (click_count > 0 && (current_time_ms - last_click_time > CLICK_TIMEOUT)) {
            if (click_count == 1) {
                blinkLED(1);
            } else if (click_count == 2) {
                blinkLED(2);
            } else {
                blinkLED(3);
            }
            click_count = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(5));
    }
}