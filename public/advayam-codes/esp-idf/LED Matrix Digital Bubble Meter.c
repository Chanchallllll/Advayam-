/*
 * This program uses the accelerometer to create a digital bubble level.
 *
 * The green center LED represents the bubble. The bubble moves
 * according to board tilt and stays centered when the board is level.
 */
 #include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13
#define POWER_PIN          16

#define LSM6DS_WHO_AM_I    0x0F
#define LSM6DS_CTRL1_XL    0x10
#define LSM6DS_OUTX_L_XL   0x28
#define LSM6DS_WHO_AM_I_VAL 0x6C

static const char *TAG = "digital_bubble_level";
static led_strip_handle_t led_strip;
static spi_device_handle_t spi_imu;

// Convert x/y coordinates to the serpentine LED index.
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) 
        return y * 5 + x;
    return y * 5 + (4 - x);
}

static int constrain_val(int val, int min_val, int max_val) {
    if (val < min_val) return min_val;
    if (val > max_val) return max_val;
    return val;
}

static void lsm6ds_write_reg(uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = { reg & 0x7F, val };
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    spi_device_transmit(spi_imu, &t);
}

static void lsm6ds_read_regs(uint8_t reg, uint8_t *data, size_t len) {
    uint8_t tx_data[1 + len];
    uint8_t rx_data[1 + len];
    memset(tx_data, 0, sizeof(tx_data));
    tx_data[0] = reg | 0x80; // Read bit set

    spi_transaction_t t = {
        .length = (1 + len) * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    spi_device_transmit(spi_imu, &t);
    memcpy(data, &rx_data[1], len);
}

void app_main(void) {
    // 1. Configure Power Pin (GPIO 16)
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << POWER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(POWER_PIN, 0);

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

    // 3. Configure SPI Bus for IMU
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
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 0,
        .spics_io_num = IMU_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_imu));

    // 4. Verify IMU Connection
    uint8_t whoami = 0;
    lsm6ds_read_regs(LSM6DS_WHO_AM_I, &whoami, 1);
    if (whoami != LSM6DS_WHO_AM_I_VAL) {
        ESP_LOGE(TAG, "IMU initialization failed! WHO_AM_I: 0x%02X", whoami);
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Set accelerometer range to +/- 4g and rate to 104 Hz
    // CTRL1_XL: ODR_XL = 104Hz (0100), FS_XL = +-4g (10) -> 0x48
    lsm6ds_write_reg(LSM6DS_CTRL1_XL, 0x48);

    ESP_LOGI(TAG, "Digital Bubble Level Initialized.");

    // 5. Main Loop
    while (1) {
        uint8_t data[6];
        lsm6ds_read_regs(LSM6DS_OUTX_L_XL, data, 6);

        int16_t raw_x = (int16_t)(((uint16_t)data[1] << 8) | data[0]);
        int16_t raw_y = (int16_t)(((uint16_t)data[3] << 8) | data[2]);

        // Convert raw values to m/s^2 (Sensitivity for +/- 4g is 0.122 mg/LSB)
        float accel_x = (float)raw_x * 0.0011965f;
        float accel_y = (float)raw_y * 0.0011965f;

        // Convert acceleration into approximate tilt position
        int x = constrain_val(2 - (int)roundf(accel_y / 2.5f), 0, 4);
        int y = constrain_val(2 + (int)roundf(accel_x / 2.5f), 0, 4);

        // Clear matrix
        for (int i = 0; i < NUM_LEDS; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }

        // Set green bubble pixel with brightness scaling
        uint8_t g_val = (255 * MATRIX_BRIGHTNESS) / 255;
        int idx = pixel_xy(x, y);
        led_strip_set_pixel(led_strip, idx, 0, g_val, 0);
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}