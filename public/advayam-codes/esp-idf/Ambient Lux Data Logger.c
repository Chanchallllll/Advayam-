/*
 * =============================================================================
 * AMBIENT LUX DATA LOGGER
 * =============================================================================
 * ESP-IDF v5+ compatible
 *
 * Samples ambient light every minute and stores the readings in the ESP32-S3
 * internal flash filesystem using LittleFS.
 *
 * Log file:
 *   /littlefs/lux_log.csv
 *
 * CSV format:
 *   Elapsed_Minutes,Lux
 *   1,123.45
 *   2,119.82
 *   3,121.17
 *
 * NOTE:
 * A correct partition table must be created for this code to run properly.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * Sensor:
 *   RPR-0521RS
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "esp_littlefs.h"

#include "rpr0521rs.h"


/* =============================================================================
 * Configuration
 * =============================================================================
 */

#define I2C_MASTER_SDA_IO       5
#define I2C_MASTER_SCL_IO       6
#define I2C_MASTER_FREQ_HZ      100000

#define I2C_PORT                I2C_NUM_0

#define LOG_INTERVAL_MS         60000ULL

/*
 * LittleFS partition label.
 *
 * This must match the "Partition Label" used by your LittleFS partition
 * in partitions.csv.
 */
#define LITTLEFS_PARTITION_LABEL    "storage"

/*
 * Mount point used by ESP-IDF.
 */
#define LITTLEFS_BASE_PATH          "/littlefs"

#define LOG_FILE                    LITTLEFS_BASE_PATH "/lux_log.csv"


/* =============================================================================
 * Globals
 * =============================================================================
 */

static const char *TAG = "LUX_LOGGER";

static i2c_master_dev_handle_t sensor_dev = NULL;


/* =============================================================================
 * I2C Initialization
 * =============================================================================
 */

static void i2c_master_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
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


    /*
     * RPR-0521RS I2C device configuration.
     *
     * RPR0521RS_DEVICE_ADDRESS comes from rpr0521rs.h.
     */
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

    ESP_LOGI(TAG, "I2C initialized");
}


/* =============================================================================
 * LittleFS Initialization
 * =============================================================================
 */

static void littlefs_init(void)
{
    esp_vfs_littlefs_conf_t conf = {
        .base_path = LITTLEFS_BASE_PATH,
        .partition_label = LITTLEFS_PARTITION_LABEL,

        /*
         * Set to true if you want ESP-IDF to automatically format the
         * partition when mounting fails.
         */
        .format_if_mount_failed = true,

        /*
         * Automatically format the filesystem if the partition is
         * detected but contains an invalid filesystem.
         */
        .dont_mount = false,
    };

    esp_err_t ret = esp_vfs_littlefs_register(&conf);

    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "LittleFS initialization failed: %s",
                 esp_err_to_name(ret));

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI(TAG, "LittleFS mounted at %s", LITTLEFS_BASE_PATH);
}


/* =============================================================================
 * Create CSV Header
 * =============================================================================
 */

static void create_log_file_if_needed(void)
{
    FILE *file = fopen(LOG_FILE, "r");

    if (file != NULL) {
        /*
         * File already exists.
         */
        fclose(file);

        ESP_LOGI(TAG, "Existing log file found: %s", LOG_FILE);
        return;
    }


    /*
     * File doesn't exist, so create it and write the CSV header.
     */
    file = fopen(LOG_FILE, "w");

    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to create log file: %s", LOG_FILE);

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    fprintf(file, "Elapsed_Minutes,Lux\n");

    fclose(file);

    ESP_LOGI(TAG, "Created new log file: %s", LOG_FILE);
}


/* =============================================================================
 * Log One Reading
 * =============================================================================
 */

static void log_lux(uint64_t elapsed_minutes, float lux)
{
    FILE *file = fopen(LOG_FILE, "a");

    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open log file for append");
        return;
    }


    fprintf(
        file,
        "%llu,%.2f\n",
        (unsigned long long)elapsed_minutes,
        (double)lux
    );

    fclose(file);


    ESP_LOGI(
        TAG,
        "Logged: %llu min, %.2f lux",
        (unsigned long long)elapsed_minutes,
        (double)lux
    );
}


/* =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * -------------------------------------------------------------------------
     * Initialize I2C
     * -------------------------------------------------------------------------
     */
    i2c_master_init();


    /*
     * -------------------------------------------------------------------------
     * Initialize LittleFS
     * -------------------------------------------------------------------------
     */
    littlefs_init();


    /*
     * -------------------------------------------------------------------------
     * Initialize RPR-0521RS
     * -------------------------------------------------------------------------
     */
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(TAG, "Sensor initialization failed!");

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }


    ESP_LOGI(TAG, "RPR-0521RS initialized");


    /*
     * -------------------------------------------------------------------------
     * Create CSV file/header if necessary
     * -------------------------------------------------------------------------
     */
    create_log_file_if_needed();


    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "AMBIENT LUX LOGGER STARTED");
    ESP_LOGI(TAG, "Logging interval: 60 seconds");
    ESP_LOGI(TAG, "Log file: %s", LOG_FILE);
    ESP_LOGI(TAG, "========================================");


    /*
     * -------------------------------------------------------------------------
     * Logging loop
     * -------------------------------------------------------------------------
     *
     * esp_timer_get_time() returns microseconds since boot.
     */
    int64_t start_time_us = esp_timer_get_time();

    uint64_t elapsed_minutes = 0;


    while (1) {

        /*
         * Wait approximately one minute between readings.
         *
         * The first measurement occurs after one minute, matching the
         * original Arduino implementation.
         */
        vTaskDelay(pdMS_TO_TICKS(60000));


        /*
         * Calculate elapsed time since startup.
         */
        int64_t elapsed_us =
            esp_timer_get_time() - start_time_us;

        elapsed_minutes =
            (uint64_t)(elapsed_us / 60000000LL);


        /*
         * ---------------------------------------------------------------------
         * Read RPR-0521RS
         * ---------------------------------------------------------------------
         */
        uint16_t proximity = 0;
        float lux = 0.0f;


        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (ret == ESP_OK) {

            log_lux(elapsed_minutes, lux);

            ESP_LOGI(
                TAG,
                "Ambient lux: %.2f",
                (double)lux
            );

        } else {

            ESP_LOGE(
                TAG,
                "Failed to read RPR-0521RS: %s",
                esp_err_to_name(ret)
            );
        }
    }
}
