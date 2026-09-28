/*
 * =============================================================================
 * LSM6DSOX TILT-TO-SOUND
 * ESP-IDF v5+
 * =============================================================================
 *
 * Board tilt controls a buzzer:
 *
 *   Pitch -> frequency
 *   Roll  -> duty cycle / volume
 *
 * Hardware:
 *
 *   BUZZER : GPIO 16
 *
 *   LSM6DSOX:
 *       CS   : GPIO 10
 *       SCK  : GPIO 12
 *       MOSI : GPIO 11
 *       MISO : GPIO 13
 *
 * LSM6DSOX:
 *       Accelerometer = +/-4g
 *       ODR           = 104 Hz
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
#include "driver/ledc.h"

#include "esp_log.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define BUZZER_PIN      16

#define PIN_NUM_MISO    13
#define PIN_NUM_MOSI    11
#define PIN_NUM_SCLK    12
#define PIN_NUM_CS      10


// =============================================================================
// LEDC CONFIGURATION
// =============================================================================

#define BUZZER_SPEED_MODE    LEDC_LOW_SPEED_MODE
#define BUZZER_TIMER         LEDC_TIMER_0
#define BUZZER_CHANNEL       LEDC_CHANNEL_0

/*
 * 8-bit duty resolution:
 *
 * duty = 0    -> 0%
 * duty = 255  -> 100%
 *
 * This matches the Arduino code.
 */

#define BUZZER_DUTY_RES      LEDC_TIMER_8_BIT

/*
 * Initial frequency.
 *
 * This gets changed dynamically from 200 Hz to 2000 Hz.
 */

#define BUZZER_INITIAL_FREQ  1000


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
// ACCELEROMETER CONSTANTS
// =============================================================================

/*
 * +/-4g sensitivity:
 *
 * 0.122 mg/LSB
 *
 * 0.000122 g/LSB
 */

#define ACCEL_SENSITIVITY_G      0.000122f

#define GRAVITY                  9.80665f

#define PI_F                     3.14159265358979323846f


// =============================================================================
// TAG / GLOBALS
// =============================================================================

static const char *TAG = "TILT_SOUND";

static spi_device_handle_t imu_spi = NULL;


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
     * MSB = 0 -> write.
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
     * MSB = 1 -> read.
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

        out_data[i] =
            rx_buf[i + 1];
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
     * X:
     *
     * 0x28 = X low
     * 0x29 = X high
     */

    *x = (int16_t)(
        ((uint16_t)raw[1] << 8) |
        raw[0]
    );


    /*
     * Y:
     *
     * 0x2A = Y low
     * 0x2B = Y high
     */

    *y = (int16_t)(
        ((uint16_t)raw[3] << 8) |
        raw[2]
    );


    /*
     * Z:
     *
     * 0x2C = Z low
     * 0x2D = Z high
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
         * Conservative 1 MHz SPI clock.
         */

        .clock_speed_hz = 1 * 1000 * 1000,

        /*
         * SPI mode 0.
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
// BUZZER INITIALIZATION
// =============================================================================

static void buzzer_init(void)
{
    // =========================================================================
    // LEDC TIMER
    // =========================================================================

    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_SPEED_MODE,

        .duty_resolution = BUZZER_DUTY_RES,

        .timer_num = BUZZER_TIMER,

        .freq_hz = BUZZER_INITIAL_FREQ,

        .clk_cfg = LEDC_AUTO_CLK,

        .deconfigure = false,
    };


    ESP_ERROR_CHECK(
        ledc_timer_config(
            &timer_config
        )
    );


    // =========================================================================
    // LEDC CHANNEL
    // =========================================================================

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,

        .speed_mode = BUZZER_SPEED_MODE,

        .channel = BUZZER_CHANNEL,

        .intr_type = LEDC_INTR_DISABLE,

        .timer_sel = BUZZER_TIMER,

        /*
         * Start at zero duty.
         */

        .duty = 0,

        .hpoint = 0,

        .flags = {
            .output_invert = 0,
        },
    };


    ESP_ERROR_CHECK(
        ledc_channel_config(
            &channel_config
        )
    );


    ESP_LOGI(
        TAG,
        "Buzzer PWM initialized on GPIO %d",
        BUZZER_PIN
    );
}


// =============================================================================
// SET BUZZER FREQUENCY
// =============================================================================

static void buzzer_set_frequency(
    uint32_t frequency)
{
    /*
     * Change LEDC timer frequency.
     */

    ESP_ERROR_CHECK(
        ledc_set_freq(
            BUZZER_SPEED_MODE,
            BUZZER_TIMER,
            frequency
        )
    );
}


// =============================================================================
// SET BUZZER DUTY
// =============================================================================

static void buzzer_set_duty(
    uint32_t duty)
{
    /*
     * 8-bit resolution means:
     *
     * 0   -> 0%
     * 255 -> 100%
     */

    if (duty > 255) {
        duty = 255;
    }


    ESP_ERROR_CHECK(
        ledc_set_duty(
            BUZZER_SPEED_MODE,
            BUZZER_CHANNEL,
            duty
        )
    );


    ESP_ERROR_CHECK(
        ledc_update_duty(
            BUZZER_SPEED_MODE,
            BUZZER_CHANNEL
        )
    );
}


// =============================================================================
// MAP FLOAT
// =============================================================================
//
// Equivalent to Arduino's map(), but for float values.
//
// =============================================================================

static float map_float(
    float value,
    float in_min,
    float in_max,
    float out_min,
    float out_max)
{
    return
        (value - in_min) *
        (out_max - out_min) /
        (in_max - in_min) +
        out_min;
}


// =============================================================================
// CONSTRAIN FLOAT
// =============================================================================

static float constrain_float(
    float value,
    float min_value,
    float max_value)
{
    if (value < min_value) {
        return min_value;
    }


    if (value > max_value) {
        return max_value;
    }


    return value;
}


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE IMU
    // =========================================================================

    imu_init();


    // =========================================================================
    // INITIALIZE BUZZER
    // =========================================================================

    buzzer_init();


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


            /*
             * Silence buzzer on an IMU failure.
             */

            buzzer_set_duty(0);


            vTaskDelay(
                pdMS_TO_TICKS(20)
            );

            continue;
        }


        // ---------------------------------------------------------------------
        // CONVERT RAW ACCELERATION TO m/s²
        // ---------------------------------------------------------------------

        float accel_x =
            (float)raw_x *
            ACCEL_SENSITIVITY_G *
            GRAVITY;


        float accel_y =
            (float)raw_y *
            ACCEL_SENSITIVITY_G *
            GRAVITY;


        float accel_z =
            (float)raw_z *
            ACCEL_SENSITIVITY_G *
            GRAVITY;


        // ---------------------------------------------------------------------
        // CALCULATE PITCH
        // ---------------------------------------------------------------------

        /*
         * Same formula as the Arduino version:
         *
         * pitch =
         *   atan2(
         *       accel.x,
         *       sqrt(accel.y² + accel.z²)
         *   )
         *
         * Converted from radians to degrees.
         */

        float pitch =
            atan2f(
                accel_x,
                sqrtf(
                    accel_y * accel_y +
                    accel_z * accel_z
                )
            ) *
            180.0f /
            PI_F;


        // ---------------------------------------------------------------------
        // CALCULATE ROLL
        // ---------------------------------------------------------------------

        /*
         * Same formula as the Arduino version:
         *
         * roll =
         *   atan2(
         *       -accel.y,
         *       accel.z
         *   )
         */

        float roll =
            atan2f(
                -accel_y,
                accel_z
            ) *
            180.0f /
            PI_F;


        // ---------------------------------------------------------------------
        // PITCH -> FREQUENCY
        // ---------------------------------------------------------------------

        /*
         * Arduino:
         *
         * constrain(pitch, -45, 45)
         *
         * map(-45, 45, 200, 2000)
         *
         * Therefore:
         *
         *   -45 degrees -> 200 Hz
         *     0 degrees -> 1100 Hz
         *   +45 degrees -> 2000 Hz
         */

        float pitch_clamped =
            constrain_float(
                pitch,
                -45.0f,
                45.0f
            );


        float frequency_float =
            map_float(
                pitch_clamped,

                -45.0f,
                45.0f,

                200.0f,
                2000.0f
            );


        uint32_t frequency =
            (uint32_t)frequency_float;


        // ---------------------------------------------------------------------
        // ROLL -> DUTY
        // ---------------------------------------------------------------------

        /*
         * Arduino:
         *
         * fabs(roll)
         *
         * 0 degrees  -> duty 255
         * 45 degrees -> duty 40
         *
         * Greater roll angle therefore makes
         * the buzzer quieter.
         */

        float roll_abs =
            fabsf(roll);


        float roll_clamped =
            constrain_float(
                roll_abs,
                0.0f,
                45.0f
            );


        float duty_float =
            map_float(
                roll_clamped,

                0.0f,
                45.0f,

                255.0f,
                40.0f
            );


        uint32_t duty =
            (uint32_t)duty_float;


        // ---------------------------------------------------------------------
        // UPDATE BUZZER
        // ---------------------------------------------------------------------

        buzzer_set_frequency(
            frequency
        );


        buzzer_set_duty(
            duty
        );


        // ---------------------------------------------------------------------
        // OPTIONAL DEBUG
        // ---------------------------------------------------------------------

        /*
         * Uncomment if you want to monitor the calculated values.
         *
         * ESP_LOGI(
         *     TAG,
         *     "Pitch: %.1f deg  Roll: %.1f deg  "
         *     "Freq: %lu Hz  Duty: %lu",
         *     pitch,
         *     roll,
         *     (unsigned long)frequency,
         *     (unsigned long)duty
         * );
         */


        // ---------------------------------------------------------------------
        // UPDATE EVERY 20 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(20)
        );
    }
}
