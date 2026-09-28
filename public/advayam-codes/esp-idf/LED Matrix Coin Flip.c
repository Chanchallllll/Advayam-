/*
  Coin Flip Simulator

  Press the button to flip the coin.
  The matrix shows a spinning coin animation,
  then stops on HEADS or TAILS and plays a chime.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "led_strip.h"
#include "esp_random.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define BUTTON_PIN         17
#define BUZZER_PIN         16

#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL       LEDC_CHANNEL_0
#define LEDC_DUTY_RES      LEDC_TIMER_8_BIT

static const char *TAG = "coin_flip";

static led_strip_handle_t led_strip;

// Convert x/y position to LED number
static int pixel_xy(int x, int y) {
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

// Set one matrix pixel
static void set_pixel_xy(int x, int y, uint32_t color) {
    if (x < 0 || x >= 5 || y < 0 || y >= 5)
        return;

    int idx = pixel_xy(x, y);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;

    led_strip_set_pixel(led_strip, idx, r, g, b);
}

// Draw the coin
// mode: 0 = heads, 1 = tails, 2 = thin coin
static void draw_coin(int mode) {
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    uint32_t color = 0xFFB400; // RGB equivalent of 255, 180, 0

    if (mode == 0 || mode == 1) {
        // Coin outline
        for (int x = 1; x <= 3; x++) {
            set_pixel_xy(x, 0, color);
            set_pixel_xy(x, 4, color);
        }

        for (int y = 1; y <= 3; y++) {
            set_pixel_xy(0, y, color);
            set_pixel_xy(4, y, color);
        }

        // HEADS
        if (mode == 0) {
            set_pixel_xy(2, 1, color);
            set_pixel_xy(1, 2, color);
            set_pixel_xy(2, 2, color);
            set_pixel_xy(3, 2, color);
            set_pixel_xy(2, 3, color);
        }
        // TAILS
        else {
            set_pixel_xy(1, 1, color);
            set_pixel_xy(3, 1, color);
            set_pixel_xy(2, 2, color);
            set_pixel_xy(1, 3, color);
            set_pixel_xy(3, 3, color);
        }
    }
    // Coin viewed edge-on
    else if (mode == 2) {
        for (int y = 0; y < 5; y++) {
            set_pixel_xy(2, y, color);
        }
    }

    led_strip_refresh(led_strip);
}

// Coin flipping animation
static void flip_animation(bool result) {
    int delay_time = 60;
    int total_flips = 12;

    for (int i = 0; i < total_flips; i++) {
        draw_coin(0);
        vTaskDelay(pdMS_TO_TICKS(delay_time));

        draw_coin(2);
        vTaskDelay(pdMS_TO_TICKS(delay_time / 2));

        draw_coin(1);
        vTaskDelay(pdMS_TO_TICKS(delay_time));

        draw_coin(2);
        vTaskDelay(pdMS_TO_TICKS(delay_time / 2));

        if (i > 5)
            delay_time += 25;
        else
            delay_time += 8;
    }

    draw_coin(result ? 0 : 1);
}

// Play buzzer tone using LEDC PWM
static void play_tone(uint32_t freq, uint32_t duration_ms) {
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, freq);
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128); // 50% duty cycle
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

// Play a short chime
static void play_chime(bool heads) {
    if (heads) {
        play_tone(880, 150);
        vTaskDelay(pdMS_TO_TICKS(170));
        play_tone(1175, 180);
    } else {
        play_tone(660, 150);
        vTaskDelay(pdMS_TO_TICKS(170));
        play_tone(440, 180);
    }
}

void app_main(void) {
    // 1. Configure LEDC PWM for Buzzer Tone Generation
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

    // 2. Configure Button Pin (GPIO 17) with internal pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

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

    // 4. Log startup instructions
    ESP_LOGI(TAG, "Coin Flip Simulator initialized.");
    ESP_LOGI(TAG, "Press button on GPIO %d to flip.", BUTTON_PIN);

    bool last_button = true; // HIGH with pull-up

    // 5. Main Loop
    while (1) {
        int button_level = gpio_get_level(BUTTON_PIN);
        bool button_state = (button_level == 1);

        // Detect button press (HIGH to LOW transition)
        if (last_button && !button_state) {
            bool heads = (esp_random() % 2 == 0);

            ESP_LOGI(TAG, "Flipping...");

            flip_animation(heads);

            if (heads)
                ESP_LOGI(TAG, "HEADS");
            else
                ESP_LOGI(TAG, "TAILS");

            play_chime(heads);

            // Small debounce delay
            vTaskDelay(pdMS_TO_TICKS(300));
        }

        last_button = button_state;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}