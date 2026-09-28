/*
 * =============================================================================
 * PRECISION DIGITAL SPIRIT LEVEL
 * =============================================================================
 *
 * Native ESP-IDF 6.0.2 version.
 *
 * Green  = level (< 2 degrees)
 * Yellow = small tilt (< 8 degrees)
 * Red    = large tilt (>= 8 degrees)
 *
 * Hardware:
 *
 * RGB Matrix:
 *   DATA = GPIO 7
 *
 * LSM6DSOX:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * Buzzer / spare output:
 *   GPIO 16
 *
 * ESP32-S3
 * ESP-IDF 6.0.2
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "led_strip.h"
#include "led_strip_rmt.h"

#include "esp_err.h"
#include "esp_log.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN      7
#define NUM_LEDS        25

#define IMU_CS          10
#define IMU_SCK         12
#define IMU_MOSI        11
#define IMU_MISO        13

#define BUZZER_PIN      16


//=============================================================================
// Matrix brightness
//=============================================================================

#define MATRIX_BRIGHTNESS 25


//=============================================================================
// LSM6DSOX Registers
//=============================================================================

#define LSM6DSOX_WHO_AM_I        0x0F
#define LSM6DSOX_CTRL1_XL        0x10

#define LSM6DSOX_OUTX_L_A        0x28
#define LSM6DSOX_OUTX_H_A        0x29
#define LSM6DSOX_OUTY_L_A        0x2A
#define LSM6DSOX_OUTY_H_A        0x2B
#define LSM6DSOX_OUTZ_L_A        0x2C
#define LSM6DSOX_OUTZ_H_A        0x2D

#define LSM6DSOX_WHO_AM_I_VALUE  0x6C


//=============================================================================
// LSM6DSOX configuration
//=============================================================================
//
// CTRL1_XL:
//
// ODR = 104 Hz
// FS  = +/-4 g
//
// 0100 = 104 Hz
// 10   = +/-4 g
//
// 0x48
//
//=============================================================================

#define LSM6DSOX_ACCEL_CONFIG    0x48


//=============================================================================
// SPI
//=============================================================================

static spi_device_handle_t imu_spi;


//=============================================================================
// LED strip
//=============================================================================

static led_strip_handle_t matrix;

static const char *TAG = "SPIRIT_LEVEL";


//=============================================================================
// Matrix Coordinate Mapping
//=============================================================================

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


//=============================================================================
// Set Matrix Pixel
//=============================================================================

static void set_pixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }

    /*
     * Apply brightness.
     *
     * Arduino version:
     *
     *   matrix.setBrightness(25)
     */

    red =
        ((uint16_t)red * MATRIX_BRIGHTNESS) / 255;

    green =
        ((uint16_t)green * MATRIX_BRIGHTNESS) / 255;

    blue =
        ((uint16_t)blue * MATRIX_BRIGHTNESS) / 255;

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            pixel_xy(x, y),
            red,
            green,
            blue
        )
    );
}


//=============================================================================
// Initialize Matrix
//=============================================================================

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//=============================================================================
// SPI Register Write
//=============================================================================

static esp_err_t imu_write_reg(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t data[2];

    /*
     * LSM6DSOX:
     * Bit 7 = 0 -> write
     */

    data[0] = reg & 0x7F;
    data[1] = value;

    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = data,
    };

    return spi_device_transmit(
        imu_spi,
        &transaction
    );
}


//=============================================================================
// SPI Register Read
//=============================================================================

static esp_err_t imu_read_reg(
    uint8_t reg,
    uint8_t *value
)
{
    uint8_t tx[2] = {
        (uint8_t)(reg | 0x80),
        0x00
    };

    uint8_t rx[2] = {
        0,
        0
    };

    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t err =
        spi_device_transmit(
            imu_spi,
            &transaction
        );

    if (err != ESP_OK) {
        return err;
    }

    *value = rx[1];

    return ESP_OK;
}


//=============================================================================
// Read 16-bit Accelerometer Value
//=============================================================================

static int16_t imu_read16(uint8_t register_address)
{
    uint8_t low;
    uint8_t high;

    ESP_ERROR_CHECK(
        imu_read_reg(
            register_address,
            &low
        )
    );

    ESP_ERROR_CHECK(
        imu_read_reg(
            register_address + 1,
            &high
        )
    );

    return (int16_t)(
        ((uint16_t)high << 8) |
        low
    );
}


//=============================================================================
// Initialize IMU
//=============================================================================

static void imu_init(void)
{
    spi_bus_config_t bus_config = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST,
            &bus_config,
            SPI_DMA_CH_AUTO
        )
    );

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 3,
        .spics_io_num = IMU_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(
        spi_bus_add_device(
            SPI2_HOST,
            &device_config,
            &imu_spi
        )
    );

    //------------------------------------------------------------------------
    // Check WHO_AM_I
    //------------------------------------------------------------------------

    uint8_t who_am_i = 0;

    ESP_ERROR_CHECK(
        imu_read_reg(
            LSM6DSOX_WHO_AM_I,
            &who_am_i
        )
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX WHO_AM_I = 0x%02X",
        who_am_i
    );

    if (who_am_i != LSM6DSOX_WHO_AM_I_VALUE) {

        ESP_LOGE(
            TAG,
            "LSM6DSOX not detected!"
        );

        ESP_LOGE(
            TAG,
            "Expected 0x%02X, got 0x%02X",
            LSM6DSOX_WHO_AM_I_VALUE,
            who_am_i
        );

        while (1) {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    //------------------------------------------------------------------------
    // Configure accelerometer
    //------------------------------------------------------------------------

    ESP_ERROR_CHECK(
        imu_write_reg(
            LSM6DSOX_CTRL1_XL,
            LSM6DSOX_ACCEL_CONFIG
        )
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX configured: 104 Hz, +/-4g"
    );
}


//=============================================================================
// Convert Raw Accelerometer Value To G
//=============================================================================
//
// For +/-4g:
//
// Sensitivity = 0.122 mg/LSB
//
// Therefore:
//
// g = raw * 0.000122
//
//=============================================================================

static float raw_to_g(int16_t raw)
{
    return (float)raw * 0.000122f;
}


//=============================================================================
// Draw Spirit Level
//=============================================================================

static void draw_level(
    float roll,
    float pitch
)
{
    float error = fabsf(roll);

    if (fabsf(pitch) > error) {
        error = fabsf(pitch);
    }

    uint8_t red;
    uint8_t green;
    uint8_t blue;

    //---------------------------------------------------------------------
    // Level
    //---------------------------------------------------------------------

    if (error < 2.0f) {

        red = 0;
        green = 255;
        blue = 0;
    }

    //---------------------------------------------------------------------
    // Small error
    //---------------------------------------------------------------------

    else if (error < 8.0f) {

        red = 255;
        green = 150;
        blue = 0;
    }

    //---------------------------------------------------------------------
    // Large tilt
    //---------------------------------------------------------------------

    else {

        red = 255;
        green = 0;
        blue = 0;
    }

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    /*
     * Center LED.
     *
     * Physical index 12 is the center of the 5x5 matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            12,
            ((uint16_t)red * MATRIX_BRIGHTNESS) / 255,
            ((uint16_t)green * MATRIX_BRIGHTNESS) / 255,
            ((uint16_t)blue * MATRIX_BRIGHTNESS) / 255
        )
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//=============================================================================
// Main
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting precision digital spirit level"
    );

    //------------------------------------------------------------------------
    // GPIO 16
    //------------------------------------------------------------------------

    gpio_config_t buzzer_config = {
        .pin_bit_mask = 1ULL << BUZZER_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&buzzer_config)
    );

    gpio_set_level(
        BUZZER_PIN,
        0
    );

    //------------------------------------------------------------------------
    // Matrix
    //------------------------------------------------------------------------

    matrix_init();

    //------------------------------------------------------------------------
    // IMU
    //------------------------------------------------------------------------

    imu_init();

    ESP_LOGI(
        TAG,
        "Spirit level running..."
    );

    //------------------------------------------------------------------------
    // Main loop
    //------------------------------------------------------------------------

    while (1) {

        //---------------------------------------------------------------------
        // Read accelerometer
        //---------------------------------------------------------------------

        int16_t raw_x =
            imu_read16(LSM6DSOX_OUTX_L_A);

        int16_t raw_y =
            imu_read16(LSM6DSOX_OUTY_L_A);

        int16_t raw_z =
            imu_read16(LSM6DSOX_OUTZ_L_A);

        //---------------------------------------------------------------------
        // Convert to G
        //---------------------------------------------------------------------

        float ax = raw_to_g(raw_x);
        float ay = raw_to_g(raw_y);
        float az = raw_to_g(raw_z);

        //---------------------------------------------------------------------
        // Calculate roll
        //---------------------------------------------------------------------

        float roll =
            atan2f(
                -ay,
                az
            )
            * 180.0f
            / M_PI;

        //---------------------------------------------------------------------
        // Calculate pitch
        //---------------------------------------------------------------------

        float pitch =
            atan2f(
                ax,
                sqrtf(
                    ay * ay +
                    az * az
                )
            )
            * 180.0f
            / M_PI;

        //---------------------------------------------------------------------
        // Draw
        //---------------------------------------------------------------------

        draw_level(
            roll,
            pitch
        );

        //---------------------------------------------------------------------
        // Serial debugging
        //---------------------------------------------------------------------

        ESP_LOGI(
            TAG,
            "Roll: %6.2f deg | Pitch: %6.2f deg | "
            "Accel: X=%+.2f Y=%+.2f Z=%+.2f g",
            roll,
            pitch,
            ax,
            ay,
            az
        );

        //---------------------------------------------------------------------
        // ~33 Hz update rate
        //---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(30)
        );
    }
}
