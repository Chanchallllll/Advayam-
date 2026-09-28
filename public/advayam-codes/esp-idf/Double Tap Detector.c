/*
 * This program detects two rapid acceleration spikes as a double tap.
 *
 * A verified double tap unlocks the system and lights the center
 * matrix pixel green.
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

#define PIN_EXTRA          16

static const char *TAG = "double_tap";

static led_strip_handle_t led_strip;
static spi_device_handle_t imu_handle;

// =====================================================
// Helper Functions: IMU SPI Communication
// =====================================================
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

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

void app_main(void) {
    // 1. Configure Extra Pin (GPIO 16)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_EXTRA),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_EXTRA, 0);

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

    // Initialize LSM6DSOX: CTRL1_XL = 0x48 (104 Hz, 4G range)
    if (imu_write_reg(imu_handle, 0x10, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    ESP_LOGI(TAG, "Double Tap Detector Initialized.");

    uint64_t first_tap = 0;
    bool waiting_second = false;

    // 4. Main Event Loop
    while (1) {
        uint8_t raw_data[6];
        if (imu_read_regs(imu_handle, 0x28, raw_data, 6) == ESP_OK) {
            int16_t ax_raw = (int16_t)((raw_data[1] << 8) | raw_data[0]);
            int16_t ay_raw = (int16_t)((raw_data[3] << 8) | raw_data[2]);
            int16_t az_raw = (int16_t)((raw_data[5] << 8) | raw_data[4]);

            // Convert raw counts to m/s^2 (4G range scale factor ~0.0011969 m/s^2 per LSB)
            float ax = (float)ax_raw * 0.0011969f;
            float ay = (float)ay_raw * 0.0011969f;
            float az = (float)az_raw * 0.0011969f;

            float magnitude = sqrtf(ax * ax + ay * ay + az * az);
            bool tap = (magnitude > 16.0f);

            uint64_t current_time = esp_timer_get_time() / 1000;

            if (tap) {
                if (!waiting_second) {
                    first_tap = current_time;
                    waiting_second = true;
                    vTaskDelay(pdMS_TO_TICKS(100)); // Debounce tap spike
                } else if ((current_time - first_tap) < 500) {
                    // Clear matrix and light center pixel (12) green
                    for (int i = 0; i < NUM_LEDS; i++) {
                        led_strip_set_pixel(led_strip, i, 0, 0, 0);
                    }
                    
                    uint8_t g_val = (180 * MATRIX_BRIGHTNESS) / 255;
                    led_strip_set_pixel(led_strip, 12, 0, g_val, 0);
                    led_strip_refresh(led_strip);

                    ESP_LOGI(TAG, "DOUBLE TAP - UNLOCKED");

                    waiting_second = false;
                    vTaskDelay(pdMS_TO_TICKS(300));
                }
            }

            if (waiting_second && ((current_time - first_tap) > 500)) {
                waiting_second = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}