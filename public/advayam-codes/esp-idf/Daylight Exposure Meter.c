/*
 * =============================================================================
 * DAYLIGHT EXPOSURE METER
 * =============================================================================
 * Tracks cumulative time spent above a configurable ambient-light threshold.
 *
 * The accumulated exposure time is reported over UART.
 *
 * This is an electronics demonstration and is NOT a medical or UV exposure
 * measurement device. The RPR-0521RS measures visible/ambient light, not UV.
 * =============================================================================
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rpr0521rs.h"

#define I2C_SDA_PIN          GPIO_NUM_5
#define I2C_SCL_PIN          GPIO_NUM_6
#define I2C_PORT             I2C_NUM_0
#define BRIGHT_LUX_THRESHOLD 300.0f

static const char *TAG = "DAYLIGHT_METER";

void app_main(void) {
    ESP_LOGI(TAG, "Daylight Exposure Meter started.");

    // 1. Initialize Modern ESP-IDF v5.x I2C Master Bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = { .enable_internal_pullup = true }
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    // 2. Add RPR-0521RS Device to Bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t sensor_dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &sensor_dev));

    // 3. Initialize Sensor using the custom driver header
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {
        ESP_LOGE(TAG, "Sensor initialization failed!");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    int64_t last_update = esp_timer_get_time() / 1000;
    uint32_t bright_milliseconds = 0;

    while (1) {
        int64_t now = esp_timer_get_time() / 1000;
        uint32_t elapsed = (uint32_t)(now - last_update);
        last_update = now;

        uint16_t proximity = 0;
        float lux = 0.0f;

        if (rpr0521rs_get_psalsval(sensor_dev, &proximity, &lux) == ESP_OK) {
            // Accumulate time only when ambient light exceeds the threshold
            if (lux >= BRIGHT_LUX_THRESHOLD) {
                bright_milliseconds += elapsed;
            }

            uint32_t total_seconds = bright_milliseconds / 1000;
            uint32_t minutes = total_seconds / 60;
            uint32_t seconds = total_seconds % 60;

            ESP_LOGI(TAG, "Lux: %.1f | Bright exposure: %lu min %lu sec", lux, (unsigned long)minutes, (unsigned long)seconds);
        } else {
            ESP_LOGW(TAG, "Failed to read sensor data");
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}