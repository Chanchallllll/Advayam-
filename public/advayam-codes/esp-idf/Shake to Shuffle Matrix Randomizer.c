/*
 * =============================================================================
 * LSM6DSOX + 5x5 WS2812 SHAKE DETECTOR
 * ESP-IDF v5+
 * =============================================================================
 *
 * Detects a strong shake using the accelerometer.
 *
 * A rapid acceleration change triggers a randomized burst of colors
 * across the 5x5 RGB matrix.
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
#include <stdlib.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "led_strip.h"

#include "esp_log.h"
#include "esp_timer.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define MATRIX_PIN      7
#define NUM_LEDS        25

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
// SHAKE CONFIGURATION
// =============================================================================

/*
 * Original Arduino threshold:
 *
 *     if (change > 10.0)
 *
 * Acceleration is measured in m/s² by the Adafruit library,
 * so we preserve the same units here.
 */
#define SHAKE_THRESHOLD           10.0f


/*
 * Duration of the random color burst.
 */
#define BURST_DURATION_MS         150


/*
 * Main accelerometer sampling interval.
 *
 * Same as:
 *
 *     delay(20);
 */
#define LOOP_DELAY_MS             20


// =============================================================================
// CONSTANTS
// =============================================================================

#define GRAVITY_MS2               9.80665f

static const char *TAG = "SHAKE";

static spi_device_handle_t imu_spi = NULL;

static led_strip_handle_t led_strip = NULL;


// =============================================================================
// MILLIS
// =============================================================================

static uint64_t millis(void)
{
    return esp_timer_get_time() / 1000ULL;
}


// =============================================================================
// MATRIX INITIALIZATION
// =============================================================================

static void matrix_init(void)
{
    /*
     * WS2812 configuration.
     *
     * This is the same configuration requested previously.
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
     * RMT backend.
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
     * Start with all LEDs off.
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
     * Actual data starts at rx_buf[1].
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
         * 1 MHz is deliberately conservative.
         *
         * You can increase this later if desired.
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
        "Accelerometer: +/-4g @ 104 Hz"
    );
}


// =============================================================================
// RANDOM COLOR BURST
// =============================================================================

static void random_color_burst(void)
{
    /*
     * Fill all 25 LEDs with random RGB values.
     *
     * Equivalent to:
     *
     * for (int i = 0; i < 25; i++) {
     *     matrix.setPixelColor(
     *         i,
     *         matrix.Color(
     *             random(256),
     *             random(256),
     *             random(256)
     *         )
     *     );
     * }
     */

    for (int i = 0; i < NUM_LEDS; i++) {

        uint8_t red =
            (uint8_t)(rand() % 256);

        uint8_t green =
            (uint8_t)(rand() % 256);

        uint8_t blue =
            (uint8_t)(rand() % 256);


        matrix_set_pixel(
            i,
            red,
            green,
            blue
        );
    }


    /*
     * Display the entire burst.
     */

    matrix_show();


    /*
     * Keep the colors visible for 150 ms.
     */

    vTaskDelay(
        pdMS_TO_TICKS(BURST_DURATION_MS)
    );


    /*
     * Turn the matrix off.
     */

    matrix_clear();

    matrix_show();
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


    gpio_set_level(
        GPIO_CONTROL,
        0
    );


    // =========================================================================
    // MATRIX
    // =========================================================================

    matrix_init();


    // =========================================================================
    // IMU
    // =========================================================================

    imu_init();


    // =========================================================================
    // RANDOM SEED
    // =========================================================================

    /*
     * Arduino version:
     *
     *     randomSeed(analogRead(1));
     *
     * ESP-IDF doesn't need an analog input for this.
     */

    srand(
        (unsigned int)esp_timer_get_time()
    );


    // =========================================================================
    // INITIAL STATE
    // =========================================================================

    /*
     * Same as Arduino:
     *
     * float previousMagnitude = 1.0;
     *
     * Since acceleration is expressed in m/s²,
     * this is 1 m/s² rather than 1g.
     */

    float previous_magnitude = 1.0f;


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       SHAKE DETECTOR STARTED"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Shake threshold: %.1f m/s^2",
        SHAKE_THRESHOLD
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
                "IMU read failed: %s",
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
        //
        // +/-4g:
        //
        // 0.122 mg/LSB
        //
        // ---------------------------------------------------------------------

        float ax =
            (float)raw_x * 0.000122f;

        float ay =
            (float)raw_y * 0.000122f;

        float az =
            (float)raw_z * 0.000122f;


        // ---------------------------------------------------------------------
        // CONVERT TO m/s²
        // ---------------------------------------------------------------------

        float accel_x =
            ax * GRAVITY_MS2;

        float accel_y =
            ay * GRAVITY_MS2;

        float accel_z =
            az * GRAVITY_MS2;


        // ---------------------------------------------------------------------
        // ACCELERATION MAGNITUDE
        // ---------------------------------------------------------------------

        /*
         * Same calculation as the Arduino code:
         *
         * sqrt(
         *     x*x +
         *     y*y +
         *     z*z
         * )
         */

        float magnitude =
            sqrtf(
                accel_x * accel_x +
                accel_y * accel_y +
                accel_z * accel_z
            );


        // ---------------------------------------------------------------------
        // CHANGE IN MAGNITUDE
        // ---------------------------------------------------------------------

        float change =
            fabsf(
                magnitude -
                previous_magnitude
            );


        // ---------------------------------------------------------------------
        // SHAKE DETECTION
        // ---------------------------------------------------------------------

        if (change > SHAKE_THRESHOLD) {

            ESP_LOGI(
                TAG,
                "SHAKE! Magnitude: %.2f -> %.2f, change: %.2f",
                previous_magnitude,
                magnitude,
                change
            );


            // -----------------------------------------------------------------
            // RANDOM RGB BURST
            // -----------------------------------------------------------------

            random_color_burst();
        }


        // ---------------------------------------------------------------------
        // SAVE CURRENT MAGNITUDE
        // ---------------------------------------------------------------------

        previous_magnitude =
            magnitude;


        // ---------------------------------------------------------------------
        // SAMPLE AGAIN AFTER 20 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
