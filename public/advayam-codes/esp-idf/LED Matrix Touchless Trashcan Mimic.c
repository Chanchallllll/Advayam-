/*
 * =============================================================================
 * TOUCHLESS TRASHCAN LID MIMIC - ESP-IDF v5+
 * =============================================================================
 *
 * Hold a hand over the RPR-0521RS proximity sensor for approximately
 * one second.
 *
 * The RGB matrix performs an animation that mimics a trashcan lid opening:
 *
 *   1. Wait for hand
 *   2. Hand remains present for 1 second
 *   3. Matrix fills from bottom to top
 *   4. Hold
 *   5. Matrix empties from top to bottom
 *   6. Wait for hand to leave
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * RGB Matrix:
 *   DATA = GPIO 7
 *   PIXELS = 25
 *   WS2812 / NeoPixel
 *   GRB
 *
 * Proximity sensor:
 *   RPR-0521RS
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

#include "rpr0521rs.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define I2C_MASTER_SDA_IO       5
#define I2C_MASTER_SCL_IO       6
#define I2C_MASTER_FREQ_HZ      100000

#define MATRIX_PIN              7
#define NUM_PIXELS              25

#define MATRIX_BRIGHTNESS       20

/*
 * Proximity threshold.
 *
 * Adjust this for your sensor/board if necessary.
 */
#define PROXIMITY_THRESHOLD     500

/*
 * Required continuous hover time.
 */
#define HOVER_TIME_MS           1000

/*
 * Animation timing.
 */
#define ROW_DELAY_MS            120
#define HOLD_TIME_MS            500

/*
 * Main sensor polling interval.
 */
#define SENSOR_DELAY_MS         50


static const char *TAG = "TRASHCAN_LID";


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;

static led_strip_handle_t matrix = NULL;


/*
 * =============================================================================
 * State
 * =============================================================================
 */

static uint32_t hover_start = 0;

static bool animation_played = false;


/*
 * =============================================================================
 * I2C Initialization
 * =============================================================================
 */

static void i2c_master_init(void)
{
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


    /*
     * RPR-0521RS I2C device.
     */
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,

        .device_address =
            RPR0521RS_DEVICE_ADDRESS,

        .scl_speed_hz =
            I2C_MASTER_FREQ_HZ,
    };


    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            bus_handle,
            &dev_config,
            &sensor_dev
        )
    );
}


/*
 * =============================================================================
 * Matrix Initialization
 * =============================================================================
 */

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,

        .max_leds = NUM_PIXELS,

        .led_pixel_format =
            LED_PIXEL_FORMAT_GRB,

        .led_model =
            LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        }
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src =
            RMT_CLK_SRC_DEFAULT,

        .resolution_hz =
            10 * 1000 * 1000,

        .mem_block_symbols = 64,

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );


    /*
     * Start with matrix OFF.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Pixel Mapping
 * =============================================================================
 *
 * The matrix is wired in a serpentine pattern:
 *
 *   Row 0:  → → → → →
 *   Row 1:  ← ← ← ← ←
 *   Row 2:  → → → → →
 *   Row 3:  ← ← ← ← ←
 *   Row 4:  → → → → →
 *
 * This function converts normal X/Y coordinates into
 * the physical LED index.
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {

        return -1;
    }


    if ((y % 2) == 0) {

        return y * 5 + x;
    }


    return y * 5 + (4 - x);
}


/*
 * =============================================================================
 * Set Pixel
 * =============================================================================
 */

static void set_pixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    int index = pixel_xy(x, y);


    if (index < 0) {
        return;
    }


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            index,
            red,
            green,
            blue
        )
    );
}


/*
 * =============================================================================
 * Set Entire Row
 * =============================================================================
 */

static void set_row(
    int row,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (row < 0 || row >= 5) {
        return;
    }


    for (int x = 0; x < 5; x++) {

        set_pixel(
            x,
            row,
            red,
            green,
            blue
        );
    }
}


/*
 * =============================================================================
 * Clear Matrix
 * =============================================================================
 */

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Animate Lid
 * =============================================================================
 *
 * Opening:
 *
 *   Row 0
 *   Row 1
 *   Row 2
 *   Row 3
 *   Row 4
 *
 * Closing:
 *
 *   Row 4
 *   Row 3
 *   Row 2
 *   Row 1
 *   Row 0
 *
 * =============================================================================
 */

static void animate_lid(void)
{
    ESP_LOGI(
        TAG,
        "Opening lid..."
    );


    /*
     * ---------------------------------------------------------
     * OPENING ANIMATION
     * ---------------------------------------------------------
     *
     * Gradually illuminate rows from bottom to top.
     *
     * This matches the original Arduino program:
     *
     * for (row = 0; row < 5; row++)
     *
     * ---------------------------------------------------------
     */

    for (int row = 0; row < 5; row++) {

        set_row(
            row,

            255,    // Red
            120,    // Green
            20      // Blue
        );


        ESP_ERROR_CHECK(
            led_strip_refresh(matrix)
        );


        vTaskDelay(
            pdMS_TO_TICKS(ROW_DELAY_MS)
        );
    }


    /*
     * ---------------------------------------------------------
     * LID OPEN
     * ---------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "Lid open"
    );


    vTaskDelay(
        pdMS_TO_TICKS(HOLD_TIME_MS)
    );


    /*
     * ---------------------------------------------------------
     * CLOSING ANIMATION
     * ---------------------------------------------------------
     *
     * Turn rows off from top to bottom.
     * ---------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "Closing lid..."
    );


    for (int row = 4; row >= 0; row--) {

        set_row(
            row,

            0,
            0,
            0
        );


        ESP_ERROR_CHECK(
            led_strip_refresh(matrix)
        );


        vTaskDelay(
            pdMS_TO_TICKS(ROW_DELAY_MS)
        );
    }


    ESP_LOGI(
        TAG,
        "Lid closed"
    );
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * ---------------------------------------------------------
     * Initialize I2C
     * ---------------------------------------------------------
     */

    i2c_master_init();


    /*
     * ---------------------------------------------------------
     * Initialize RPR-0521RS
     * ---------------------------------------------------------
     */

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Sensor initialization failed!"
        );


        while (1) {

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    /*
     * ---------------------------------------------------------
     * Initialize RGB matrix
     * ---------------------------------------------------------
     */

    matrix_init();


    /*
     * ---------------------------------------------------------
     * Startup information
     * ---------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "      TOUCHLESS TRASHCAN LID"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "I2C SDA: GPIO %d",
        I2C_MASTER_SDA_IO
    );

    ESP_LOGI(
        TAG,
        "I2C SCL: GPIO %d",
        I2C_MASTER_SCL_IO
    );

    ESP_LOGI(
        TAG,
        "Matrix: GPIO %d",
        MATRIX_PIN
    );

    ESP_LOGI(
        TAG,
        "Proximity threshold: %d",
        PROXIMITY_THRESHOLD
    );

    ESP_LOGI(
        TAG,
        "Hover time: %d ms",
        HOVER_TIME_MS
    );


    /*
     * ---------------------------------------------------------
     * Main Loop
     * ---------------------------------------------------------
     */

    while (1)
    {
        uint16_t proximity = 0;
        float lux = 0.0f;


        /*
         * Read proximity sensor.
         */
        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (ret == ESP_OK)
        {
            /*
             * -------------------------------------------------
             * HAND DETECTED
             * -------------------------------------------------
             */

            if (proximity > PROXIMITY_THRESHOLD)
            {
                /*
                 * Start timing when the hand first appears.
                 */
                if (hover_start == 0)
                {
                    hover_start = xTaskGetTickCount()
                                   * portTICK_PERIOD_MS;

                    ESP_LOGI(
                        TAG,
                        "Hand detected - starting timer"
                    );
                }


                uint32_t now =
                    xTaskGetTickCount()
                    * portTICK_PERIOD_MS;


                /*
                 * -------------------------------------------------
                 * CONTINUOUS HOVER
                 * -------------------------------------------------
                 */

                if (!animation_played &&
                    (now - hover_start >= HOVER_TIME_MS))
                {
                    /*
                     * Play animation.
                     */
                    animate_lid();


                    /*
                     * Prevent another animation while the
                     * hand remains above the sensor.
                     */
                    animation_played = true;


                    ESP_LOGI(
                        TAG,
                        "Animation complete"
                    );
                }
            }


            /*
             * -------------------------------------------------
             * HAND REMOVED
             * -------------------------------------------------
             */

            else
            {
                /*
                 * Reset the timer and allow another animation.
                 */
                if (hover_start != 0)
                {
                    ESP_LOGI(
                        TAG,
                        "Hand removed"
                    );
                }


                hover_start = 0;

                animation_played = false;
            }
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );
        }


        /*
         * Optional debug output.
         *
         * Uncomment if you want raw proximity values:
         *
         * ESP_LOGI(
         *     TAG,
         *     "Proximity: %u | Lux: %.2f",
         *     proximity,
         *     lux
         * );
         */


        /*
         * Equivalent to Arduino delay(50).
         */
        vTaskDelay(
            pdMS_TO_TICKS(SENSOR_DELAY_MS)
        );
    }
}
