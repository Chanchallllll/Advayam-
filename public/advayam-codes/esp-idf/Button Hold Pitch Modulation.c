#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define BUTTON_PIN      17
#define BUZZER_PIN      16

#define LEDC_TIMER      LEDC_TIMER_0
#define LEDC_MODE       LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL    LEDC_CHANNEL_0
#define LEDC_DUTY_RES   LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY  1000

static const char *TAG = "button_buzzer";

void app_main(void) {
    // 1. Configure Button GPIO with Internal Pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 2. Configure LEDC PWM for Buzzer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0,
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "Button Hold Buzzer Controller Initialized.");

    uint64_t press_start = 0;
    bool button_pressed = false;

    // 3. Main Task Loop
    while (1) {
        bool pressed = (gpio_get_level(BUTTON_PIN) == 0); // Active LOW
        uint64_t current_time_ms = esp_timer_get_time() / 1000;

        // Detect button press
        if (pressed && !button_pressed) {
            button_pressed = true;
            press_start = current_time_ms;
        }

        // Increase pitch while held
        if (pressed) {
            unsigned long hold_time = (unsigned long)(current_time_ms - press_start);

            int frequency = 300 + (hold_time / 10);
            if (frequency > 4000) {
                frequency = 4000;
            }

            // Update PWM frequency and 50% duty cycle (128 out of 255)
            ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

            ESP_LOGI(TAG, "Hold Time: %lu ms | Frequency: %d Hz", hold_time, frequency);
        }

        // Detect button release
        if (!pressed && button_pressed) {
            button_pressed = false;

            // Stop buzzer (0% duty cycle)
            ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
            ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}