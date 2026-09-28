/*
 * This program runs Conway's Game of Life on the 5x5 RGB
 * LED matrix connected to GPIO 7.
 *
 * The initial cells are randomized and the simulation
 * continuously evolves on the matrix.
 */
 #include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_random.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define MATRIX_PIN       7
#define NUM_LEDS         25
#define MATRIX_BRIGHTNESS 20 // Scale (0 - 255)

static const char *TAG = "game_of_life";

static led_strip_handle_t led_strip;
static bool currentGrid[5][5];
static bool nextGrid[5][5];

// Convert x/y coordinates to serpentine LED index
int pixelXY(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Draw the current Game of Life state
void drawGrid(void) {
    // Clear strip buffer
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (currentGrid[y][x]) {
                int index = pixelXY(x, y);
                if (index >= 0 && index < NUM_LEDS) {
                    // Set Green with brightness scaling
                    uint8_t g_val = (255 * MATRIX_BRIGHTNESS) / 255;
                    led_strip_set_pixel(led_strip, index, 0, g_val, 0);
                }
            }
        }
    }

    led_strip_refresh(led_strip);
}

// Count living neighbors
int countNeighbors(int x, int y) {
    int count = 0;

    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) {
                continue;
            }

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < 5 && ny >= 0 && ny < 5) {
                if (currentGrid[ny][nx]) {
                    count++;
                }
            }
        }
    }

    return count;
}

// Calculate the next generation
void updateGrid(void) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            int neighbors = countNeighbors(x, y);

            if (currentGrid[y][x]) {
                nextGrid[y][x] = (neighbors == 2 || neighbors == 3);
            } else {
                nextGrid[y][x] = (neighbors == 3);
            }
        }
    }

    // Copy next generation
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            currentGrid[y][x] = nextGrid[y][x];
        }
    }
}

// Create a random starting pattern
void randomizeGrid(void) {
    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            currentGrid[y][x] = ((rand() % 100) < 45);
        }
    }
}

void app_main(void) {
    // Seed random number generator using hardware entropy
    srand(esp_random());

    // 1. Configure LED Strip via RMT
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

    ESP_LOGI(TAG, "Conway's Game of Life Initialized on 5x5 Matrix.");

    randomizeGrid();

    // 2. Main Simulation Loop
    while (1) {
        drawGrid();
        updateGrid();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}