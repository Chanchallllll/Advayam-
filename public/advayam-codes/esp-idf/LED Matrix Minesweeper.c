/*
 * =============================================================================
 * MINESWEEPER MINI
 * =============================================================================
 *
 * Simplified 5x5 Minesweeper game.
 *
 * CONTROLS:
 *   Tilt          -> move cursor
 *   Short press   -> uncover
 *   Long press    -> flag / unflag
 *
 * IMU CONFIGURATION:
 *   Physical Y axis -> horizontal movement
 *   Physical X axis -> vertical movement
 *
 *   +Y -> RIGHT
 *   -Y -> LEFT
 *
 *   +X -> DOWN
 *   -X -> UP
 *
 *   Tilt threshold = approximately 15 degrees.
 *
 *   Holding a tilt continuously moves the cursor.
 *
 * DISPLAY:
 *   Entire matrix is rotated 180 degrees.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Buzzer = GPIO 16
 * IMU    = LSM6DSM SPI
 * =============================================================================
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "esp_random.h"
#include "esp_rom_sys.h"

#define MATRIX_PIN         7
#define BUTTON_PIN         17
#define BUZZER_PIN         16

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define NUM_PIXELS         25
#define TILT_THRESHOLD     4240
#define CURSOR_MOVE_DELAY  180
#define LONG_PRESS_TIME    700
#define BRIGHTNESS_SCALE   80  // Scaled down (equivalent to matrix.setBrightness(80))

static const char *TAG = "minesweeper";
static led_strip_handle_t led_strip;
static spi_device_handle_t spi_imu;

static bool mine[5][5];
static bool revealed[5][5];
static bool flagged[5][5];

static int cursorX = 0;
static int cursorY = 0;
static unsigned long lastCursorMove = 0;

// Forward declarations
static void draw(void);
static void setupGame(void);

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

static void setPixel(int x, int y, uint32_t color_rgb) {
    if (x < 0 || x > 4 || y < 0 || y > 4)
        return;

    int displayX = 4 - x;
    int displayY = 4 - y;
    int index = (displayY % 2 == 0) ? (displayY * 5 + displayX) : (displayY * 5 + (4 - displayX));

    uint32_t scaled = scaleColor(color_rgb);
    led_strip_set_pixel(led_strip, index, (scaled >> 16) & 0xFF, (scaled >> 8) & 0xFF, scaled & 0xFF);
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

//=============================================================================
// Count Nearby Mines
//=============================================================================

static int countMines(int x, int y) {
    int count = 0;
    for (int dx = -1; dx <= 1; dx++) {
        for (int dy = -1; dy <= 1; dy++) {
            int nx = x + dx;
            int ny = y + dy;
            if (nx >= 0 && nx < 5 && ny >= 0 && ny < 5 && mine[nx][ny]) {
                count++;
            }
        }
    }
    return count;
}

//=============================================================================
// Draw
//=============================================================================

static void draw(void) {
    led_strip_clear(led_strip);

    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            uint32_t color = 0x000000;

            if (flagged[x][y]) {
                color = 0xFF9600; // Orange/Yellow flag
            } else if (revealed[x][y]) {
                color = 0x009600; // Green safe cell
            }

            if (x == cursorX && y == cursorY) {
                color = 0xFFFFFF; // White cursor
            }

            setPixel(x, y, color);
        }
    }
    led_strip_refresh(led_strip);
}

//=============================================================================
// Reveal Everything
//=============================================================================

static void revealAll(void) {
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            revealed[x][y] = true;
        }
    }
    draw();
}

//=============================================================================
// Setup New Game
//=============================================================================

static void setupGame(void) {
    memset(mine, false, sizeof(mine));
    memset(revealed, false, sizeof(revealed));
    memset(flagged, false, sizeof(flagged));

    int placed = 0;
    while (placed < 5) {
        int x = esp_random() % 5;
        int y = esp_random() % 5;

        // Keep starting cell safe
        if (x == 0 && y == 0) continue;

        if (!mine[x][y]) {
            mine[x][y] = true;
            placed++;
        }
    }

    cursorX = 0;
    cursorY = 0;
    lastCursorMove = (unsigned long)(esp_timer_get_time() / 1000);

    draw();
}

//=============================================================================
// IMU Functions
//=============================================================================

static void imuWrite(uint8_t reg, uint8_t value) {
    uint8_t tx_data[2] = { reg & 0x7F, value };
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    spi_device_transmit(spi_imu, &t);
}

static int16_t imuRead16(uint8_t reg) {
    uint8_t tx_data[3] = { (uint8_t)(reg | 0x80), 0, 0 };
    uint8_t rx_data[3] = { 0 };
    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    spi_device_transmit(spi_imu, &t);
    return (int16_t)((rx_data[2] << 8) | rx_data[1]);
}

static void setupIMU(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = IMU_MISO,
        .mosi_io_num = IMU_MOSI,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = IMU_CS,
        .queue_size = 1,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_imu));

    // LSM6DSM: 104 Hz, +-2g, Normal operating mode
    imuWrite(0x10, 0x40);
}

//=============================================================================
// Cursor Movement
//=============================================================================

static void moveCursor(void) {
    int16_t ax = imuRead16(0x28);
    int16_t ay = imuRead16(0x2A);

    if (abs(ax) < TILT_THRESHOLD && abs(ay) < TILT_THRESHOLD) {
        return;
    }

    unsigned long current_ms = (unsigned long)(esp_timer_get_time() / 1000);
    if (current_ms - lastCursorMove < CURSOR_MOVE_DELAY) {
        return;
    }
    lastCursorMove = current_ms;

    if (abs(ay) > abs(ax)) {
        if (ay > TILT_THRESHOLD) {
            if (cursorX < 4) {
                cursorX++;
                play_tone(700, 20);
            }
        } else if (ay < -TILT_THRESHOLD) {
            if (cursorX > 0) {
                cursorX--;
                play_tone(700, 20);
            }
        }
    } else {
        // X AXIS = VERTICAL (Inverted x+ and x-)
        if (ax > TILT_THRESHOLD) {
            if (cursorY > 0) {
                cursorY--;
                play_tone(700, 20);
            }
        } else if (ax < -TILT_THRESHOLD) {
            if (cursorY < 4) {
                cursorY++;
                play_tone(700, 20);
            }
        }
    }
}

//=============================================================================
// Uncover
//=============================================================================

static void uncover(void) {
    if (flagged[cursorX][cursorY]) return;
    if (revealed[cursorX][cursorY]) return;

    if (mine[cursorX][cursorY]) {
        ESP_LOGI(TAG, "BOOM!");
        revealAll();
        play_tone(120, 500);
        vTaskDelay(pdMS_TO_TICKS(1000));
        setupGame();
        return;
    }

    revealed[cursorX][cursorY] = true;
    int nearby = countMines(cursorX, cursorY);
    play_tone(500 + nearby * 150, 80);
    ESP_LOGI(TAG, "Nearby mines: %d", nearby);
}

//=============================================================================
// Main App Entry
//=============================================================================

void app_main(void) {
    ESP_LOGI(TAG, "Minesweeper starting...");

    gpio_config_t input_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
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
        .max_leds = NUM_PIXELS,
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

    setupIMU();
    setupGame();

    static bool previousButton = true;
    static unsigned long buttonStart = 0;

    while (1) {
        moveCursor();

        bool button = gpio_get_level(BUTTON_PIN);
        unsigned long current_ms = (unsigned long)(esp_timer_get_time() / 1000);

        if (button == 0 && previousButton == 1) {
            buttonStart = current_ms;
        }

        if (button == 1 && previousButton == 0) {
            unsigned long duration = current_ms - buttonStart;

            if (duration >= LONG_PRESS_TIME) {
                if (!revealed[cursorX][cursorY]) {
                    flagged[cursorX][cursorY] = !flagged[cursorX][cursorY];
                    play_tone(700, 60);
                }
            } else {
                uncover();
            }
        }
        previousButton = button;

        draw();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}