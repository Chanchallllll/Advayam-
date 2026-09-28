/*
  Buzzer Tone Pitch Ladder

  This program uses a push button to cycle through 8 musical notes,
  from C4 to C5. Each time the button is pressed, the buzzer plays
  the next note in the sequence.

  The button uses INPUT_PULLUP, so a LOW signal indicates a press.
  A debounce system is included to prevent a single button press
  from being detected multiple times due to mechanical button bounce.

  After the final note (C5), the sequence starts again from C4.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_timer.h"

#define BUTTON_PIN     17
#define BUZZER_PIN     16

#define LEDC_TIMER     LEDC_TIMER_0
#define LEDC_MODE      LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL   LEDC_CHANNEL_0
#define LEDC_DUTY_RES  LEDC_TIMER_8_BIT

static const char *TAG = "tone_ladder";

// Musical note frequencies in Hz
const int notes[] = {
    262,  // C4
    294,  // D4
    330,  // E4
    349,  // F4
    392,  // G4
    440,  // A4
    494,  // B4
    523   // C5
};

// Names corresponding to the note frequencies
const char* note_names[] = {
    "C4", "D4", "E4", "F4",
    "G4", "A4", "B4", "C5"
};

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
        .freq_hz          = 1000,
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

    ESP_LOGI(TAG, "Buzzer Tone Pitch Ladder Initialized.");
    ESP_LOGI(TAG, "Press button to cycle notes.");

    int current_note = 0;
    bool last_button_state = true;       // HIGH (pull-up)
    bool current_button_state = true;    // HIGH (pull-up)
    uint64_t last_debounce_time = 0;
    const uint64_t debounce_delay = 50;  // 50 ms

    // 3. Main Event Loop
    while (1) {
        bool reading = (gpio_get_level(BUTTON_PIN) == 1);
        uint64_t current_time_ms = esp_timer_get_time() / 1000;

        // If the physical reading changed, restart the debounce timer
        if (reading != last_button_state) {
            last_debounce_time = current_time_ms;
        }

        // Only accept the new state if it has remained stable long enough
        if ((current_time_ms - last_debounce_time) > debounce_delay) {
            if (reading != current_button_state) {
                current_button_state = reading;

                // Because INPUT_PULLUP is used, LOW means the button is pressed
                if (current_button_state == false) {
                    ESP_LOGI(TAG, "Playing: %s (%d Hz)", note_names[current_note], notes[current_note]);

                    // Set frequency and 50% duty cycle (128 out of 255) to play tone
                    ledc_set_freq(LEDC_MODE, LEDC_TIMER, notes[current_note]);
                    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128);
                    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

                    // Play note for 300 ms
                    vTaskDelay(pdMS_TO_TICKS(300));

                    // Stop tone (0% duty cycle)
                    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
                    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

                    // Move to the next note
                    current_note++;
                    if (current_note >= 8) {
                        current_note = 0;
                    }
                }
            }
        }

        last_button_state = reading;
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}