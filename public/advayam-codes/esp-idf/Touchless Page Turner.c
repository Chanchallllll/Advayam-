/*
 * =============================================================================
 * TOUCHLESS PAGE TURNER
 * ESP-IDF v5+
 * =============================================================================
 *
 * Generates UART commands for controlling presentation slides.
 *
 * IMPORTANT:
 *
 * The single RPR-0521RS sensor cannot determine actual left/right movement.
 * This implementation interprets rapid proximity changes as gestures:
 *
 *   Rapid approach -> NEXT
 *   Rapid retreat  -> PREV
 *
 * UART commands:
 *
 *   NEXT
 *   PREV
 *
 * A PC-side program can translate these commands into keyboard actions.
 *
 * Hardware:
 *
 *   RPR-0521RS:
 *       SDA : GPIO 5
 *       SCL : GPIO 6
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"
#include "esp_timer.h"

#include "rpr0521rs.h"


// =============================================================================
// PIN CONFIGURATION
// =============================================================================

#define I2C_MASTER_SDA_IO       5
#define I2C_MASTER_SCL_IO       6

#define I2C_MASTER_FREQ_HZ      100000


// =============================================================================
// GESTURE CONFIGURATION
// =============================================================================

/*
 * Minimum proximity change required to trigger a gesture.
 *
 * Positive change:
 *     Hand approaching sensor -> NEXT
 *
 * Negative change:
 *     Hand moving away -> PREV
 */

#define GESTURE_CHANGE           300


/*
 * Minimum time between gestures.
 *
 * Prevents one hand movement from generating multiple commands.
 */

#define GESTURE_TIMEOUT_MS       500


/*
 * Sensor polling interval.
 */

#define SENSOR_POLL_MS           40


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "PAGE_TURNER";

static i2c_master_dev_handle_t sensor_dev = NULL;


/*
 * Previous proximity measurement.
 */

static uint16_t previous_proximity = 0;


/*
 * Time at which the last gesture was detected.
 */

static uint64_t last_gesture_ms = 0;


// =============================================================================
// MILLIS
// =============================================================================

static uint64_t millis(void)
{
    return esp_timer_get_time() / 1000ULL;
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

        /*
         * Use the address defined by the RPR-0521RS ESP-IDF driver.
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
// SENSOR INITIALIZATION
// =============================================================================

static void proximity_sensor_init(void)
{
    /*
     * Use the same native RPR-0521RS driver as the working
     * reaction-game example.
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
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE I2C
    // =========================================================================

    i2c_master_init();


    // =========================================================================
    // INITIALIZE RPR-0521RS
    // =========================================================================

    proximity_sensor_init();


    // =========================================================================
    // INITIAL SENSOR READING
    // =========================================================================
    //
    // This is important.
    //
    // We initialize previous_proximity using an actual sensor reading so that
    // the first measurement after boot cannot accidentally look like a large
    // gesture.
    //

    uint16_t proximity = 0;

    float lux = 0.0f;


    if (rpr0521rs_get_psalsval(
            sensor_dev,
            &proximity,
            &lux
        ) == ESP_OK) {

        previous_proximity =
            proximity;


        ESP_LOGI(
            TAG,
            "Initial proximity: %u",
            proximity
        );
    }


    // =========================================================================
    // READY
    // =========================================================================

    ESP_LOGI(
        TAG,
        "Touchless Page Turner ready."
    );


    printf(
        "Touchless Page Turner ready.\n"
    );


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        // ---------------------------------------------------------------------
        // SENSOR VALUES
        // ---------------------------------------------------------------------

        uint16_t current_proximity = 0;

        float current_lux = 0.0f;


        // ---------------------------------------------------------------------
        // READ RPR-0521RS
        // ---------------------------------------------------------------------

        esp_err_t err =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &current_proximity,
                &current_lux
            );


        if (err != ESP_OK) {

            ESP_LOGW(
                TAG,
                "Proximity sensor read failed: %s",
                esp_err_to_name(err)
            );


            vTaskDelay(
                pdMS_TO_TICKS(
                    SENSOR_POLL_MS
                )
            );


            continue;
        }


        // ---------------------------------------------------------------------
        // CALCULATE PROXIMITY CHANGE
        // ---------------------------------------------------------------------

        int change =
            (int)current_proximity -
            (int)previous_proximity;


        // ---------------------------------------------------------------------
        // CURRENT TIME
        // ---------------------------------------------------------------------

        uint64_t now =
            millis();


        // ---------------------------------------------------------------------
        // GESTURE TIMEOUT
        // ---------------------------------------------------------------------

        if ((now - last_gesture_ms) >
            GESTURE_TIMEOUT_MS) {


            // -----------------------------------------------------------------
            // RAPID APPROACH -> NEXT
            // -----------------------------------------------------------------

            if (change > GESTURE_CHANGE) {

                /*
                 * Large positive proximity change means the hand
                 * is rapidly approaching the sensor.
                 */

                printf(
                    "NEXT\n"
                );


                fflush(stdout);


                ESP_LOGI(
                    TAG,
                    "Gesture: NEXT "
                    "(proximity %u -> %u, change %+d)",
                    previous_proximity,
                    current_proximity,
                    change
                );


                last_gesture_ms =
                    now;
            }


            // -----------------------------------------------------------------
            // RAPID RETREAT -> PREV
            // -----------------------------------------------------------------

            else if (change < -GESTURE_CHANGE) {

                /*
                 * Large negative proximity change means the hand
                 * is rapidly moving away from the sensor.
                 */

                printf(
                    "PREV\n"
                );


                fflush(stdout);


                ESP_LOGI(
                    TAG,
                    "Gesture: PREV "
                    "(proximity %u -> %u, change %+d)",
                    previous_proximity,
                    current_proximity,
                    change
                );


                last_gesture_ms =
                    now;
            }
        }


        // ---------------------------------------------------------------------
        // SAVE PROXIMITY FOR NEXT ITERATION
        // ---------------------------------------------------------------------

        previous_proximity =
            current_proximity;


        // ---------------------------------------------------------------------
        // POLL EVERY 40 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_POLL_MS
            )
        );
    }
}
