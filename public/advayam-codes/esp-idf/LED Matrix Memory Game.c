/*
 * =============================================================================
 * SIMON SAYS MEMORY GAME
 * =============================================================================
 * The matrix flashes a growing sequence of four colours.
 *
 * PLAYER INPUT:
 *   Touch GPIO 8 -> Cycle through colours
 *   Button GPIO 17 -> Select/confirm current colour
 *
 * COLOUR ORDER:
 *   RED -> GREEN -> BLUE -> YELLOW -> RED...
 *
 * Every touch gives a short beep.
 *
 * The sequence gets longer after every successful round.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Touch  = GPIO 8
 * Buzzer = GPIO 16
 * =============================================================================
 */
 #include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "esp_random.h"
#include "esp_rom_sys.h"

#define MATRIX_PIN         7
#define BUTTON_PIN         17
#define TOUCH_PIN          8
#define BUZZER_PIN         16

#define MAX_SEQUENCE       20
#define BRIGHTNESS_SCALE   7  // Scaled down (approx equivalent to Adafruit setBrightness(7))

static const char *TAG = "simon_says";
static led_strip_handle_t led_strip;

static uint8_t sequence[MAX_SEQUENCE];
static int sequenceLength = 1;
static bool lastTouchState = true; // HIGH (due to pull-up)

// Forward declarations
static void showSelection(uint8_t color);

// Helper to scale RGB brightness uniformly
static uint32_t scaleColor(uint32_t color_rgb) {
    uint8_t r = (color_rgb >> 16) & 0xFF;
    uint8_t g = (color_rgb >> 8) & 0xFF;
    uint8_t b = color_rgb & 0xFF;

    r = (r * BRIGHTNESS_SCALE) / 255;
    g = (g * BRIGHTNESS_SCALE) / 255;
    b = (b * BRIGHTNESS_SCALE) / 255;

    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static void allColor(uint32_t color_rgb) {
    uint32_t scaled = scaleColor(color_rgb);
    uint8_t r = (scaled >> 16) & 0xFF;
    uint8_t g = (scaled >> 8) & 0xFF;
    uint8_t b = scaled & 0xFF;

    for (int i = 0; i < 25; i++) {
        led_strip_set_pixel(led_strip, i, r, g, b);
    }
    led_strip_refresh(led_strip);
}

static void play_tone(uint32_t frequency, uint32_t duration_ms) {
    if (frequency == 0) {
        vTaskDelay(pdMS_TO_TICKS(duration_ms));
        return;
    }
    uint32_t cycles = (frequency * duration_ms) / 1000;
    uint32_t delay_us = 500000 / frequency;

    for (uint32_t i = 0; i < cycles; i++) {
        gpio_set_level(BUZZER_PIN, 1);
        esp_rom_delay_us(delay_us);
        gpio_set_level(BUZZER_PIN, 0);
        esp_rom_delay_us(delay_us);
    }
}

static void showColor(uint8_t color) {
    uint32_t c = 0;

    if (color == 0) c = 0xFF0000;       // RED
    else if (color == 1) c = 0x00FF00;  // GREEN
    else if (color == 2) c = 0x0000FF;  // BLUE
    else if (color == 3) c = 0xFFB400;  // YELLOW

    allColor(c);
    play_tone(400 + color * 250, 200);
    vTaskDelay(pdMS_TO_TICKS(300));

    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);
    vTaskDelay(pdMS_TO_TICKS(150));
}

static bool touchPressed(void) {
    int touchState = gpio_get_level(TOUCH_PIN);
    bool pressed = (lastTouchState == 1 && touchState == 0);
    lastTouchState = touchState;
    return pressed;
}

static uint8_t waitForSelection(void) {
    uint8_t selected = 0;

    // Start with RED
    showSelection(selected);

    while (1) {
        // Touch -> cycle to next colour
        if (touchPressed()) {
            selected++;
            if (selected > 3) {
                selected = 0;
            }

            // Short beep for every touch
            gpio_set_level(BUZZER_PIN, 1);
            vTaskDelay(pdMS_TO_TICKS(100));
            gpio_set_level(BUZZER_PIN, 0);

            // Simple debounce
            vTaskDelay(pdMS_TO_TICKS(200));

            // Show newly selected colour
            showSelection(selected);
        }

        // Button -> confirm colour (Active LOW)
        if (gpio_get_level(BUTTON_PIN) == 0) {
            vTaskDelay(pdMS_TO_TICKS(150));
            while (gpio_get_level(BUTTON_PIN) == 0) {
                vTaskDelay(pdMS_TO_TICKS(10));
            }
            return selected;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void showSelection(uint8_t color) {
    uint32_t c = 0;
    if (color == 0) c = 0xFF0000;       // RED
    else if (color == 1) c = 0x00FF00;  // GREEN
    else if (color == 2) c = 0x0000FF;  // BLUE
    else if (color == 3) c = 0xFFB400;  // YELLOW

    allColor(c);
}

static void failure(void) {
    for (int i = 0; i < 3; i++) {
        allColor(0xFF0000);
        play_tone(150, 150);
        vTaskDelay(pdMS_TO_TICKS(200));

        led_strip_clear(led_strip);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

static void success(void) {
    allColor(0x00FF00);
    play_tone(1200, 200);
    vTaskDelay(pdMS_TO_TICKS(300));

    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);
}

void app_main(void) {
    // Configure GPIOs
    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN) | (1ULL << TOUCH_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&input_conf);

    gpio_config_t buz_conf = {
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&buz_conf);
    gpio_set_level(BUZZER_PIN, 0);

    // Initialize LED Strip via RMT
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = 25,
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

    // Generate random sequence
    for (int i = 0; i < MAX_SEQUENCE; i++) {
        sequence[i] = (uint8_t)(esp_random() % 4);
    }

    ESP_LOGI(TAG, "Simon Says started.");

    while (1) {
        ESP_LOGI(TAG, "Round: %d", sequenceLength);

        // Show the sequence
        for (int i = 0; i < sequenceLength; i++) {
            showColor(sequence[i]);
            vTaskDelay(pdMS_TO_TICKS(150));
        }

        // Player input loop
        bool round_failed = false;
        for (int i = 0; i < sequenceLength; i++) {
            uint8_t answer = waitForSelection();
            ESP_LOGI(TAG, "Selected: %d", answer);

            if (answer != sequence[i]) {
                ESP_LOGI(TAG, "Wrong!");
                failure();
                sequenceLength = 1;
                vTaskDelay(pdMS_TO_TICKS(1000));
                round_failed = true;
                break;
            }
        }

        if (round_failed) {
            continue;
        }

        // Round completed successfully
        ESP_LOGI(TAG, "Correct!");
        success();

        sequenceLength++;
        if (sequenceLength > MAX_SEQUENCE) {
            sequenceLength = MAX_SEQUENCE;
        }

        vTaskDelay(pdMS_TO_TICKS(800));
    }
}