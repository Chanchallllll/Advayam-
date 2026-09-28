/*
 * =============================================================================
 * TOUCHLESS VOLUME KNOB
 * ESP-IDF v5+
 * =============================================================================
 *
 * Calibrates the current hand position as the "center" position.
 *
 * Move hand closer:
 *   Increase matrix brightness
 *
 * Move hand farther:
 *   Decrease matrix brightness
 *
 * Hold your hand in position for CALIBRATION_TIME to establish the center.
 *
 * Hardware:
 *
 *   RPR-0521RS:
 *       SDA : GPIO 5
 *       SCL : GPIO 6
 *
 *   WS2812 Matrix:
 *       DATA : GPIO 7
 *       LEDs : 25
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "led_strip.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "rpr0521rs.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define I2C_MASTER_SDA_IO       5
#define I2C_MASTER_SCL_IO       6

#define I2C_MASTER_FREQ_HZ      100000

#define MATRIX_PIN              7
#define NUM_PIXELS              25


// =============================================================================
// CALIBRATION CONFIGURATION
// =============================================================================

/*
 * Time the hand must remain steady before calibration completes.
 */

#define CALIBRATION_TIME_MS     2000


/*
 * Hand must be above this proximity level before calibration begins.
 */

#define CALIBRATION_THRESHOLD   200


// =============================================================================
// BRIGHTNESS CONFIGURATION
// =============================================================================

/*
 * Same range as the Arduino program.
 */

#define MIN_BRIGHTNESS          10
#define MAX_BRIGHTNESS          255


/*
 * Difference range used for mapping proximity to brightness.
 */

#define DIFFERENCE_MIN          -800
#define DIFFERENCE_MAX          800


// =============================================================================
// SENSOR POLLING
// =============================================================================

#define SENSOR_POLL_MS          50


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "VOLUME_KNOB";


static i2c_master_dev_handle_t sensor_dev = NULL;


static led_strip_handle_t led_strip = NULL;


/*
 * Calibration state.
 */

static bool calibrated = false;


/*
 * Proximity value recorded as the center position.
 */

static uint16_t center_proximity = 0;


/*
 * Time at which calibration started.
 *
 * Zero means calibration is not currently in progress.
 */

static uint64_t calibration_start_ms = 0;


// =============================================================================
// MILLIS
// =============================================================================

static uint64_t millis(void)
{
    return esp_timer_get_time() / 1000ULL;
}


// =============================================================================
// FLOAT MAP
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
// INTEGER CONSTRAIN
// =============================================================================

static int constrain_int(
    int value,
    int min_value,
    int max_value)
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
// I2C INITIALIZATION
// =============================================================================

static void i2c_master_init(void)
{
    // =========================================================================
    // I2C BUS
    // =========================================================================

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


    i2c_master_bus_handle_t bus_handle = NULL;


    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );


    // =========================================================================
    // RPR-0521RS DEVICE
    // =========================================================================

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


    ESP_LOGI(
        TAG,
        "I2C initialized: SDA=%d SCL=%d",
        I2C_MASTER_SDA_IO,
        I2C_MASTER_SCL_IO
    );
}


// =============================================================================
// RPR-0521RS INITIALIZATION
// =============================================================================

static void proximity_sensor_init(void)
{
    /*
     * Use the same native ESP-IDF RPR-0521RS driver as the
     * working reaction-game program.
     */

    esp_err_t err =
        rpr0521rs_init(
            sensor_dev
        );


    if (err != ESP_OK) {

        ESP_LOGE(
            TAG,
            "RPR-0521RS initialization failed: %s",
            esp_err_to_name(err)
        );


        while (1) {

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    ESP_LOGI(
        TAG,
        "RPR-0521RS initialized successfully"
    );
}


// =============================================================================
// MATRIX INITIALIZATION
// =============================================================================

static void matrix_init(void)
{
    /*
     * WS2812 uses GRB color ordering.
     */

    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,

        .max_leds = NUM_PIXELS,

        .led_pixel_format =
            LED_PIXEL_FORMAT_GRB,

        .led_model =
            LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        },
    };


    /*
     * RMT backend.
     *
     * 10 MHz = 100 ns per RMT tick.
     */

    led_strip_rmt_config_t rmt_config = {
        .clk_src =
            RMT_CLK_SRC_DEFAULT,

        .resolution_hz =
            10 * 1000 * 1000,

        .mem_block_symbols =
            64,

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


    ESP_ERROR_CHECK(
        led_strip_clear(
            led_strip
        )
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(
            led_strip
        )
    );


    ESP_LOGI(
        TAG,
        "5x5 WS2812 matrix initialized"
    );
}


// =============================================================================
// MATRIX SHOW WHITE
// =============================================================================
//
// The Arduino version uses:
//
//     matrix.setBrightness(brightness);
//
// followed by:
//
//     matrix.setPixelColor(i, 255, 255, 255);
//
//
//
// Here we apply the brightness scaling directly to the RGB values.
//
// =============================================================================

static void matrix_show_white(
    uint8_t brightness)
{
    /*
     * White = 255,255,255.
     *
     * Scale all three channels by brightness.
     */

    uint8_t value =
        brightness;


    for (int i = 0; i < NUM_PIXELS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                value,
                value,
                value
            )
        );
    }


    ESP_ERROR_CHECK(
        led_strip_refresh(
            led_strip
        )
    );
}


// =============================================================================
// MATRIX CLEAR
// =============================================================================

static void matrix_clear(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(
            led_strip
        )
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(
            led_strip
        )
    );
}


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE I2C
    // =========================================================================

    i2c_master_init();


    // =========================================================================
    // INITIALIZE PROXIMITY SENSOR
    // =========================================================================

    proximity_sensor_init();


    // =========================================================================
    // INITIALIZE LED MATRIX
    // =========================================================================

    matrix_init();


    // =========================================================================
    // READY
    // =========================================================================

    ESP_LOGI(
        TAG,
        "Hold hand steady to calibrate..."
    );


    printf(
        "Hold hand steady to calibrate...\n"
    );


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        uint16_t proximity = 0;

        float lux = 0.0f;


        // ---------------------------------------------------------------------
        // READ RPR-0521RS
        // ---------------------------------------------------------------------

        esp_err_t err =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (err != ESP_OK) {

            ESP_LOGW(
                TAG,
                "Sensor read failed: %s",
                esp_err_to_name(err)
            );


            vTaskDelay(
                pdMS_TO_TICKS(
                    SENSOR_POLL_MS
                )
            );


            continue;
        }


        // =========================================================================
        // CALIBRATION
        // =========================================================================

        if (!calibrated)
        {
            // -----------------------------------------------------------------
            // HAND IS IN CALIBRATION RANGE
            // -----------------------------------------------------------------

            if (proximity >
                CALIBRATION_THRESHOLD)
            {

                /*
                 * Start the calibration timer when the hand first
                 * enters the valid calibration range.
                 */

                if (calibration_start_ms == 0) {

                    calibration_start_ms =
                        millis();


                    ESP_LOGI(
                        TAG,
                        "Calibration started..."
                    );
                }


                // -------------------------------------------------------------
                // CHECK CALIBRATION TIME
                // -------------------------------------------------------------

                uint64_t elapsed =
                    millis() -
                    calibration_start_ms;


                if (elapsed >=
                    CALIBRATION_TIME_MS)
                {

                    /*
                     * Record the current proximity as the center position.
                     */

                    center_proximity =
                        proximity;


                    calibrated = true;


                    ESP_LOGI(
                        TAG,
                        "Calibration complete. Center = %u",
                        center_proximity
                    );


                    printf(
                        "Calibration complete. Center = %u\n",
                        center_proximity
                    );
                }
            }


            // -----------------------------------------------------------------
            // HAND LEFT CALIBRATION RANGE
            // -----------------------------------------------------------------

            else {

                /*
                 * Reset the timer.
                 *
                 * The user must keep the hand continuously above
                 * CALIBRATION_THRESHOLD for the full calibration period.
                 */

                calibration_start_ms =
                    0;
            }


            vTaskDelay(
                pdMS_TO_TICKS(
                    SENSOR_POLL_MS
                )
            );


            continue;
        }


        // =========================================================================
        // CALIBRATED OPERATION
        // =========================================================================

        /*
         * Calculate the difference between the current hand position
         * and the calibrated center position.
         *
         * Positive:
         *     Hand closer than center.
         *
         * Negative:
         *     Hand farther than center.
         */

        int difference =
            (int)proximity -
            (int)center_proximity;


        // ---------------------------------------------------------------------
        // LIMIT DIFFERENCE
        // ---------------------------------------------------------------------

        int limited_difference =
            constrain_int(
                difference,
                DIFFERENCE_MIN,
                DIFFERENCE_MAX
            );


        // ---------------------------------------------------------------------
        // MAP DIFFERENCE TO BRIGHTNESS
        // ---------------------------------------------------------------------

        /*
         * -800 -> brightness 10
         *    0 -> approximately 132
         * +800 -> brightness 255
         */

        float brightness_float =
            map_float(
                (float)limited_difference,

                DIFFERENCE_MIN,
                DIFFERENCE_MAX,

                MIN_BRIGHTNESS,
                MAX_BRIGHTNESS
            );


        int brightness =
            (int)brightness_float;


        brightness =
            constrain_int(
                brightness,
                MIN_BRIGHTNESS,
                MAX_BRIGHTNESS
            );


        // ---------------------------------------------------------------------
        // DISPLAY WHITE
        // ---------------------------------------------------------------------

        matrix_show_white(
            (uint8_t)brightness
        );


        // ---------------------------------------------------------------------
        // DEBUG OUTPUT
        // ---------------------------------------------------------------------

        printf(
            "Proximity: %u | Center: %u | Difference: %d | Brightness: %d\n",
            proximity,
            center_proximity,
            difference,
            brightness
        );


        // ---------------------------------------------------------------------
        // NEXT SAMPLE
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_POLL_MS
            )
        );
    }
}
