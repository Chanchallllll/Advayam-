/*
 * =============================================================================
 * LSM6DSOX 6-DIRECTION ORIENTATION DETECTOR
 * ESP-IDF v5+
 * =============================================================================
 *
 * Determines which of six directions the board is facing:
 *
 *     UP
 *     DOWN
 *     LEFT
 *     RIGHT
 *     FORWARD
 *     BACKWARD
 *
 * The center LED of the 5x5 WS2812 matrix displays the orientation:
 *
 *     UP / DOWN      = Yellow
 *     LEFT / RIGHT   = Green
 *     FORWARD / BACK = Blue
 *
 * Pin Configuration:
 *
 *   WS2812 DATA : GPIO 7
 *
 *   LSM6DSOX:
 *       CS   : GPIO 10
 *       SCK  : GPIO 12
 *       MOSI : GPIO 11
 *       MISO : GPIO 13
 *
 *   GPIO 16 : LOW
 *
 * LSM6DSOX:
 *       Accelerometer = +/-4g
 *       ODR           = 104 Hz
 *
 * Matrix:
 *       5 x 5 = 25 LEDs
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
#include "driver/gpio.h"

#include "led_strip.h"

#include "esp_log.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define MATRIX_PIN       7
#define NUM_LEDS         25

#define PIN_NUM_MISO     13
#define PIN_NUM_MOSI     11
#define PIN_NUM_SCLK     12
#define PIN_NUM_CS       10

#define GPIO_CONTROL     16


// =============================================================================
// MATRIX CONFIGURATION
// =============================================================================

/*
 * Center LED of the 5x5 matrix.
 *
 * Index:
 *
 *     0   1   2   3   4
 *     5   6   7   8   9
 *    10  11  12  13  14
 *    15  16  17  18  19
 *    20  21  22  23  24
 *
 * Center = 12
 */

#define CENTER_LED       12


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
#define CTRL3_C_CONFIG           0x44


// =============================================================================
// ACCELEROMETER
// =============================================================================

/*
 * LSM6DSOX +/-4g sensitivity:
 *
 * 0.122 mg/LSB
 *
 * Therefore:
 *
 *     0.000122 g / LSB
 */
#define ACCEL_SENSITIVITY_G      0.000122f

#define GRAVITY_MS2              9.80665f


// =============================================================================
// LOOP CONFIGURATION
// =============================================================================

/*
 * Same as Arduino:
 *
 *     delay(250);
 */
#define LOOP_DELAY_MS            250


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "ORIENTATION";

static spi_device_handle_t imu_spi = NULL;

static led_strip_handle_t led_strip = NULL;


// =============================================================================
// MATRIX INITIALIZATION
// =============================================================================

static void matrix_init(void)
{
    /*
     * Configure WS2812 LED strip.
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
     * Use ESP-IDF RMT peripheral to drive WS2812.
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
     * Initially turn all LEDs off.
     */

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


// =============================================================================
// MATRIX CLEAR
// =============================================================================

static void matrix_clear(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


// =============================================================================
// MATRIX SET PIXEL
// =============================================================================

static void matrix_set_pixel(
    int index,
    uint8_t red,
    uint8_t green,
    uint8_t blue)
{
    if (index < 0 || index >= NUM_LEDS) {
        return;
    }


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            index,
            red,
            green,
            blue
        )
    );
}


// =============================================================================
// MATRIX SHOW
// =============================================================================

static void matrix_show(void)
{
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
     * Bit 7 = 0 -> write.
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
     * First received byte corresponds to the command byte.
     *
     * Actual register data begins at rx_buf[1].
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
         * LSM6DSOX uses SPI mode 0.
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
    // CHECK WHO_AM_I
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
        "Accelerometer configured: +/-4g @ 104 Hz"
    );
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


    ESP_ERROR_CHECK(
        gpio_config(
            &gpio_cfg
        )
    );


    /*
     * Keep GPIO 16 LOW.
     */

    ESP_ERROR_CHECK(
        gpio_set_level(
            GPIO_CONTROL,
            0
        )
    );


    // =========================================================================
    // INITIALIZE MATRIX
    // =========================================================================

    matrix_init();


    // =========================================================================
    // INITIALIZE IMU
    // =========================================================================

    imu_init();


    // =========================================================================
    // START
    // =========================================================================

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "    6-DIRECTION ORIENTATION DETECTOR"
    );

    ESP_LOGI(
        TAG,
        "========================================"
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

            ESP_LOGE(
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
        // CONVERT RAW VALUES TO g
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
        // CONVERT TO m/s²
        // ---------------------------------------------------------------------

        float ax =
            ax_g * GRAVITY_MS2;

        float ay =
            ay_g * GRAVITY_MS2;

        float az =
            az_g * GRAVITY_MS2;


        // ---------------------------------------------------------------------
        // ABSOLUTE AXIS VALUES
        // ---------------------------------------------------------------------

        float abs_x =
            fabsf(ax);

        float abs_y =
            fabsf(ay);

        float abs_z =
            fabsf(az);


        // ---------------------------------------------------------------------
        // CLEAR MATRIX
        // ---------------------------------------------------------------------

        matrix_clear();


        // ---------------------------------------------------------------------
        // DETERMINE ORIENTATION
        // ---------------------------------------------------------------------

        const char *orientation;


        /*
         * Z axis is dominant.
         *
         * Board is either:
         *
         *     UP
         *     DOWN
         */

        if (abs_z >= abs_x &&
            abs_z >= abs_y)
        {
            if (az > 0) {

                orientation = "UP";

            } else {

                orientation = "DOWN";
            }


            /*
             * UP / DOWN = Yellow.
             */

            matrix_set_pixel(
                CENTER_LED,
                255,
                255,
                0
            );
        }


        /*
         * Y axis is dominant.
         *
         * Board is either:
         *
         *     RIGHT
         *     LEFT
         */

        else if (abs_y >= abs_x)
        {
            if (ay > 0) {

                orientation = "RIGHT";

            } else {

                orientation = "LEFT";
            }


            /*
             * LEFT / RIGHT = Green.
             */

            matrix_set_pixel(
                CENTER_LED,
                0,
                255,
                0
            );
        }


        /*
         * X axis is dominant.
         *
         * Board is either:
         *
         *     FORWARD
         *     BACKWARD
         */

        else
        {
            if (ax > 0) {

                orientation = "FORWARD";

            } else {

                orientation = "BACKWARD";
            }


            /*
             * FORWARD / BACKWARD = Blue.
             */

            matrix_set_pixel(
                CENTER_LED,
                0,
                0,
                255
            );
        }


        // ---------------------------------------------------------------------
        // DISPLAY
        // ---------------------------------------------------------------------

        matrix_show();


        // ---------------------------------------------------------------------
        // SERIAL OUTPUT
        // ---------------------------------------------------------------------

        ESP_LOGI(
            TAG,
            "%s | ax=%.2f ay=%.2f az=%.2f",
            orientation,
            ax,
            ay,
            az
        );


        // ---------------------------------------------------------------------
        // WAIT
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
