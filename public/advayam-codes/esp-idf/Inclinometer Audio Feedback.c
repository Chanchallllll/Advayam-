/*
 * This program measures the board's pitch relative to a 45-degree target.
 *
 * The buzzer beeps increasingly quickly as the board approaches
 * the target angle.
 */
 /*
 * =============================================================================
 * LSM6DSOX PITCH LEVELER GAME (ESP-IDF v5+ COMPATIBLE - PURE C)
 * =============================================================================
 * Native ESP-IDF implementation replacing the Arduino LSM6DSOX SPI driver.
 * 
 * Pin Configuration:
 *   - SPI CS   : GPIO 10
 *   - SPI SCK  : GPIO 12
 *   - SPI MOSI : GPIO 11
 *   - SPI MISO : GPIO 13
 *   - Buzzer   : GPIO 16
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"

#define PIN_NUM_MISO 13
#define PIN_NUM_MOSI 11
#define PIN_NUM_SCLK 12
#define PIN_NUM_CS   10

#define BUZZER_PIN   16
#define LEDC_TIMER   LEDC_TIMER_0
#define LEDC_MODE    LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL LEDC_CHANNEL_0
#define LEDC_DUTY_RES LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY 1800 // 1800 Hz tone

#define LSM6DSOX_WHO_AM_I   0x0F
#define LSM6DSOX_CTRL1_XL   0x10
#define LSM6DSOX_OUTX_L_A   0x28

static const char *TAG = "PITCH_GAME";
static spi_device_handle_t imu_spi = NULL;

static unsigned long millis(void) {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

static esp_err_t lsm6dsox_write(spi_device_handle_t spi, uint8_t reg, uint8_t data) {
    uint8_t tx_buf[2];
    tx_buf[0] = reg & ~0x80; // Write command (MSB = 0)
    tx_buf[1] = data;
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_buf,
    };
    return spi_device_transmit(spi, &t);
}

static esp_err_t lsm6dsox_read(spi_device_handle_t spi, uint8_t reg, uint8_t *out_data, size_t len) {
    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};
    tx_buf[0] = reg | 0x80; // Read command (MSB = 1)

    spi_transaction_t t = {
        .length = 8 * (1 + len),
        .rxlength = 8 * len,
        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };
    esp_err_t err = spi_device_transmit(spi, &t);
    if (err == ESP_OK) {
        for (size_t i = 0; i < len; i++) {
            out_data[i] = rx_buf[1 + i];
        }
    }
    return err;
}

static void buzzer_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BUZZER_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
}

static void tone_on(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 4096); // 50% duty cycle
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void tone_off(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void app_main(void) {
    buzzer_init();

    // Initialize SPI Bus
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_SCLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    // Add LSM6DSOX device to SPI Bus
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_spi));

    // Verify WHO_AM_I (Expected 0x6C for LSM6DSOX)
    uint8_t who_am_i = 0;
    ESP_ERROR_CHECK(lsm6dsox_read(imu_spi, LSM6DSOX_WHO_AM_I, &who_am_i, 1));
    ESP_LOGI(TAG, "LSM6DSOX WHO_AM_I = 0x%02X", who_am_i);

    if (who_am_i != 0x6C) {
        ESP_LOGE(TAG, "LSM6DSOX not found!");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Configure Accelerometer: 104 Hz ODR, ±4G full scale (CTRL1_XL = 0x48)
    ESP_ERROR_CHECK(lsm6dsox_write(imu_spi, LSM6DSOX_CTRL1_XL, 0x48));

    unsigned long lastBeep = 0;
    unsigned long beepDurationStart = 0;
    bool beepActive = false;

    ESP_LOGI(TAG, "=== PITCH LEVELER GAME STARTED ===");

    while (1) {
        uint8_t raw_data[6];
        if (lsm6dsox_read(imu_spi, LSM6DSOX_OUTX_L_A, raw_data, 6) != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        int16_t accel_x = (int16_t)(((uint16_t)raw_data[1] << 8) | raw_data[0]);
        int16_t accel_y = (int16_t)(((uint16_t)raw_data[3] << 8) | raw_data[2]);
        int16_t accel_z = (int16_t)(((uint16_t)raw_data[5] << 8) | raw_data[4]);

        float pitch = atan2(
            -(float)accel_x,
            sqrt((float)accel_y * accel_y + (float)accel_z * accel_z)
        ) * 180.0f / 3.14159265f;

        float error = fabsf(45.0f - pitch);

        if (error < 30.0f) {
            // Map error (0 to 30) to interval (80 to 700 ms)
            int constrained_error = (int)error;
            if (constrained_error < 0) constrained_error = 0;
            if (constrained_error > 30) constrained_error = 30;

            int interval = 80 + ((constrained_error - 0) * (700 - 80)) / (30 - 0);

            if (millis() - lastBeep > (unsigned long)interval) {
                tone_on();
                beepDurationStart = millis();
                beepActive = true;
                lastBeep = millis();
            }
        }

        // Turn off buzzer after 50ms pulse duration
        if (beepActive && (millis() - beepDurationStart >= 50)) {
            tone_off();
            beepActive = false;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}