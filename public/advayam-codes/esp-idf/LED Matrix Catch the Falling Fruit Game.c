/*
 * =============================================================================
 * CATCH THE FALLING FRUIT
 * =============================================================================
 * Green fruit falls from the top of the matrix.
 *
 * Tilt left/right controls the basket along the bottom row.
 *
 * IMU CONFIGURATION:
 *
 *   Physical Y axis controls horizontal movement.
 *
 *   +Y -> LEFT
 *   -Y -> RIGHT
 *
 *   Tilt below approximately 15 degrees is ignored.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Matrix = GPIO 7
 * Buzzer = GPIO 16
 * IMU    = LSM6DSM SPI
 * =============================================================================
 */
 #include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "led_strip.h"
#include "esp_timer.h"
#include "esp_random.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  10 // Scale (0 - 255)

#define BUZZER_PIN         16

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

// Approximately 15 degrees with the ±2g accelerometer setting.
#define TILT_THRESHOLD     4240

#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL       LEDC_CHANNEL_0
#define LEDC_DUTY_RES      LEDC_TIMER_8_BIT

static const char *TAG = "catch_fruit";

static led_strip_handle_t led_strip;
static spi_device_handle_t imu_handle;

// Game Variables
static int basket_x = 2;
static int fruit_x;
static int fruit_y;
static int score = 0;
static int lives = 3;

static uint64_t last_fall = 0;
static uint64_t last_tilt_move = 0;
const uint64_t TILT_MOVE_DELAY = 180;

// Helper: Write register to LSM6DSM over SPI
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

// Helper: Read 16-bit register from LSM6DSM over SPI
static int16_t imu_read_16(spi_device_handle_t handle, uint8_t reg) {
    uint8_t tx_data[3] = {reg | 0x80, 0, 0};
    uint8_t rx_data[3] = {0};
    spi_transaction_t t = {
        .length = 24,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    if (spi_device_transmit(handle, &t) == ESP_OK) {
        uint8_t l = rx_data[1];
        uint8_t h = rx_data[2];
        return (int16_t)((h << 8) | l);
    }
    return 0;
}

// Matrix coordinate mapping with 180 degree rotation and serpentine layout
static int xy(int x, int y) {
    int display_x = 4 - x;
    int display_y = 4 - y;

    if (display_y % 2 == 0)
        return display_y * 5 + display_x;

    return display_y * 5 + (4 - display_x);
}

static void set_pixel(int x, int y, uint32_t color) {
    if (x < 0 || x > 4 || y < 0 || y > 4)
        return;

    int idx = xy(x, y);
    uint8_t r = (color >> 16) & 0xFF;
    uint8_t g = (color >> 8) & 0xFF;
    uint8_t b = color & 0xFF;

    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;

    led_strip_set_pixel(led_strip, idx, r, g, b);
}

static uint32_t make_color(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
}

static void draw(void) {
    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, 0, 0, 0);
    }

    // Basket
    set_pixel(basket_x, 4, make_color(0, 0, 255));

    // Falling fruit
    if (fruit_y >= 0 && fruit_y <= 4) {
        set_pixel(fruit_x, fruit_y, make_color(0, 255, 0));
    }

    led_strip_refresh(led_strip);
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

static void new_fruit(void) {
    fruit_x = esp_random() % 5;
    fruit_y = 0;
}

static void game_over(void) {
    ESP_LOGI(TAG, "GAME OVER");

    for (int i = 0; i < NUM_LEDS; i++) {
        uint8_t r_val = (255 * MATRIX_BRIGHTNESS) / 255;
        led_strip_set_pixel(led_strip, i, r_val, 0, 0);
    }
    led_strip_refresh(led_strip);

    play_tone(150, 500);

    score = 0;
    lives = 3;
    basket_x = 2;
    new_fruit();

    last_fall = esp_timer_get_time() / 1000;
}

static void read_tilt(void) {
    int16_t ay = imu_read_16(imu_handle, 0x2A);

    // Ignore small tilts
    if (abs(ay) < TILT_THRESHOLD)
        return;

    uint64_t current_time = esp_timer_get_time() / 1000;
    if (current_time - last_tilt_move < TILT_MOVE_DELAY)
        return;

    last_tilt_move = current_time;

    // Y AXIS IS INVERTED: +Y -> LEFT, -Y -> RIGHT
    if (ay > TILT_THRESHOLD) {
        if (basket_x > 0)
            basket_x--;
    } else if (ay < -TILT_THRESHOLD) {
        if (basket_x < 4)
            basket_x++;
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

    // 2. Configure LED Strip via RMT
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

    // 3. Initialize SPI Bus for IMU
    spi_bus_config_t buscfg = {
        .miso_io_num = IMU_MISO,
        .mosi_io_num = IMU_MOSI,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = IMU_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_handle));

    // Initialize LSM6DSM: CTRL1_XL = 0x40 (104 Hz, ±2g)
    if (imu_write_reg(imu_handle, 0x10, 0x40) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    new_fruit();
    last_fall = esp_timer_get_time() / 1000;

    ESP_LOGI(TAG, "Catch the Falling Fruit started.");

    // 4. Main Game Loop
    while (1) {
        read_tilt();

        uint64_t current_time = esp_timer_get_time() / 1000;

        if (current_time - last_fall >= 500) {
            last_fall = current_time;
            fruit_y++;

            // Fruit has reached the basket row
            if (fruit_y >= 4) {
                if (fruit_x == basket_x) {
                    // Successfully caught
                    score++;
                    play_tone(1200, 80);
                    ESP_LOGI(TAG, "Score: %d", score);
                } else {
                    // Missed
                    lives--;
                    play_tone(200, 150);
                    ESP_LOGI(TAG, "Lives: %d", lives);
                }

                new_fruit();

                if (lives <= 0) {
                    game_over();
                }
            }
        }

        draw();

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}