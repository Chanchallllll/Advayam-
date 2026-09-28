/*
 * =============================================================================
 * TOUCHLESS GESTURE MODE SWITCH - ESP-IDF v5+
 * =============================================================================
 *
 * Uses the RPR-0521RS proximity sensor to detect a hand approaching
 * the sensor.
 *
 * Each detected approach cycles the RGB matrix through:
 *
 *   Mode 0 -> RED
 *   Mode 1 -> GREEN
 *   Mode 2 -> BLUE
 *   Mode 0 -> RED
 *
 * IMPORTANT:
 * The RPR-0521RS is a single-point proximity sensor.
 * It cannot determine true LEFT/RIGHT hand movement.
 *
 * This program therefore treats:
 *
 *   Proximity below threshold
 *             ->
 *   Proximity rising above threshold
 *             ->
 *   GESTURE DETECTED
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * RGB Matrix:
 *   Data   = GPIO 7
 *   Pixels = 25
 *   WS2812 / NeoPixel
 *   GRB
 *   800 kHz
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

/*
 * Matrix brightness.
 *
 * The ESP-IDF led_strip driver does not have the same global brightness
 * function as Adafruit_NeoPixel, so brightness is applied directly to
 * the RGB values.
 *
 * 80 / 255 is approximately the original Arduino brightness.
 */
#define MATRIX_BRIGHTNESS       80


/*
 * Proximity level that triggers a gesture.
 *
 * Calibrate this value for your particular sensor and physical setup.
 */
#define GESTURE_THRESHOLD       500


/*
 * Minimum time between gestures.
 *
 * Equivalent to:
 *
 * millis() - lastGestureTime > 700
 */
#define GESTURE_COOLDOWN_MS     700


static const char *TAG = "GESTURE_MODE";


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;

static led_strip_handle_t matrix = NULL;


/*
 * =============================================================================
 * Game / Display State
 * =============================================================================
 */

static int display_mode = 0;

static uint16_t previous_proximity = 0;

static int64_t last_gesture_time_ms = 0;


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


    i2c_master_bus_handle_t bus_handle;


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
}


/*
 * =============================================================================
 * RGB Matrix Initialization
 * =============================================================================
 */

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,

        .max_leds = NUM_PIXELS,

        /*
         * WS2812 uses GRB ordering.
         */
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,

        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        }
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        .resolution_hz = 10 * 1000 * 1000,

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
     * Start with the matrix completely off.
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
 * Brightness Scaling
 * =============================================================================
 *
 * Converts an RGB value from the normal 0-255 range to the configured
 * matrix brightness.
 * =============================================================================
 */

static uint8_t scale_brightness(uint8_t value)
{
    return (uint8_t)(
        ((uint16_t)value * MATRIX_BRIGHTNESS) / 255
    );
}


/*
 * =============================================================================
 * Set Entire Matrix
 * =============================================================================
 */

static void set_matrix_color(
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    /*
     * Apply brightness scaling.
     */
    red   = scale_brightness(red);
    green = scale_brightness(green);
    blue  = scale_brightness(blue);


    /*
     * Set all 25 pixels.
     */
    for (int i = 0; i < NUM_PIXELS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                matrix,
                i,
                red,
                green,
                blue
            )
        );
    }


    /*
     * Send data to the matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Display Current Mode
 * =============================================================================
 */

static void show_mode(void)
{
    switch (display_mode)
    {
        /*
         * Mode 0 - RED
         */
        case 0:

            set_matrix_color(
                255,
                0,
                0
            );

            ESP_LOGI(
                TAG,
                "Display mode: 0 - RED"
            );

            break;


        /*
         * Mode 1 - GREEN
         */
        case 1:

            set_matrix_color(
                0,
                255,
                0
            );

            ESP_LOGI(
                TAG,
                "Display mode: 1 - GREEN"
            );

            break;


        /*
         * Mode 2 - BLUE
         */
        case 2:

            set_matrix_color(
                0,
                0,
                255
            );

            ESP_LOGI(
                TAG,
                "Display mode: 2 - BLUE"
            );

            break;
    }
}


/*
 * =============================================================================
 * Cycle Display Mode
 * =============================================================================
 */

static void next_mode(void)
{
    display_mode++;

    if (display_mode > 2) {
        display_mode = 0;
    }


    show_mode();
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * -------------------------------------------------------------------------
     * Initialize I2C
     * -------------------------------------------------------------------------
     */

    i2c_master_init();


    /*
     * -------------------------------------------------------------------------
     * Initialize RPR-0521RS
     * -------------------------------------------------------------------------
     */

    if (rpr0521rs_init(sensor_dev) != ESP_OK)
    {
        ESP_LOGE(
            TAG,
            "RPR-0521RS initialization failed!"
        );

        while (1) {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    /*
     * -------------------------------------------------------------------------
     * Initialize RGB matrix
     * -------------------------------------------------------------------------
     */

    matrix_init();


    /*
     * -------------------------------------------------------------------------
     * Startup
     * -------------------------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "    TOUCHLESS GESTURE MODE SWITCH"
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
        "Gesture threshold: %d",
        GESTURE_THRESHOLD
    );


    /*
     * Show initial mode.
     *
     * Starts RED.
     */
    show_mode();


    /*
     * -------------------------------------------------------------------------
     * Main Loop
     * -------------------------------------------------------------------------
     */

    while (1)
    {
        uint16_t proximity;
        float lux;


        /*
         * Read proximity and ambient light.
         */
        esp_err_t result =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (result == ESP_OK)
        {
            /*
             * Get current time in milliseconds.
             *
             * We use FreeRTOS ticks instead of esp_timer_get_time(),
             * avoiding the esp_timer_get_time() include problem you
             * encountered in the earlier Arduino conversions.
             */
            int64_t now_ms =
                (int64_t)(
                    xTaskGetTickCount()
                    * portTICK_PERIOD_MS
                );


            /*
             * ================================================================
             * Gesture Detection
             * ================================================================
             *
             * A gesture occurs when:
             *
             *   current proximity >= threshold
             *
             * AND
             *
             *   previous proximity < threshold
             *
             * AND
             *
             *   enough time has passed since the previous gesture.
             *
             * This detects the rising edge of the proximity signal.
             */

            if (proximity > GESTURE_THRESHOLD &&
                previous_proximity <= GESTURE_THRESHOLD &&
                (now_ms - last_gesture_time_ms) >
                    GESTURE_COOLDOWN_MS)
            {
                /*
                 * Cycle to the next display mode.
                 */
                next_mode();


                /*
                 * Record gesture time.
                 */
                last_gesture_time_ms =
                    now_ms;


                ESP_LOGI(
                    TAG,
                    "Gesture detected! "
                    "Proximity: %u",
                    proximity
                );
            }


            /*
             * Store proximity for the next loop.
             */
            previous_proximity =
                proximity;
        }
        else
        {
            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );
        }


        /*
         * Equivalent to Arduino:
         *
         * delay(40);
         */
        vTaskDelay(
            pdMS_TO_TICKS(40)
        );
    }
}
