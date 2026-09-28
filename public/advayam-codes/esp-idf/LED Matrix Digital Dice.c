/*
  Digital Dice Roller

  Pressing the button starts an animated dice-rolling effect.
  After the animation, a random number from 1 to 6 is selected
  and displayed on the 5x5 NeoPixel matrix using dice dots.

  The rolling dots are shown in orange, while the final result
  is displayed in green.

  The result is also printed to the Serial Monitor.
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_random.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define BUTTON_PIN         17
#define POWER_PIN          16
#define MATRIX_BRIGHTNESS  50 // Scale (0 - 255)

static const char *TAG = "digital_dice";
static led_strip_handle_t led_strip;

// Row-major coordinate conversion
static int pixel_xy(int x, int y) {
    return y * 5 + x;
}

// Set one LED using x/y coordinates with color and brightness scaling
static void set_pixel_xy(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    if (x < 0 || x > 4 || y < 0 || y > 4) {
        return;
    }

    // Apply global brightness scaling
    uint8_t r_val = (r * MATRIX_BRIGHTNESS) / 255;
    uint8_t g_val = (g * MATRIX_BRIGHTNESS) / 255;
    uint8_t b_val = (b * MATRIX_BRIGHTNESS) / 255;

    int idx = pixel_xy(x, y);
    led_strip_set_pixel(led_strip, idx, r_val, g_val, b_val);
}

// Draw a dice number from 1 to 6
static void draw_die(int number, uint8_t r, uint8_t g, uint8_t b) {
    // Clear matrix first
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    // Center dot
    if (number == 1 || number == 3 || number == 5) {
        set_pixel_xy(2, 2, r, g, b);
    }

    // Top-left and bottom-right dots
    if (number == 2 || number == 3 || number == 4 || number == 5 || number == 6) {
        set_pixel_xy(1, 1, r, g, b);
        set_pixel_xy(3, 3, r, g, b);
    }

    // Top-right and bottom-left dots
    if (number == 4 || number == 5 || number == 6) {
        set_pixel_xy(3, 1, r, g, b);
        set_pixel_xy(1, 3, r, g, b);
    }

    // Middle-left and middle-right dots
    if (number == 6) {
        set_pixel_xy(1, 2, r, g, b);
        set_pixel_xy(3, 2, r, g, b);
    }

    led_strip_refresh(led_strip);
}

// Animate the dice and display the final result
static void roll_dice(void) {
    ESP_LOGI(TAG, "Rolling...");

    // Show random dice values during the animation
    for (int i = 0; i < 15; i++) {
        int random_number = (esp_random() % 6) + 1;

        // Display the rolling number in orange (RGB: 255, 80, 0)
        draw_die(random_number, 255, 80, 0);

        // Gradually slow down the animation
        vTaskDelay(pdMS_TO_TICKS(80 + i * 10));
    }

    // Select the final dice result
    int result = (esp_random() % 6) + 1;

    // Display the final result in green (RGB: 0, 255, 0)
    draw_die(result, 0, 255, 0);

    ESP_LOGI(TAG, "Dice result: %d", result);
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

    ESP_LOGI(TAG, "Digital Dice Roller Initialized.");
    ESP_LOGI(TAG, "Press button to roll.");

    bool last_button = true; // HIGH with pull-up

    // 4. Main Event Loop
    while (1) {
        int button_level = gpio_get_level(BUTTON_PIN);
        bool button = (button_level == 1);

        // Detect a new button press (HIGH to LOW transition)
        if (last_button && !button) {
            roll_dice();
            vTaskDelay(pdMS_TO_TICKS(200)); // Basic debounce
        }

        last_button = button;
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}