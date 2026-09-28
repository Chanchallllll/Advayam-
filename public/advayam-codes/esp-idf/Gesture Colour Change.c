/*
 * This program detects a quick left-to-right flick of the board.
 *
 * Each detected gesture changes the RGB matrix to the next
 * color in a predefined palette.
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
#include "led_strip.h"
#include "esp_timer.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

static const char *TAG = "flick_gesture";

static led_strip_handle_t led_strip;
static spi_device_handle_t imu_handle;

static uint8_t palette = 0;
static uint64_t last_gesture = 0;

// Color palette definition (RGB)
static const uint32_t colors[6][3] = {
    {255, 0, 0},    // Red
    {0, 255, 0},    // Green
    {0, 0, 255},    // Blue
    {255, 255, 0},  // Yellow
    {255, 0, 255},  // Magenta
    {0, 255, 255}   // Cyan
};

// Helper: Write register to LSM6DSOX over SPI
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

// Helper: Read registers from LSM6DSOX over SPI
static esp_err_t imu_read_regs(spi_device_handle_t handle, uint8_t reg, uint8_t *rx_buf, size_t len) {
    uint8_t *temp_tx = malloc(len + 1);
    uint8_t *temp_rx = malloc(len + 1);
    if (!temp_tx || !temp_rx) {
        free(temp_tx);
        free(temp_rx);
        return ESP_ERR_NO_MEM;
    }
    temp_tx[0] = reg | 0x80;
    memset(&temp_tx[1], 0, len);

    spi_transaction_t t = {
        .length = (len + 1) * 8,
        .tx_buffer = temp_tx,
        .rx_buffer = temp_rx,
    };
    esp_err_t err = spi_device_transmit(handle, &t);
    if (err == ESP_OK) {
        memcpy(rx_buf, &temp_rx[1], len);
    }
    free(temp_tx);
    free(temp_rx);
    return err;
}

static void show_color(void) {
    uint8_t r = (colors[palette][0] * MATRIX_BRIGHTNESS) / 255;
    uint8_t g = (colors[palette][1] * MATRIX_BRIGHTNESS) / 255;
    uint8_t b = (colors[palette][2] * MATRIX_BRIGHTNESS) / 255;

    for (int i = 0; i < NUM_LEDS; i++) {
        led_strip_set_pixel(led_strip, i, r, g, b);
    }
    led_strip_refresh(led_strip);
}

void app_main(void) {
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

    // 2. Initialize SPI Bus for IMU
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

    // Initialize LSM6DSOX: CTRL1_XL = 0x48 (104 Hz, 4G range)
    if (imu_write_reg(imu_handle, 0x10, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    ESP_LOGI(TAG, "Flick Gesture Detector Initialized.");

    show_color();

    // 3. Main Event Loop
    while (1) {
        uint8_t raw_data[6];
        if (imu_read_regs(imu_handle, 0x28, raw_data, 6) == ESP_OK) {
            int16_t ay_raw = (int16_t)((raw_data[3] << 8) | raw_data[2]);

            // Convert raw counts to m/s^2 (4G range scale factor ~0.0011969 m/s^2 per LSB)
            float ay = (float)ay_raw * 0.0011969f;

            uint64_t current_time = esp_timer_get_time() / 1000;

            if (ay < -14.0f && (current_time - last_gesture) > 500) {
                palette++;
                palette %= 6;

                show_color();

                last_gesture = current_time;
                ESP_LOGI(TAG, "Flick detected! Palette index: %u", palette);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}