/*
  8-bit Binary Counter

  Each press of the button increases an 8-bit counter by 1.

  The first 8 LEDs of the matrix represent the binary value:
  - LED ON  = 1
  - LED DIM = 0

  The counter goes from 0 to 255 and then wraps back to 0.

  The decimal and binary values are also printed to
  the Serial Monitor.
*/
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)
#define BUTTON_PIN         8
#define EXTRA_PIN          16

static const char *TAG = "binary_counter";

static led_strip_handle_t led_strip;
static uint8_t counter = 0;
static bool last_button_state = true; // HIGH with pull-up

// Display the counter as binary on the matrix
static void display_binary(void) {
    // Clear matrix first
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    // Use the first 8 LEDs for the 8 bits
    for (int bit = 0; bit < 8; bit++) {
        bool state = (counter & (1 << bit)) != 0;

        if (state) {
            // ON = binary 1 (Green, scaled by brightness)
            uint8_t g_val = (255 * MATRIX_BRIGHTNESS) / 255;
            led_strip_set_pixel(led_strip, bit, 0, g_val, 0);
        } else {
            // DIM = binary 0 (Dim white/gray, scaled by brightness)
            uint8_t dim_val = (10 * MATRIX_BRIGHTNESS) / 255;
            led_strip_set_pixel(led_strip, bit, dim_val, dim_val, dim_val);
        }
    }

    led_strip_refresh(led_strip);
}

// Print decimal and binary values
static void print_binary(void) {
    char bin_str[9];
    for (int bit = 7; bit >= 0; bit--) {
        bin_str[7 - bit] = ((counter >> bit) & 1) ? '1' : '0';
    }
    bin_str[8] = '\0';

    ESP_LOGI(TAG, "Decimal: %u Binary: %s", counter, bin_str);
}

void app_main(void) {
    // 1. Configure Pin 16 as output and set low
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << EXTRA_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(EXTRA_PIN, 0);

    // 2. Configure Button Pin (GPIO 17) with internal pull-up
    io_conf.pin_bit_mask = (1ULL << BUTTON_PIN);
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = GPIO_PULLUP_ENABLE;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
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

    // 4. Log startup and instructions
    ESP_LOGI(TAG, "8-bit Binary Counter Initialized");
    ESP_LOGI(TAG, "Press button on GPIO %d to increment.", BUTTON_PIN);

    display_binary();
    print_binary();

    // 5. Main Event Loop
    while (1) {
        int button_level = gpio_get_level(BUTTON_PIN);
        bool button_state = (button_level == 1);

        // Detect a new button press (HIGH to LOW transition)
        if (last_button_state && !button_state) {
            counter++;

            display_binary();
            print_binary();

            // Basic button debounce
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        last_button_state = button_state;

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}