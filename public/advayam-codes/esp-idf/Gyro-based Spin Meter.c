/*
 * This program measures rotation around the Z axis.
 *
 * A bright pixel travels around the matrix edge. Its speed
 * increases as the measured angular velocity increases.
 */
/*
 * Rotation Matrix Tracker (ESP-IDF / PlatformIO)
 * 
 * Measures rotation around the Z axis using an LSM6DSOX sensor over SPI.
 * A bright pixel travels around the matrix edge. Its speed
 * increases as the measured angular velocity increases.
*/

#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

static const char *TAG = "rotation_matrix";

// LSM6DSOX Registers
#define LSM6DSOX_WHO_AM_I   0x0F
#define LSM6DSOX_CTRL2_G    0x11
#define LSM6DSOX_OUTZ_L_G   0x26
#define LSM6DSOX_WHO_AM_I_VAL 0x6C

static spi_device_handle_t imu_spi = NULL;
static led_strip_handle_t led_strip = NULL;

// Edge pixels mapping (5x5 matrix perimeter)
static const uint8_t edgePixels[16] = {
    0, 1, 2, 3, 4,
    9, 14, 19, 24,
    23, 22, 21, 20,
    15, 10, 5
};

// SPI Read helper for LSM6DSOX
static esp_err_t lsm6dsox_read_reg(uint8_t reg, uint8_t *data, size_t size) {
    uint8_t tx_data[2 + size];
    uint8_t rx_data[2 + size];
    
    tx_data[0] = reg | 0x80; // Read bit set
    memset(&tx_data[1], 0, size);

    spi_transaction_t t = {
        .length = (1 + size) * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    gpio_set_level(IMU_CS, 0);
    esp_err_t err = spi_device_transmit(imu_spi, &t);
    gpio_set_level(IMU_CS, 1);

    if (err == ESP_OK) {
        memcpy(data, &rx_data[1], size);
    }
    return err;
}

// SPI Write helper for LSM6DSOX
static esp_err_t lsm6dsox_write_reg(uint8_t reg, uint8_t val) {
    uint8_t tx_data[2];
    tx_data[0] = reg & ~0x80; // Write bit cleared
    tx_data[1] = val;

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };

    gpio_set_level(IMU_CS, 0);
    esp_err_t err = spi_device_transmit(imu_spi, &t);
    gpio_set_level(IMU_CS, 1);
    return err;
}

static inline int constrain_val(int x, int low, int high) {
    return (x < low) ? low : (x > high) ? high : x;
}

static inline int map_val(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void app_main(void) {
    // 1. Configure Chip Select (CS) Pin as Output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IMU_CS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(IMU_CS, 1);

    // 2. Initialize SPI Bus for IMU
    spi_bus_config_t buscfg = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000, // 10 MHz
        .mode = 0,                          // SPI mode 0
        .spics_io_num = -1,                 // Manual CS control
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_spi));

    vTaskDelay(pdMS_TO_TICKS(50));

    // Verify LSM6DSOX Who Am I
    uint8_t whoami = 0;
    lsm6dsox_read_reg(LSM6DSOX_WHO_AM_I, &whoami, 1);
    if (whoami != LSM6DSOX_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "LSM6DSOX not found! Expected 0x%02X, got 0x%02X", LSM6DSOX_WHO_AM_I_VAL, whoami);
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    // Configure Gyro: 104 Hz rate, 500 dps range -> CTRL2_G = 0x44
    lsm6dsox_write_reg(LSM6DSOX_CTRL2_G, 0x44);

    // 3. Configure LED Strip via RMT
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

    ESP_LOGI(TAG, "Rotation Matrix Tracker Initialized");

    int position = 0;
    int64_t last_move = esp_timer_get_time() / 1000;

    // 4. Main Event Loop
    while (1) {
        uint8_t gyro_data[2];
        float gyro_z = 0.0f;

        if (lsm6dsox_read_reg(LSM6DSOX_OUTZ_L_G, gyro_data, 2) == ESP_OK) {
            int16_t raw_z = (int16_t)(((uint16_t)gyro_data[1] << 8) | gyro_data[0]);
            // Sensitivity for 500 dps is 17.50 mdps/LSB -> raw_z * 0.0175 dps
            gyro_z = (float)raw_z * 0.0175f;
        }

        float speed = fabsf(gyro_z);
        int speed_int = constrain_val((int)speed, 0, 500);
        int interval = map_val(speed_int, 0, 500, 500, 20);

        int64_t now = esp_timer_get_time() / 1000;
        if (now - last_move > interval) {
            position++;
            if (position >= 16) {
                position = 0;
            }
            last_move = now;
        }

        // Clear matrix
        for (int i = 0; i < NUM_LEDS; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }

        // Light up current edge pixel (Green, scaled by MATRIX_BRIGHTNESS)
        uint8_t g_val = (255 * MATRIX_BRIGHTNESS) / 255;
        uint8_t b_val = (50 * MATRIX_BRIGHTNESS) / 255;
        led_strip_set_pixel(led_strip, edgePixels[position], 0, g_val, b_val);
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}