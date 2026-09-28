/*
 * =============================================================================
 * TOUCHLESS PROXIMITY SWITCH
 * ESP-IDF v5+
 * =============================================================================
 *
 * Wave a hand near the RPR-0521RS to toggle the onboard User LED.
 *
 * The RPR-0521RS does not directly report distance in centimeters, so
 * PROXIMITY_THRESHOLD must be calibrated experimentally.
 *
 * Hardware:
 *
 *   RPR-0521RS:
 *       SDA : GPIO 5
 *       SCL : GPIO 6
 *
 *   User LED:
 *       GPIO 45
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "driver/gpio.h"

#include "esp_log.h"

#include "rpr0521rs.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define I2C_MASTER_SDA_IO       5
#define I2C_MASTER_SCL_IO       6

#define I2C_MASTER_FREQ_HZ      100000

#define USER_LED                GPIO_NUM_45


// =============================================================================
// PROXIMITY CONFIGURATION
// =============================================================================

/*
 * Increase/decrease this value after testing your sensor.
 */

#define PROXIMITY_THRESHOLD     500


/*
 * Sensor polling interval.
 */

#define SENSOR_POLL_MS          50


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "PROX_SWITCH";

static i2c_master_dev_handle_t sensor_dev = NULL;


/*
 * Current LED state.
 */

static bool led_state = false;


/*
 * Previous hand detection state.
 *
 * This is what prevents the LED from toggling repeatedly while
 * the hand remains in front of the sensor.
 */

static bool hand_was_near = false;


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

        /*
         * Use the address supplied by the RPR-0521RS driver.
         */

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
// USER LED INITIALIZATION
// =============================================================================

static void user_led_init(void)
{
    gpio_config_t led_config = {
        .pin_bit_mask =
            (1ULL << USER_LED),

        .mode =
            GPIO_MODE_OUTPUT,

        .pull_up_en =
            GPIO_PULLUP_DISABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(
            &led_config
        )
    );


    /*
     * Start with LED OFF.
     */

    led_state = false;


    ESP_ERROR_CHECK(
        gpio_set_level(
            USER_LED,
            0
        )
    );
}


// =============================================================================
// SENSOR INITIALIZATION
// =============================================================================

static void proximity_sensor_init(void)
{
    /*
     * Use the same RPR-0521RS driver initialization as your
     * working reaction-game program.
     */

    esp_err_t err =
        rpr0521rs_init(
            sensor_dev
        );


    if (err != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Sensor initialization failed: %s",
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
// SET USER LED
// =============================================================================

static void user_led_set(bool state)
{
    led_state = state;


    ESP_ERROR_CHECK(
        gpio_set_level(
            USER_LED,
            state ? 1 : 0
        )
    );
}


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE USER LED
    // =========================================================================

    user_led_init();


    // =========================================================================
    // INITIALIZE I2C
    // =========================================================================

    i2c_master_init();


    // =========================================================================
    // INITIALIZE PROXIMITY SENSOR
    // =========================================================================

    proximity_sensor_init();


    // =========================================================================
    // READY
    // =========================================================================

    ESP_LOGI(
        TAG,
        "Touchless Proximity Switch started."
    );


    printf(
        "Touchless Proximity Switch started.\n"
    );


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        uint16_t proximity = 0;

        float lux = 0.0f;


        // ---------------------------------------------------------------------
        // READ PROXIMITY SENSOR
        // ---------------------------------------------------------------------

        esp_err_t err =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (err == ESP_OK) {

            // -----------------------------------------------------------------
            // DETERMINE WHETHER HAND IS NEAR
            // -----------------------------------------------------------------

            bool hand_is_near =
                proximity >
                PROXIMITY_THRESHOLD;


            // -----------------------------------------------------------------
            // HAND ENTERED DETECTION AREA
            // -----------------------------------------------------------------

            /*
             * Toggle only on the transition:
             *
             *     FAR -> NEAR
             *
             * If the hand stays near the sensor, this condition is false
             * on subsequent iterations.
             */

            if (hand_is_near &&
                !hand_was_near) {


                // -------------------------------------------------------------
                // TOGGLE LED
                // -------------------------------------------------------------

                user_led_set(
                    !led_state
                );


                // -------------------------------------------------------------
                // UART MESSAGE
                // -------------------------------------------------------------

                printf(
                    "LED: %s\n",
                    led_state ?
                    "ON" :
                    "OFF"
                );


                fflush(stdout);


                ESP_LOGI(
                    TAG,
                    "Hand detected: proximity=%u, LED=%s",
                    proximity,
                    led_state ?
                    "ON" :
                    "OFF"
                );
            }


            // -----------------------------------------------------------------
            // SAVE CURRENT HAND STATE
            // -----------------------------------------------------------------

            hand_was_near =
                hand_is_near;
        }


        else {

            ESP_LOGW(
                TAG,
                "Proximity sensor read failed: %s",
                esp_err_to_name(err)
            );
        }


        // ---------------------------------------------------------------------
        // POLL EVERY 50 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_POLL_MS
            )
        );
    }
}
