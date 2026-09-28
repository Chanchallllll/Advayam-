/*
 * =============================================================================
 * PROXIMITY DEPTH GAUGE - ESP-IDF v5+
 * =============================================================================
 * Displays 1 to 5 illuminated rows on a 5x5 RGB matrix as a hand gets
 * closer to the RPR-0521RS.
 *
 * I2C:
 *   SDA       : GPIO 5
 *   SCL       : GPIO 6
 *
 * RGB Matrix:
 *   Data      : GPIO 7
 *   Pixels    : 25
 *   Protocol  : WS2812 / NeoPixel, GRB, 800 kHz
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
 * Proximity thresholds.
 *
 * These should be calibrated for your particular sensor/board.
 */
#define PROXIMITY_LEVEL_1       100
#define PROXIMITY_LEVEL_2       300
#define PROXIMITY_LEVEL_3       600
#define PROXIMITY_LEVEL_4       1000
#define PROXIMITY_LEVEL_5       1500


static const char *TAG = "PROXIMITY_GAUGE";


/*
 * =============================================================================
 * Global handles
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;

static led_strip_handle_t matrix = NULL;


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
         * The LED strip driver uses GRB ordering for WS2812/NeoPixel.
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
     * Clear the matrix.
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
 * Set Pixel
 * =============================================================================
 *
 * ESP-IDF uses RGB values directly here.
 *
 * color:
 *   R, G, B each range from 0 to 255.
 * =============================================================================
 */

static void set_pixel(
    uint32_t index,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (index >= NUM_PIXELS) {
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
 * Set Matrix Row
 * =============================================================================
 *
 * The physical matrix is wired in a serpentine pattern:
 *
 *   Row 0:  → → → → →
 *   Row 1:  ← ← ← ← ←
 *   Row 2:  → → → → →
 *   Row 3:  ← ← ← ← ←
 *   Row 4:  → → → → →
 *
 * This matches the Arduino code.
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


    for (int col = 0; col < 5; col++) {

        int index;


        if ((row % 2) == 0) {

            /*
             * Even rows:
             * left -> right
             */
            index = row * 5 + col;

        } else {

            /*
             * Odd rows:
             * right -> left
             */
            index = row * 5 + (4 - col);
        }


        set_pixel(
            index,
            red,
            green,
            blue
        );
    }
}


/*
 * =============================================================================
 * Set Number of Illuminated Rows
 * =============================================================================
 */

static void set_rows(int rows)
{
    /*
     * Clear all 25 pixels first.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    /*
     * Blue/cyan color.
     *
     * Equivalent to:
     *
     * matrix.Color(0, 150, 255)
     */
    for (int row = 0; row < rows; row++) {

        set_row(
            row,
            0,      // Red
            150,    // Green
            255     // Blue
        );
    }


    /*
     * Send the new pixel data to the matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Convert Proximity Value to Number of Rows
 * =============================================================================
 */

static int proximity_to_rows(uint16_t proximity)
{
    int rows = 0;


    if (proximity > PROXIMITY_LEVEL_1) {
        rows = 1;
    }

    if (proximity > PROXIMITY_LEVEL_2) {
        rows = 2;
    }

    if (proximity > PROXIMITY_LEVEL_3) {
        rows = 3;
    }

    if (proximity > PROXIMITY_LEVEL_4) {
        rows = 4;
    }

    if (proximity > PROXIMITY_LEVEL_5) {
        rows = 5;
    }


    return rows;
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * Initialize I2C.
     */
    i2c_master_init();


    /*
     * Initialize RPR-0521RS.
     */
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Sensor initialization failed!"
        );

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }


    /*
     * Initialize RGB matrix.
     */
    matrix_init();


    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "      PROXIMITY DEPTH GAUGE");
    ESP_LOGI(TAG, "========================================");

    ESP_LOGI(
        TAG,
        "Matrix: GPIO %d, %d pixels",
        MATRIX_PIN,
        NUM_PIXELS
    );


    /*
     * Main loop.
     */
    while (1) {

        uint16_t proximity;
        float lux;


        /*
         * Read proximity and ambient light.
         */
        if (rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            ) == ESP_OK) {


            /*
             * Convert proximity to 0-5 illuminated rows.
             */
            int rows = proximity_to_rows(proximity);


            /*
             * Update matrix.
             */
            set_rows(rows);


            /*
             * Optional debug output.
             *
             * Uncomment if you want to see the raw values.
             */
            /*
            ESP_LOGI(
                TAG,
                "Proximity: %u | Lux: %.2f | Rows: %d",
                proximity,
                lux,
                rows
            );
            */


        } else {

            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );
        }


        /*
         * Equivalent to Arduino delay(80).
         */
        vTaskDelay(
            pdMS_TO_TICKS(80)
        );
    }
}
