/*
 * =============================================================================
 * PIXEL MAZE ESCAPE
 * =============================================================================
 * Generates a randomized 5x5 maze.
 *
 * START:
 *   (0,0)
 *
 * EXIT:
 *   (4,4)
 *
 * TILT CONTROLS:
 *
 *   +Y -> LEFT
 *   -Y -> RIGHT
 *
 *   +X -> DOWN
 *   -X -> UP
 *
 * Tilt below approximately 15 degrees is ignored.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Walls are displayed as red pixels.
 * Player is green.
 * Exit is blue.
 *
 * Matrix = GPIO 7
 * Buzzer = GPIO 16
 * IMU    = LSM6DSM SPI
 * =============================================================================
 */
 #include <stdio.h>
#include <string.h>
#include <math.h>
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
#define BUZZER_PIN         16

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define NUM_PIXELS         25
#define TILT_THRESHOLD     4240

static const char *TAG = "pixel_maze";
static led_strip_handle_t led_strip;
static spi_device_handle_t spi_imu;

// Maze state
static bool wall[5][5];
static int playerX = 0;
static int playerY = 0;

// Tilt timing
static uint32_t lastMove = 0;
const uint32_t MOVE_DELAY = 180;

// LED Display coordinates translation (180 degree rotation)
static int xy(int x, int y) {
    int displayX = 4 - x;
    int displayY = 4 - y;

    if (displayY % 2 == 0)
        return displayY * 5 + displayX;

    return displayY * 5 + (4 - displayX);
}

static void pixel(int x, int y, uint32_t color) {
    if (x < 0 || x > 4 || y < 0 || y > 4)
        return;

    uint32_t led_idx = xy(x, y);
    led_strip_set_pixel(led_strip, led_idx, (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

// Low-level SPI read/write for LSM6DSM
static void imuWrite(uint8_t r, uint8_t v) {
    uint8_t tx_data[2] = {r & 0x7F, v};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    gpio_set_level(IMU_CS, 0);
    spi_device_transmit(spi_imu, &t);
    gpio_set_level(IMU_CS, 1);
}

static int16_t imu16(uint8_t r) {
    uint8_t tx_data[3] = {r | 0x80, 0, 0};
    uint8_t rx_data[3];
    memset(rx_data, 0, sizeof(rx_data));

    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    gpio_set_level(IMU_CS, 0);
    spi_device_transmit(spi_imu, &t);
    gpio_set_level(IMU_CS, 1);

    int16_t val = (int16_t)((rx_data[2] << 8) | rx_data[1]);
    return val;
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
        .spics_io_num = -1,
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_imu));

    gpio_config_t cs_conf = {
        .pin_bit_mask = (1ULL << IMU_CS),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cs_conf);
    gpio_set_level(IMU_CS, 1);

    vTaskDelay(pdMS_TO_TICKS(50));

    // LSM6DSM: 104 Hz, +/- 2g
    imuWrite(0x10, 0x40);
}

// Maze Generation & Solvability Verification via BFS
static void generateMaze(void) {
    while (true) {
        for (int x = 0; x < 5; x++) {
            for (int y = 0; y < 5; y++) {
                if ((x == 0 && y == 0) || (x == 4 && y == 4)) {
                    wall[x][y] = false;
                } else {
                    wall[x][y] = ((esp_random() % 100) < 30);
                }
            }
        }

        bool visited[5][5];
        memset(visited, false, sizeof(visited));

        int queueX[25];
        int queueY[25];
        int head = 0;
        int tail = 0;

        queueX[tail] = 0;
        queueY[tail] = 0;
        tail++;
        visited[0][0] = true;

        while (head < tail) {
            int x = queueX[head];
            int y = queueY[head];
            head++;

            if (x == 4 && y == 4)
                break;

            const int dx[4] = { 1, -1, 0, 0 };
            const int dy[4] = { 0, 0, 1, -1 };

            for (int d = 0; d < 4; d++) {
                int nx = x + dx[d];
                int ny = y + dy[d];

                if (nx < 0 || nx >= 5 || ny < 0 || ny >= 5)
                    continue;
                if (wall[nx][ny])
                    continue;
                if (visited[nx][ny])
                    continue;

                visited[nx][ny] = true;
                queueX[tail] = nx;
                queueY[tail] = ny;
                tail++;
            }
        }

        if (visited[4][4])
            break;
    }
}

static void draw(void) {
    led_strip_clear(led_strip);

    // Walls
    for (int x = 0; x < 5; x++) {
        for (int y = 0; y < 5; y++) {
            if (wall[x][y]) {
                pixel(x, y, 0xFF0000); // Red
            }
        }
    }

    // Exit
    pixel(4, 4, 0x0000FF); // Blue

    // Player
    pixel(playerX, playerY, 0x00FF00); // Green

    led_strip_refresh(led_strip);
}

// Simple tone generator for ESP32 via ledc or esp_rom
static void play_tone(uint32_t frequency, uint32_t duration_ms) {
    // Basic software buzzer toggle or standard ESP-ROM tone substitute
    // Using standard GPIO square wave approximation for simplicity matching Arduino tone()
    gpio_config_t buz_conf = {
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&buz_conf);

    uint32_t cycles = (frequency * duration_ms) / 1000;
    uint32_t delay_us = 500000 / frequency;

    for (uint32_t i = 0; i < cycles; i++) {
        gpio_set_level(BUZZER_PIN, 1);
        esp_rom_delay_us(delay_us);
        gpio_set_level(BUZZER_PIN, 0);
        esp_rom_delay_us(delay_us);
    }
}

static void movePlayer(void) {
    int16_t ax = imu16(0x28);
    int16_t ay = imu16(0x2A);

    if (abs(ax) < TILT_THRESHOLD && abs(ay) < TILT_THRESHOLD)
        return;

    uint32_t now = (uint32_t)(esp_timer_get_time() / 1000ULL);
    if (now - lastMove < MOVE_DELAY)
        return;

    int newX = playerX;
    int newY = playerY;

    if (abs(ay) > abs(ax)) {
        if (ay > TILT_THRESHOLD)
            newX--;
        else if (ay < -TILT_THRESHOLD)
            newX++;
    } else {
        if (ax > TILT_THRESHOLD)
            newY++;
        else if (ax < -TILT_THRESHOLD)
            newY--;
    }

    if (newX >= 0 && newX < 5 && newY >= 0 && newY < 5 && !wall[newX][newY]) {
        playerX = newX;
        playerY = newY;
        lastMove = now;
        play_tone(700, 30);
    }
}

static void mazeEscaped(void) {
    ESP_LOGI(TAG, "MAZE ESCAPED!");

    for (int i = 0; i < 3; i++) {
        led_strip_clear(led_strip);
        for (int j = 0; j < NUM_PIXELS; j++) {
            led_strip_set_pixel(led_strip, j, 0, 255, 0);
        }
        led_strip_refresh(led_strip);
        play_tone(1000 + i * 300, 150);
        vTaskDelay(pdMS_TO_TICKS(200));

        led_strip_clear(led_strip);
        led_strip_refresh(led_strip);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    vTaskDelay(pdMS_TO_TICKS(500));

    playerX = 0;
    playerY = 0;
    generateMaze();
    lastMove = (uint32_t)(esp_timer_get_time() / 1000ULL);
}

void app_main(void) {
    gpio_config_t buz_conf = {
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&buz_conf);

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
    generateMaze();
    draw();

    ESP_LOGI(TAG, "Pixel Maze Escape started.");

    while (1) {
        movePlayer();

        if (playerX == 4 && playerY == 4) {
            mazeEscaped();
        }

        draw();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}