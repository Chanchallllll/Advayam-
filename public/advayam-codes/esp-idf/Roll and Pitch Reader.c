/*
 * =============================================================================
 * LSM6DSOX ROLL + PITCH STREAMER - ESP-IDF v5+
 * =============================================================================
 *
 * Output:
 *
 *     time_ms,roll,pitch
 *
 * SPI:
 *     CS   = GPIO 10
 *     SCK  = GPIO 12
 *     MOSI = GPIO 11
 *     MISO = GPIO 13
 *
 * LSM6DSOX:
 *     Accelerometer = +/-4g
 *     ODR           = 104 Hz
 *     SPI mode      = 0
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_timer.h"


// =============================================================================
// PINS
// =============================================================================

#define PIN_NUM_MISO    13
#define PIN_NUM_MOSI    11
#define PIN_NUM_SCLK    12
#define PIN_NUM_CS      10

#define GPIO_CONTROL    16


// =============================================================================
// LSM6DSOX REGISTERS
// =============================================================================

#define LSM6DSOX_WHO_AM_I       0x0F
#define LSM6DSOX_CTRL1_XL       0x10
#define LSM6DSOX_CTRL3_C        0x12
#define LSM6DSOX_OUTX_L_A       0x28

#define LSM6DSOX_WHO_AM_I_VALUE 0x6C


// =============================================================================
// CONFIGURATION
// =============================================================================
//
// CTRL1_XL:
//
// ODR_XL = 104 Hz
// FS_XL  = +/-4g
//
// 0100 10 00 = 0x48
//
// =============================================================================

#define ACCEL_CONFIG    0x48


// =============================================================================
// CTRL3_C
// =============================================================================
//
// BDU     = bit 6 = 1
// IF_INC  = bit 2 = 1
//
// 0100 0100 = 0x44
//
// BDU:
//   Output registers are not updated until both LSB and MSB have been read.
//
// IF_INC:
//   Register address automatically increments during multi-byte SPI reads.
//
// =============================================================================

#define CTRL3_C_CONFIG  0x44


#define PI_F 3.14159265358979323846f

static const char *TAG = "LSM6DSOX";

static spi_device_handle_t imu_spi = NULL;


// =============================================================================
// MILLIS
// =============================================================================

static uint64_t millis(void)
{
    return esp_timer_get_time() / 1000ULL;
}


// =============================================================================
// SPI WRITE
// =============================================================================

static esp_err_t lsm6dsox_write(
    spi_device_handle_t spi,
    uint8_t reg,
    uint8_t data)
{
    uint8_t tx_buf[2];

    /*
     * SPI write:
     *
     * bit 7 = 0
     */

    tx_buf[0] = reg & 0x7F;
    tx_buf[1] = data;

    spi_transaction_t t = {
        .length = 16,
        .rxlength = 0,
        .tx_buffer = tx_buf,
        .rx_buffer = NULL,
    };

    return spi_device_transmit(spi, &t);
}


// =============================================================================
// SPI READ
// =============================================================================

static esp_err_t lsm6dsox_read(
    spi_device_handle_t spi,
    uint8_t reg,
    uint8_t *out_data,
    size_t len)
{
    /*
     * We need:
     *
     *   1 byte register address
     *   N bytes sensor data
     *
     * Total = 1 + len bytes.
     */

    if (len > 63) {
        return ESP_ERR_INVALID_SIZE;
    }

    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};

    /*
     * SPI read:
     *
     * bit 7 = 1
     */

    tx_buf[0] = reg | 0x80;

    spi_transaction_t t = {
        .length = 8 * (1 + len),

        /*
         * IMPORTANT:
         *
         * Receive the entire transaction, including the
         * register-address byte.
         */
        .rxlength = 8 * (1 + len),

        .tx_buffer = tx_buf,
        .rx_buffer = rx_buf,
    };

    esp_err_t err = spi_device_transmit(spi, &t);

    if (err != ESP_OK) {
        return err;
    }

    /*
     * rx_buf[0] is associated with the address phase.
     *
     * Actual register data starts at rx_buf[1].
     */

    for (size_t i = 0; i < len; i++) {
        out_data[i] = rx_buf[i + 1];
    }

    return ESP_OK;
}


// =============================================================================
// READ ACCELEROMETER RAW VALUES
// =============================================================================

static esp_err_t lsm6dsox_read_accel(
    int16_t *x,
    int16_t *y,
    int16_t *z)
{
    uint8_t raw[6];

    esp_err_t err = lsm6dsox_read(
        imu_spi,
        LSM6DSOX_OUTX_L_A,
        raw,
        6
    );

    if (err != ESP_OK) {
        return err;
    }

    /*
     * X:
     *
     * OUTX_L_A = 0x28
     * OUTX_H_A = 0x29
     */

    *x = (int16_t)(
        ((uint16_t)raw[1] << 8) |
        raw[0]
    );

    /*
     * Y:
     *
     * OUTY_L_A = 0x2A
     * OUTY_H_A = 0x2B
     */

    *y = (int16_t)(
        ((uint16_t)raw[3] << 8) |
        raw[2]
    );

    /*
     * Z:
     *
     * OUTZ_L_A = 0x2C
     * OUTZ_H_A = 0x2D
     */

    *z = (int16_t)(
        ((uint16_t)raw[5] << 8) |
        raw[4]
    );

    return ESP_OK;
}


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // GPIO 16
    // =========================================================================

    gpio_config_t gpio_cfg = {
        .pin_bit_mask = (1ULL << GPIO_CONTROL),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(gpio_config(&gpio_cfg));

    gpio_set_level(GPIO_CONTROL, 0);


    // =========================================================================
    // SPI BUS
    // =========================================================================

    spi_bus_config_t buscfg = {
        .mosi_io_num = PIN_NUM_MOSI,
        .miso_io_num = PIN_NUM_MISO,
        .sclk_io_num = PIN_NUM_SCLK,

        .quadwp_io_num = -1,
        .quadhd_io_num = -1,

        .max_transfer_sz = 64,
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST,
            &buscfg,
            SPI_DMA_CH_AUTO
        )
    );


    // =========================================================================
    // SPI DEVICE
    // =========================================================================

    spi_device_interface_config_t devcfg = {
        /*
         * Start conservatively at 1 MHz.
         *
         * Once everything works, you can increase this to 10 MHz.
         */
        .clock_speed_hz = 1 * 1000 * 1000,

        .mode = 0,

        .spics_io_num = PIN_NUM_CS,

        .queue_size = 1,
    };

    ESP_ERROR_CHECK(
        spi_bus_add_device(
            SPI2_HOST,
            &devcfg,
            &imu_spi
        )
    );


    // =========================================================================
    // WHO AM I
    // =========================================================================

    uint8_t who_am_i = 0;

    ESP_ERROR_CHECK(
        lsm6dsox_read(
            imu_spi,
            LSM6DSOX_WHO_AM_I,
            &who_am_i,
            1
        )
    );

    printf(
        "LSM6DSOX WHO_AM_I = 0x%02X\n",
        who_am_i
    );

    if (who_am_i != LSM6DSOX_WHO_AM_I_VALUE) {

        ESP_LOGE(
            TAG,
            "Wrong WHO_AM_I! Expected 0x6C"
        );

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }


    // =========================================================================
    // CONFIGURE CTRL3_C
    // =========================================================================

    ESP_ERROR_CHECK(
        lsm6dsox_write(
            imu_spi,
            LSM6DSOX_CTRL3_C,
            CTRL3_C_CONFIG
        )
    );


    // =========================================================================
    // CONFIGURE ACCELEROMETER
    // =========================================================================

    ESP_ERROR_CHECK(
        lsm6dsox_write(
            imu_spi,
            LSM6DSOX_CTRL1_XL,
            ACCEL_CONFIG
        )
    );


    ESP_LOGI(
        TAG,
        "Accelerometer configured: 104 Hz, +/-4g"
    );


    // =========================================================================
    // VERIFY CTRL1_XL
    // =========================================================================

    uint8_t ctrl1 = 0;

    ESP_ERROR_CHECK(
        lsm6dsox_read(
            imu_spi,
            LSM6DSOX_CTRL1_XL,
            &ctrl1,
            1
        )
    );

    printf(
        "CTRL1_XL = 0x%02X\n",
        ctrl1
    );


    // =========================================================================
    // VERIFY CTRL3_C
    // =========================================================================

    uint8_t ctrl3 = 0;

    ESP_ERROR_CHECK(
        lsm6dsox_read(
            imu_spi,
            LSM6DSOX_CTRL3_C,
            &ctrl3,
            1
        )
    );

    printf(
        "CTRL3_C  = 0x%02X\n",
        ctrl3
    );


    // =========================================================================
    // CSV HEADER
    // =========================================================================

    printf("time_ms,roll,pitch\n");


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        int16_t raw_x;
        int16_t raw_y;
        int16_t raw_z;

        esp_err_t err = lsm6dsox_read_accel(
            &raw_x,
            &raw_y,
            &raw_z
        );

        if (err != ESP_OK) {

            ESP_LOGE(
                TAG,
                "Accelerometer read failed: %s",
                esp_err_to_name(err)
            );

            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }


        // =====================================================================
        // CONVERT RAW VALUES TO g
        // =====================================================================
        //
        // For +/-4g:
        //
        // sensitivity = 0.122 mg/LSB
        //
        //                  0.122
        // acceleration = raw * ----- g
        //                  1000
        //
        // =====================================================================

        float ax = (float)raw_x * 0.000122f;
        float ay = (float)raw_y * 0.000122f;
        float az = (float)raw_z * 0.000122f;


        // =====================================================================
        // ROLL
        // =====================================================================

        float roll = atan2f(
            ay,
            az
        ) * 180.0f / PI_F;


        // =====================================================================
        // PITCH
        // =====================================================================

        float pitch = atan2f(
            -ax,
            sqrtf(
                ay * ay +
                az * az
            )
        ) * 180.0f / PI_F;


        // =====================================================================
        // CSV
        // =====================================================================

        printf(
            "%llu,%.2f,%.2f\n",
            (unsigned long long)millis(),
            roll,
            pitch
        );


        // =====================================================================
        // 20 Hz OUTPUT
        // =====================================================================

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}
