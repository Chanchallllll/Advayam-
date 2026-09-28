#include <stdio.h>
#include <math.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define LSM6DSOX_CTRL1_XL  0x10
#define LSM6DSOX_OUTX_L_XL 0x28

static const char *TAG = "imu_matrix";

// Helper: Write to LSM6DSOX register via SPI
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val}; // MSB 0 for write
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

// Helper: Read from LSM6DSOX registers via SPI
static esp_err_t imu_read_regs(spi_device_handle_t handle, uint8_t reg, uint8_t *rx_buf, size_t len) {
    uint8_t tx_prefix = reg | 0x80; // MSB 1 for read
    spi_transaction_t t = {
        .length = 8 + (len * 8),
        .tx_data = {tx_prefix, 0},
        .rx_buffer = rx_buf,
        .flags = SPI_TRANS_USE_RXDATA | SPI_TRANS_USE_TXDATA,
    };
    // For simplicity with multi-byte reads, use separate transmit/receive or a custom tx/rx buffer
    uint8_t *temp_tx = malloc(len + 1);
    uint8_t *temp_rx = malloc(len + 1);
    if (!temp_tx || !temp_rx) {
        free(temp_tx);
        free(temp_rx);
        return ESP_ERR_NO_MEM;
    }
    temp_tx[0] = reg | 0x80;
    memset(&temp_tx[1], 0, len);

    spi_transaction_t t_ext = {
        .length = (len + 1) * 8,
        .tx_buffer = temp_tx,
        .rx_buffer = temp_rx,
    };
    esp_err_t err = spi_device_transmit(handle, &t_ext);
    if (err == ESP_OK) {
        memcpy(rx_buf, &temp_rx[1], len);
    }
    free(temp_tx);
    free(temp_rx);
    return err;
}

// Simple HSV to RGB conversion with gamma approximation matching NeoPixel gamma
static void hsv_to_rgb(uint16_t hue, uint8_t sat, uint8_t val, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t region = hue / 10922; // 65536 / 6
    uint32_t remainder = (hue % 10922) * 6;
    
    uint8_t p = (val * (255 - sat)) >> 8;
    uint8_t q = (val * (255 - ((sat * remainder) >> 16))) >> 8;
    uint8_t t = (val * (255 - ((sat * (65535 - remainder)) >> 16))) >> 8;

    uint8_t dr = 0, dg = 0, db = 0;
    switch (region) {
        case 0: dr = val; dg = t; db = p; break;
        case 1: dr = q; dg = val; db = p; break;
        case 2: dr = p; dg = val; db = t; break;
        case 3: dr = p; dg = q; db = val; break;
        case 4: dr = t; dg = p; db = val; break;
        default: dr = val; dg = p; db = q; break;
    }

    // Apply basic gamma correction (gamma8 approximation)
    *r = (dr * dr) >> 8;
    *g = (dg * dg) >> 8;
    *b = (db * db) >> 8;
}

void app_main(void) {
    // 1. Initialize LED strip using official ESP-IDF led_strip component (RMT backend)
    led_strip_handle_t led_strip;
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10 MHz
        .flags.with_dma = false,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));

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
        .clock_speed_hz = 1 * 1000 * 1000, // 1 MHz
        .mode = 0,                          // SPI mode 0 (CPOL=0, CPHA=0) or Mode 3 work for LSM6DSOX
        .spics_io_num = IMU_CS,
        .queue_size = 7,
    };
    spi_device_handle_t imu_handle;
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_handle));

    // 3. Configure LSM6DSOX Accelerometer (104 Hz, ±4g range -> 0x48)
    if (imu_write_reg(imu_handle, LSM6DSOX_CTRL1_XL, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(1000)); }
    }

    ESP_LOGI(TAG, "IMU and LED Matrix initialized successfully.");

    // 4. Main Loop
    while (1) {
        uint8_t raw_data[6];
        if (imu_read_regs(imu_handle, LSM6DSOX_OUTX_L_XL, raw_data, 6) == ESP_OK) {
            int16_t ax = (int16_t)((raw_data[1] << 8) | raw_data[0]);
            int16_t ay = (int16_t)((raw_data[3] << 8) | raw_data[2]);

            // Calculate tilt angle using acceleration ratio
            float angle = atan2f((float)ay, (float)ax) * 180.0f / M_PI;
            if (angle < 0) {
                angle += 360.0f;
            }

            uint16_t hue = (uint16_t)(angle * 65535.0f / 360.0f);
            uint8_t r, g, b;
            hsv_to_rgb(hue, 255, 25, &r, &g, &b); // brightness scaled via value/brightness argument

            // Fill all LEDs in the strip
            for (int i = 0; i < NUM_LEDS; i++) {
                led_strip_set_pixel(led_strip, i, r, g, b);
            }
            led_strip_refresh(led_strip);
        }

        vTaskDelay(pdMS_TO_TICKS(30));
    }
}