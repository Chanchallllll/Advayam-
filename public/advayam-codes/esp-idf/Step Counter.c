/*
 * =============================================================================
 * LSM6DSOX STEP COUNTER + 5x5 WS2812 MATRIX
 * ESP-IDF v5+
 * =============================================================================
 *
 * Estimates walking steps from acceleration peaks.
 *
 * Each detected step increments the counter and displays the current
 * count in binary on the 5x5 WS2812 matrix.
 *
 * Matrix:
 *     GPIO 7
 *     25 LEDs
 *
 * LSM6DSOX SPI:
 *     CS   = GPIO 10
 *     SCK  = GPIO 12
 *     MOSI = GPIO 11
 *     MISO = GPIO 13
 *
 * LSM6DSOX:
 *     Accelerometer = +/-4g
 *     ODR           = 104 Hz
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"

#include "led_strip.h"

#include "esp_log.h"
#include "esp_timer.h"


// =============================================================================
// MATRIX CONFIGURATION
// =============================================================================

#define MATRIX_PIN              7
#define NUM_LEDS                25


// =============================================================================
// LSM6DSOX SPI CONFIGURATION
// =============================================================================

#define PIN_NUM_MISO            13
#define PIN_NUM_MOSI            11
#define PIN_NUM_SCLK            12
#define PIN_NUM_CS              10


// =============================================================================
// LSM6DSOX REGISTERS
// =============================================================================

#define LSM6DSOX_WHO_AM_I       0x0F
#define LSM6DSOX_CTRL1_XL       0x10
#define LSM6DSOX_CTRL3_C        0x12
#define LSM6DSOX_OUTX_L_A       0x28

#define LSM6DSOX_WHO_AM_I_VALUE 0x6C


/*
 * CTRL1_XL:
 *
 * ODR_XL = 104 Hz
 * FS_XL  = +/-4g
 *
 * 0x48
 */
#define ACCEL_CONFIG             0x48


/*
 * CTRL3_C:
 *
 * BDU    = 1
 * IF_INC = 1
 *
 * 0x44
 */
#define CTRL3_C_CONFIG            0x44


// =============================================================================
// ACCELEROMETER CONFIGURATION
// =============================================================================

/*
 * +/-4g sensitivity:
 *
 * 0.122 mg/LSB
 */
#define ACCEL_SENSITIVITY_G       0.000122f

#define GRAVITY_MS2               9.80665f


// =============================================================================
// STEP DETECTION CONFIGURATION
// =============================================================================

/*
 * Original Arduino code:
 *
 *     if (magnitude > 13.0 &&
 *         millis() - lastStep > 300)
 *
 * Acceleration is in m/s².
 */
#define STEP_THRESHOLD             13.0f


/*
 * Minimum time between steps.
 *
 * Prevents a single walking motion from being counted
 * multiple times.
 */
#define STEP_MIN_INTERVAL_MS       300


/*
 * Original:
 *
 *     delay(20);
 */
#define LOOP_DELAY_MS               20


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "STEP_COUNTER";

static spi_device_handle_t imu_spi = NULL;

static led_strip_handle_t led_strip = NULL;

static uint32_t steps = 0;

static uint64_t last_step = 0;


// =============================================================================
// MATRIX INITIALIZATION
// =============================================================================

static void matrix_init(void)
{
    /*
     * WS2812 configuration.
     *
     * GRB pixel format matches the original NeoPixel configuration:
     *
     *     NEO_GRB + NEO_KHZ800
     */

    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,

        .max_leds = NUM_LEDS,

        .led_pixel_format = LED_PIXEL_FORMAT_GRB,

        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        },
    };


    /*
     * Use the ESP-IDF RMT backend.
     */

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
            &led_strip
        )
    );


    /*
     * Start with matrix completely off.
     */

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


// =============================================================================
// DISPLAY STEPS
// =============================================================================

static void display_steps(void)
{
    /*
     * Clear the matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    /*
     * Display the lower 25 bits of the step counter.
     *
     * This is equivalent to:
     *
     *     for (int i = 0; i < 25; i++) {
     *         if (steps & (1UL << i)) {
     *             matrix.setPixelColor(
     *                 i,
     *                 matrix.Color(0, 255, 0)
     *             );
     *         }
     *     }
     *
     * Each LED represents one binary bit.
     *
     * LED 0 = bit 0
     * LED 1 = bit 1
     * ...
     * LED 24 = bit 24
     */

    for (int i = 0; i < 25; i++) {

        /*
         * Use a 32-bit value so bit 24 is handled safely.
         */

        if (steps & (1UL << i)) {

            ESP_ERROR_CHECK(
                led_strip_set_pixel(
                    led_strip,
                    i,
                    0,
                    255,
                    0
                )
            );
        }
    }


    /*
     * Send the new LED data to the matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
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
     * LSM6DSOX SPI:
     *
     * Bit 7 = 0 -> write
     */

    tx_buf[0] = reg & 0x7F;
    tx_buf[1] = data;


    spi_transaction_t t = {
        .length = 16,

        .rxlength = 0,

        .tx_buffer = tx_buf,

        .rx_buffer = NULL,
    };


    return spi_device_transmit(
        spi,
        &t
    );
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
    if (len > 63) {
        return ESP_ERR_INVALID_SIZE;
    }


    uint8_t tx_buf[64] = {0};
    uint8_t rx_buf[64] = {0};


    /*
     * Bit 7 = 1 -> read.
     */

    tx_buf[0] = reg | 0x80;


    spi_transaction_t t = {
        .length = 8 * (1 + len),

        .rxlength = 8 * (1 + len),

        .tx_buffer = tx_buf,

        .rx_buffer = rx_buf,
    };


    esp_err_t err =
        spi_device_transmit(
            spi,
            &t
        );


    if (err != ESP_OK) {
        return err;
    }


    /*
     * First received byte corresponds to the command.
     *
     * Sensor data starts at rx_buf[1].
     */

    for (size_t i = 0; i < len; i++) {
        out_data[i] = rx_buf[i + 1];
    }


    return ESP_OK;
}


// =============================================================================
// READ ACCELEROMETER
// =============================================================================

static esp_err_t lsm6dsox_read_accel(
    int16_t *x,
    int16_t *y,
    int16_t *z)
{
    uint8_t raw[6];


    esp_err_t err =
        lsm6dsox_read(
            imu_spi,
            LSM6DSOX_OUTX_L_A,
            raw,
            6
        );


    if (err != ESP_OK) {
        return err;
    }


    /*
     * X axis.
     */

    *x = (int16_t)(
        ((uint16_t)raw[1] << 8) |
        raw[0]
    );


    /*
     * Y axis.
     */

    *y = (int16_t)(
        ((uint16_t)raw[3] << 8) |
        raw[2]
    );


    /*
     * Z axis.
     */

    *z = (int16_t)(
        ((uint16_t)raw[5] << 8) |
        raw[4]
    );


    return ESP_OK;
}


// =============================================================================
// IMU INITIALIZATION
// =============================================================================

static void imu_init(void)
{
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
         * Conservative SPI clock.
         */

        .clock_speed_hz = 1 * 1000 * 1000,

        /*
         * LSM6DSOX SPI mode 0.
         */

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


    ESP_LOGI(
        TAG,
        "LSM6DSOX WHO_AM_I = 0x%02X",
        who_am_i
    );


    if (who_am_i != LSM6DSOX_WHO_AM_I_VALUE) {

        ESP_LOGE(
            TAG,
            "LSM6DSOX not found!"
        );


        while (1) {

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    // =========================================================================
    // CTRL3_C
    // =========================================================================

    ESP_ERROR_CHECK(
        lsm6dsox_write(
            imu_spi,
            LSM6DSOX_CTRL3_C,
            CTRL3_C_CONFIG
        )
    );


    // =========================================================================
    // ACCELEROMETER
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
        "Accelerometer configured: +/-4g @ 104 Hz"
    );
}


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE MATRIX
    // =========================================================================

    matrix_init();


    // =========================================================================
    // INITIALIZE IMU
    // =========================================================================

    imu_init();


    // =========================================================================
    // INITIALIZE STEP COUNTER
    // =========================================================================

    steps = 0;

    last_step = 0;


    /*
     * Display zero.
     *
     * All 25 LEDs are off because all bits of zero are 0.
     */

    display_steps();


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "        STEP COUNTER STARTED"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Threshold: %.2f m/s^2",
        STEP_THRESHOLD
    );

    ESP_LOGI(
        TAG,
        "Minimum step interval: %d ms",
        STEP_MIN_INTERVAL_MS
    );


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        int16_t raw_x;
        int16_t raw_y;
        int16_t raw_z;


        // ---------------------------------------------------------------------
        // READ ACCELEROMETER
        // ---------------------------------------------------------------------

        esp_err_t err =
            lsm6dsox_read_accel(
                &raw_x,
                &raw_y,
                &raw_z
            );


        if (err != ESP_OK) {

            ESP_LOGW(
                TAG,
                "Accelerometer read failed: %s",
                esp_err_to_name(err)
            );


            vTaskDelay(
                pdMS_TO_TICKS(LOOP_DELAY_MS)
            );

            continue;
        }


        // ---------------------------------------------------------------------
        // CONVERT RAW DATA TO g
        // ---------------------------------------------------------------------

        float ax_g =
            (float)raw_x *
            ACCEL_SENSITIVITY_G;

        float ay_g =
            (float)raw_y *
            ACCEL_SENSITIVITY_G;

        float az_g =
            (float)raw_z *
            ACCEL_SENSITIVITY_G;


        // ---------------------------------------------------------------------
        // CONVERT g TO m/s²
        // ---------------------------------------------------------------------

        float ax =
            ax_g * GRAVITY_MS2;

        float ay =
            ay_g * GRAVITY_MS2;

        float az =
            az_g * GRAVITY_MS2;


        // ---------------------------------------------------------------------
        // CALCULATE ACCELERATION MAGNITUDE
        // ---------------------------------------------------------------------

        float magnitude =
            sqrtf(
                ax * ax +
                ay * ay +
                az * az
            );


        // ---------------------------------------------------------------------
        // CURRENT TIME
        // ---------------------------------------------------------------------

        uint64_t now =
            esp_timer_get_time() / 1000ULL;


        // ---------------------------------------------------------------------
        // STEP DETECTION
        // ---------------------------------------------------------------------

        /*
         * A step is detected when:
         *
         *     magnitude > 13 m/s²
         *
         * AND
         *
         *     at least 300 ms have passed
         *     since the previous step.
         */

        if (magnitude > STEP_THRESHOLD &&
            (now - last_step) > STEP_MIN_INTERVAL_MS)
        {
            steps++;

            last_step = now;


            // -----------------------------------------------------------------
            // SERIAL OUTPUT
            // -----------------------------------------------------------------

            ESP_LOGI(
                TAG,
                "Steps: %lu",
                (unsigned long)steps
            );


            // -----------------------------------------------------------------
            // UPDATE MATRIX
            // -----------------------------------------------------------------

            display_steps();
        }


        // ---------------------------------------------------------------------
        // SAMPLE AGAIN AFTER 20 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
