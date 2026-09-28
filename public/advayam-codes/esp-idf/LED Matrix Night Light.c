/*
 * =============================================================================
 * NIGHT LIGHT CONTROLLER
 * =============================================================================
 * Turns the RGB matrix into a soft warm-white night light when ambient
 * brightness falls below DARK_LUX_THRESHOLD.
 *
 * Matrix:
 *   GPIO 7
 * =============================================================================
 */
 /*
 * =============================================================================
 * NIGHT LIGHT CONTROLLER (ESP-IDF v5+ COMPATIBLE - PURE C)
 * =============================================================================
 * Turns the RGB matrix into a soft warm-white night light when ambient
 * brightness falls below DARK_LUX_THRESHOLD.
 * 
 * Pin Configuration:
 *   - I2C SDA  : GPIO 5
 *   - I2C SCL  : GPIO 6
 *   - Matrix   : GPIO 7 (WS2812 / NeoPixel via ESP-IDF led_strip component)
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "led_strip.h"
#include "rpr0521rs.h"

#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000

#define MATRIX_PIN           7
#define NUM_PIXELS           25
#define DARK_LUX_THRESHOLD   20

static const char *TAG = "NIGHT_LIGHT";
static i2c_master_dev_handle_t sensor_dev = NULL;
static led_strip_handle_t led_strip = NULL;

// Initialize Modern ESP-IDF v5+ I2C Master Bus and Add Device
static void i2c_master_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        }
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &sensor_dev));
}

// Initialize LED Strip using ESP-IDF led_strip component
static void led_strip_init_board(void) {
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_PIXELS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,           
        .flags = {
            .invert_out = false,
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz resolution
        .flags = {
            .with_dma = false,
        }
    };

    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    ESP_ERROR_CHECK(led_strip_clear(led_strip));
}

void app_main(void) {
    i2c_master_init();
    led_strip_init_board();

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {
        ESP_LOGE(TAG, "Sensor initialization failed!");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI(TAG, "=== NIGHT LIGHT CONTROLLER STARTED ===");

    while (1) {
        uint16_t proximity;
        float lux;

        if (rpr0521rs_get_psalsval(sensor_dev, &proximity, &lux) == ESP_OK) {
            ESP_ERROR_CHECK(led_strip_clear(led_strip));

            if (lux < (float)DARK_LUX_THRESHOLD) {
                // Apply brightness scaling factor (30/255 ≈ 0.1176) matching matrix.setBrightness(30)
                // Warm-white base: R=255, G=120, B=30
                uint8_t red = (uint8_t)(255 * 30 / 255);
                uint8_t green = (uint8_t)(120 * 30 / 255);
                uint8_t blue = (uint8_t)(30 * 30 / 255);

                for (int i = 0; i < NUM_PIXELS; i++) {
                    led_strip_set_pixel(led_strip, i, red, green, blue);
                }
            }

            ESP_ERROR_CHECK(led_strip_refresh(led_strip));
            ESP_LOGI(TAG, "Ambient lux: %.2f", (double)lux);
        }

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}