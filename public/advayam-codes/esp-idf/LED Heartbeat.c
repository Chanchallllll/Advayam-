/*
 * This program simulates a human heartbeat using the User LED
 * on GPIO 45.
 *
 * The LED performs two fading pulses followed by a short pause,
 * creating a repeating heartbeat-like pattern.
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/ledc.h"

#define LED_PIN          45

#define LEDC_TIMER       LEDC_TIMER_0
#define LEDC_MODE        LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL     LEDC_CHANNEL_0
#define LEDC_DUTY_RES    LEDC_TIMER_8_BIT
#define PWM_FREQ         5000

static const char *TAG = "heartbeat_led";

// Create a smooth LED pulse matching the Arduino timing
static void pulse_led(int max_brightness, int duration) {
    // Fade in
    for (int brightness = 0; brightness <= max_brightness; brightness += 5) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(duration / 50));
    }

    // Fade out
    for (int brightness = max_brightness; brightness >= 0; brightness -= 5) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, brightness);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(pdMS_TO_TICKS(duration / 50));
    }
}

void app_main(void) {
    // 1. Configure LEDC Timer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = PWM_FREQ,
        .clk_cfg          = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2. Configure LEDC Channel
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = LED_PIN,
        .duty           = 0,
        .hpoint         = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));

    ESP_LOGI(TAG, "Heartbeat LED Simulation Initialized.");

    // 3. Main Heartbeat Loop
    while (1) {
        // First heartbeat pulse
        pulse_led(255, 250);

        vTaskDelay(pdMS_TO_TICKS(80));

        // Second heartbeat pulse
        pulse_led(180, 180);

        // Pause between heartbeats
        vTaskDelay(pdMS_TO_TICKS(600));
    }
}