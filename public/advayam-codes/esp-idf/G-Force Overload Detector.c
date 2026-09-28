/*
 * This program monitors total acceleration.
 *
 * If acceleration exceeds 2.5g, the matrix flashes red and
 * the buzzer produces a high-frequency warning chirp.
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

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define BUZZER_PIN         16

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL       LEDC_CHANNEL_0
#define LEDC_DUTY_RES      LEDC_TIMER_8_BIT

static const char *TAG = "g_force_warning";

static led_strip_handle_t led_strip;
static spi_device_handle_t imu_handle;

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

void app_main(void) {
    // 1. Configure LEDC PWM for Buzzer Tone Generation
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = 3500,
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

    // Initialize LSM6DSOX: CTRL1_XL = 0x48 (104 Hz, 4G range)
    if (imu_write_reg(imu_handle, 0x10, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    ESP_LOGI(TAG, "G-Force Warning System Initialized.");

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

            float g = sqrtf(ax * ax + ay * ay + az * az) / 9.80665f;

            if (g > 2.5f) {
                // Fill matrix red
                uint8_t r_val = (255 * MATRIX_BRIGHTNESS) / 255;
                for (int i = 0; i < NUM_LEDS; i++) {
                    led_strip_set_pixel(led_strip, i, r_val, 0, 0);
                }
                led_strip_refresh(led_strip);

                // Sound buzzer at 3500 Hz (50% duty cycle)
                ledc_set_freq(LEDC_MODE, LEDC_TIMER, 3500);
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

                vTaskDelay(pdMS_TO_TICKS(120));

                // Stop buzzer
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);

                // Clear matrix
                led_strip_clear(led_strip);
                led_strip_refresh(led_strip);

                ESP_LOGI(TAG, "High G-Force detected: %.2f g", g);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}