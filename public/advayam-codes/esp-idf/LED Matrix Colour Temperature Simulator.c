/*
 * =============================================================================
 * COLOR TEMPERATURE SIMULATOR FROM AMBIENT LUX
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Adjusts the RGB matrix color according to ambient light:
 *
 *     Bright room -> cool white
 *     Dim room    -> warm amber
 *
 * Sensors:
 *
 *     RPR-0521RS
 *         SDA = GPIO 5
 *         SCL = GPIO 6
 *
 * Matrix:
 *
 *     WS2812 / NeoPixel
 *         Data = GPIO 7
 *         LEDs = 25
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"
#include "esp_err.h"

#include "led_strip.h"
#include "rpr0521rs.h"


/* =============================================================================
 * Hardware Configuration
 * =============================================================================
 */

#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000

#define MATRIX_PIN           7
#define NUM_PIXELS           25

#define MATRIX_BRIGHTNESS    100


static const char *TAG = "LUX_COLOR";


/* =============================================================================
 * Globals
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;

static led_strip_handle_t led_strip = NULL;


/* =============================================================================
 * I2C Initialization
 * =============================================================================
 */

static void i2c_master_init(void)
{
    /*
     * Create I2C master bus.
     */
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
     * Add RPR-0521RS to the I2C bus.
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


    ESP_LOGI(
        TAG,
        "I2C initialized"
    );
}


/* =============================================================================
 * RGB Matrix Initialization
 * =============================================================================
 */

static void led_strip_init_board(void)
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

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );


    /*
     * Start with the matrix turned off.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "RGB matrix initialized on GPIO %d",
        MATRIX_PIN
    );
}


/* =============================================================================
 * Constrain Float
 * =============================================================================
 *
 * Arduino provides constrain().
 * ESP-IDF/C does not, so we implement the equivalent here.
 *
 * =============================================================================
 */

static float constrain_float(
    float value,
    float minimum,
    float maximum
)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}


/* =============================================================================
 * Update Matrix From Lux
 * =============================================================================
 */

static void update_matrix(float lux)
{
    /*
     * -------------------------------------------------------------------------
     * Convert 0-500 lux into a 0.0-1.0 ratio.
     * -------------------------------------------------------------------------
     *
     *     0 lux    -> 0.0
     *     250 lux  -> 0.5
     *     500 lux  -> 1.0
     *
     * Values above 500 lux are clamped to 1.0.
     */
    float ratio =
        constrain_float(
            lux,
            0.0f,
            500.0f
        ) / 500.0f;


    /*
     * -------------------------------------------------------------------------
     * Calculate RGB color.
     * -------------------------------------------------------------------------
     *
     * Red:
     *     Always 255
     *
     * Green:
     *     80 at 0 lux
     *     255 at 500 lux
     *
     * Blue:
     *     0 at 0 lux
     *     255 at 500 lux
     *
     * Therefore:
     *
     *     Dark -> warm amber
     *     Bright -> cool white
     */
    uint8_t red =
        255;


    uint8_t green =
        80 +
        (uint8_t)(175.0f * ratio);


    uint8_t blue =
        (uint8_t)(255.0f * ratio);


    /*
     * -------------------------------------------------------------------------
     * Update all 25 LEDs.
     * -------------------------------------------------------------------------
     */

    for (int i = 0; i < NUM_PIXELS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                red,
                green,
                blue
            )
        );
    }


    /*
     * Send the new frame to the matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    /*
     * Print values.
     */
    ESP_LOGI(
        TAG,
        "Lux: %.2f | RGB: %d,%d,%d",
        (double)lux,
        red,
        green,
        blue
    );
}


/* =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " Color Temperature From Ambient Lux");
    ESP_LOGI(TAG, "==============================================");


    /* -------------------------------------------------------------------------
     * Initialize I2C
     * -------------------------------------------------------------------------
     */

    i2c_master_init();


    /* -------------------------------------------------------------------------
     * Initialize RPR-0521RS
     * -------------------------------------------------------------------------
     */

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "RPR-0521RS initialization failed!"
        );

        ESP_LOGE(
            TAG,
            "Check the sensor and I2C connections."
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


    /* -------------------------------------------------------------------------
     * Initialize RGB matrix
     * -------------------------------------------------------------------------
     */

    led_strip_init_board();


    /*
     * The Arduino code uses:
     *
     *     matrix.setBrightness(100);
     *
     * The ESP-IDF led_strip component doesn't provide an equivalent
     * global setBrightness() function.
     *
     * For this program, we reproduce the same effect by scaling the
     * RGB values before sending them to the LEDs.
     *
     * See the brightness scaling below.
     */


    ESP_LOGI(
        TAG,
        "Matrix brightness: %d",
        MATRIX_BRIGHTNESS
    );


    ESP_LOGI(
        TAG,
        "Starting ambient-light color control..."
    );


    /* -------------------------------------------------------------------------
     * Main Loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        uint16_t proximity = 0;

        float lux = 0.0f;


        /*
         * Read proximity and ambient light from RPR-0521RS.
         */
        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (ret == ESP_OK) {

            /*
             * ---------------------------------------------------------------
             * Convert lux into color.
             * ---------------------------------------------------------------
             */
            float ratio =
                constrain_float(
                    lux,
                    0.0f,
                    500.0f
                ) / 500.0f;


            uint8_t red =
                (uint8_t)(
                    255.0f *
                    MATRIX_BRIGHTNESS /
                    255.0f
                );


            uint8_t green =
                (uint8_t)(
                    (80.0f + 175.0f * ratio) *
                    MATRIX_BRIGHTNESS /
                    255.0f
                );


            uint8_t blue =
                (uint8_t)(
                    (255.0f * ratio) *
                    MATRIX_BRIGHTNESS /
                    255.0f
                );


            /*
             * ---------------------------------------------------------------
             * Set all LEDs.
             * ---------------------------------------------------------------
             */
            for (int i = 0; i < NUM_PIXELS; i++) {

                ESP_ERROR_CHECK(
                    led_strip_set_pixel(
                        led_strip,
                        i,
                        red,
                        green,
                        blue
                    )
                );
            }


            /*
             * Send frame.
             */
            ESP_ERROR_CHECK(
                led_strip_refresh(led_strip)
            );


            /*
             * Display readings.
             *
             * We calculate the unscaled RGB values for the serial output,
             * matching the Arduino sketch.
             */
            int display_red = 255;

            int display_green =
                80 +
                (int)(175.0f * ratio);

            int display_blue =
                (int)(255.0f * ratio);


            ESP_LOGI(
                TAG,
                "Lux: %.2f | RGB: %d,%d,%d",
                (double)lux,
                display_red,
                display_green,
                display_blue
            );
        }
        else {

            ESP_LOGE(
                TAG,
                "Error: Failed to read RPR-0521RS: %s",
                esp_err_to_name(ret)
            );
        }


        /*
         * Take another measurement after 300 ms.
         */
        vTaskDelay(
            pdMS_TO_TICKS(300)
        );
    }
}
