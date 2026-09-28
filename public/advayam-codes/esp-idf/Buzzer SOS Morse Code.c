/*
 * This program sends the Morse code distress signal "SOS" (... --- ...)
 * through a buzzer connected to GPIO 16.
 *
 * Pressing the button connected to GPIO 17 once triggers the buzzer to
 * play the complete SOS signal. The button uses the internal pull-up
 * resistor, so it is normally HIGH and becomes LOW when pressed.
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define DOT_TIME       100
#define DASH_TIME      300
#define SYMBOL_GAP     100
#define LETTER_GAP     300

static const char *TAG = "morse_sos";

void beep(int duration) {
    gpio_set_level(BUZZER_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(duration));

    gpio_set_level(BUZZER_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(SYMBOL_GAP));
}

void sendSOS(void) {
    // S = ...
    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);

    vTaskDelay(pdMS_TO_TICKS(LETTER_GAP));

    // O = ---
    beep(DASH_TIME);
    beep(DASH_TIME);
    beep(DASH_TIME);

    vTaskDelay(pdMS_TO_TICKS(LETTER_GAP));

    // S = ...
    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);
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
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&out_conf);

    gpio_set_level(BUZZER_PIN, 0);

    ESP_LOGI(TAG, "Morse Code SOS Generator Initialized.");

    bool last_button_state = true; // HIGH (pull-up)

    // 2. Main Event Loop
    while (1) {
        bool button_state = (gpio_get_level(BUTTON_PIN) == 1);

        // Detect button press (HIGH to LOW transition)
        if (last_button_state && !button_state) {
            ESP_LOGI(TAG, "SOS Triggered!");
            sendSOS();

            // Debounce delay
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        last_button_state = button_state;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}