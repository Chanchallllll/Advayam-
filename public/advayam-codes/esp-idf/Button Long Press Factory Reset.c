/*
 * This program detects how long a button connected to GPIO 17 is held.
 *
 * A normal short press does nothing. While the button is being held,
 * the LED connected to GPIO 45 blinks progressively faster to show
 * that the reset timer is counting down.
 *
 * If the button is held for 5 seconds, a reset sound is played through
 * the buzzer on GPIO 16 and the LED flashes to indicate that the
 * factory-reset condition has been reached.
 *
 * This is a starter-kit demonstration, so no actual flash, NVS, or
 * EEPROM data is erased. The performFactoryReset() function is where
 * real application reset logic can be added.
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"

#define BUTTON_PIN 17
#define USER_LED   45
#define BUZZER_PIN 16

#define RESET_TIME 5000

static const char *TAG = "factory_reset";

void resetSound(void) {
    for (int i = 0; i < 3; i++) {
        gpio_set_level(BUZZER_PIN, 1);
        vTaskDelay(pdMS_TO_TICKS(150));
        gpio_set_level(BUZZER_PIN, 0);
        vTaskDelay(pdMS_TO_TICKS(150));
    }
}

void performFactoryReset(void) {
    ESP_LOGI(TAG, "FACTORY RESET TRIGGERED");
    resetSound();

    for (int i = 0; i < 5; i++) {
        gpio_set_level(USER_LED, 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(USER_LED, 0);
        vTaskDelay(pdMS_TO_TICKS(100));
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
        .pin_bit_mask = (1ULL << USER_LED) | (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&out_conf);

    gpio_set_level(USER_LED, 0);
    gpio_set_level(BUZZER_PIN, 0);

    ESP_LOGI(TAG, "Long Press Factory Reset Initialized.");
    ESP_LOGI(TAG, "Hold button for 5 seconds.");

    bool was_pressed = false;
    bool reset_triggered = false;
    uint64_t press_start_time = 0;
    uint64_t last_blink_time = 0;
    int led_state = 0;

    // 2. Main Event Loop
    while (1) {
        bool button_pressed = (gpio_get_level(BUTTON_PIN) == 0); // Active LOW
        uint64_t current_time_ms = esp_timer_get_time() / 1000;

        // Button Press Started
        if (button_pressed && !was_pressed) {
            press_start_time = current_time_ms;
            last_blink_time = current_time_ms;
            reset_triggered = false;
            ESP_LOGI(TAG, "Button hold started");
        }

        // Button Being Held
        if (button_pressed) {
            unsigned long hold_time = (unsigned long)(current_time_ms - press_start_time);

            unsigned long blink_interval;
            if (hold_time < 1000)
                blink_interval = 500;
            else if (hold_time < 2000)
                blink_interval = 350;
            else if (hold_time < 3000)
                blink_interval = 250;
            else if (hold_time < 4000)
                blink_interval = 150;
            else
                blink_interval = 75;

            // Blink LED progressively faster
            if ((current_time_ms - last_blink_time) >= blink_interval) {
                led_state = !led_state;
                gpio_set_level(USER_LED, led_state);
                last_blink_time = current_time_ms;
            }

            // 5 Second Reset Threshold
            if (hold_time >= RESET_TIME && !reset_triggered) {
                reset_triggered = true;
                gpio_set_level(USER_LED, 0);
                performFactoryReset();
            }
        }

        // Button Released
        if (!button_pressed && was_pressed) {
            gpio_set_level(USER_LED, 0);
            if (!reset_triggered) {
                ESP_LOGI(TAG, "Button released before 5 seconds");
            }
        }

        was_pressed = button_pressed;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}