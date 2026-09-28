#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

#define BUTTON_PIN         17
#define BUZZER_PIN         16

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define LEDC_TIMER         LEDC_TIMER_0
#define LEDC_MODE          LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL       LEDC_CHANNEL_0
#define LEDC_DUTY_RES      LEDC_TIMER_8_BIT
#define LEDC_FREQUENCY     1000

#define LSM6DSOX_CTRL1_XL  0x10
#define LSM6DSOX_OUTX_L_XL 0x28

static const char *TAG = "alarm_system";

// Helper: Write to LSM6DSOX register via SPI
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

// Helper: Read from LSM6DSOX registers via SPI
static esp_err_t imu_read_regs(spi_device_handle_t handle, uint8_t reg, uint8_t *rx_buf, size_t len) {
    uint8_t *temp_tx = malloc(len + 1);
    uint8_t *temp_rx = malloc(len + 1);
    if (!temp_tx || !temp_rx) {
        free(temp_tx);
        free(temp_rx);
        return ESP_ERR_NO_MEM;
    }
    temp_tx[0] = reg | 0x80; // Read bit set
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
    // 1. Configure Button GPIO with Internal Pull-up
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);

    // 2. Configure LEDC PWM for Buzzer
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = LEDC_FREQUENCY,
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
    spi_device_handle_t imu_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_handle));

    // 4. Configure LSM6DSOX Accelerometer (±4g range, 104 Hz -> register value 0x48)
    if (imu_write_reg(imu_handle, LSM6DSOX_CTRL1_XL, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    ESP_LOGI(TAG, "Security Alarm System Initialized.");

    bool armed = false;
    float baseline = 9.81f;

    // 5. Main Task Loop
    while (1) {
        // Check button press (Active LOW)
        if (gpio_get_level(BUTTON_PIN) == 0) {
            armed = true;
            ESP_LOGI(TAG, "ARMED");
            vTaskDelay(pdMS_TO_TICKS(500)); // Debounce / delay
        }

        uint8_t raw_data[6];
        if (imu_read_regs(imu_handle, LSM6DSOX_OUTX_L_XL, raw_data, 6) == ESP_OK) {
            int16_t ax_raw = (int16_t)((raw_data[1] << 8) | raw_data[0]);
            int16_t ay_raw = (int16_t)((raw_data[3] << 8) | raw_data[2]);
            int16_t az_raw = (int16_t)((raw_data[5] << 8) | raw_data[4]);

            // Convert raw values to m/s^2 for ±4g scale (sensitivity ~0.122 mg/LSB)
            float ax = (float)ax_raw * 0.122f * 0.001f * 9.80665f;
            float ay = (float)ay_raw * 0.122f * 0.001f * 9.80665f;
            float az = (float)az_raw * 0.122f * 0.001f * 9.80665f;

            float magnitude = sqrtf(ax * ax + ay * ay + az * az);

            if (armed && fabsf(magnitude - baseline) > 1.5f) {
                // Trigger buzzer frequency sweep alert
                for (int frequency = 600; frequency <= 2400; frequency += 300) {
                    ledc_set_freq(LEDC_MODE, LEDC_TIMER, frequency);
                    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 128);
                    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
                    vTaskDelay(pdMS_TO_TICKS(100));
                }
                ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
                ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}