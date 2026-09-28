/*
 * =============================================================================
 * PROXIMITY COUNTER - ESP-IDF v5+
 * =============================================================================
 * Counts how many times an object enters the configured proximity range.
 *
 * Each object must leave the detection area before another count can occur.
 *
 * UART:
 *   ESP-IDF logging at 115200 baud
 *
 * I2C:
 *   SDA: GPIO 5
 *   SCL: GPIO 6
 * =============================================================================
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "rpr0521rs.h"


#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000

#define PROXIMITY_THRESHOLD  500


static const char *TAG = "PROXIMITY_COUNTER";

static i2c_master_dev_handle_t sensor_dev = NULL;


/*
 * =============================================================================
 * Initialize I2C Master Bus and RPR-0521RS Device
 * =============================================================================
 */
static void i2c_master_init(void)
{
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

    ESP_ERROR_CHECK(
        i2c_new_master_bus(&bus_config, &bus_handle)
    );


    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };

    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            bus_handle,
            &dev_config,
            &sensor_dev
        )
    );
}


/*
 * =============================================================================
 * Application
 * =============================================================================
 */
void app_main(void)
{
    /*
     * Initialize I2C.
     */
    i2c_master_init();


    /*
     * Initialize RPR-0521RS sensor.
     */
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(TAG, "Sensor initialization failed!");

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }


    /*
     * Counter state.
     */
    unsigned long objectCount = 0;
    bool objectPresent = false;


    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "     PROXIMITY COUNTER STARTED");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Threshold: %d", PROXIMITY_THRESHOLD);


    /*
     * Main loop.
     *
     * Equivalent to the Arduino:
     *
     *     delay(50);
     *
     * so we sample approximately every 50 ms.
     */
    while (1) {

        uint16_t proximity;
        float lux;


        /*
         * Read proximity + ambient light value.
         */
        if (rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            ) == ESP_OK) {


            /*
             * Object is considered detected when proximity
             * exceeds the configured threshold.
             */
            bool detected =
                proximity > PROXIMITY_THRESHOLD;


            /*
             * Count only the rising edge:
             *
             *     NOT DETECTED -> DETECTED
             *
             * This prevents the same object from being counted
             * repeatedly while it remains in front of the sensor.
             */
            if (detected && !objectPresent) {

                objectCount++;

                ESP_LOGI(
                    TAG,
                    "Object detected! Total count: %lu",
                    objectCount
                );
            }


            /*
             * Save current detection state.
             *
             * When the object leaves:
             *
             *     detected = false
             *     objectPresent = false
             *
             * allowing the next object to be counted.
             */
            objectPresent = detected;


        } else {

            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );
        }


        /*
         * Sample every 50 ms.
         */
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
