/*
 * =============================================================================
 * LSM6DSOX + 5x5 WS2812 TILT MAZE
 * ESP-IDF v5+
 * =============================================================================
 *
 * Tilt-controlled maze game.
 *
 * Tilt the board to move the green player through the maze.
 *
 * Blue  = wall
 * Red   = goal
 * Green = player
 *
 * Hardware:
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
 *       Serpentine wiring
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "led_strip.h"

#include "esp_log.h"


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
 * 0100 = 104 Hz
 * 10   = +/-4g
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
// CONSTANTS
// =============================================================================

#define ACCEL_THRESHOLD          4.0f

/*
 * +/-4g sensitivity:
 *
 * 0.122 mg/LSB
 *
 * 0.000122 g/LSB
 */
#define ACCEL_SENSITIVITY_G      0.000122f

#define GRAVITY                  9.80665f


static const char *TAG = "TILT_MAZE";

static spi_device_handle_t imu_spi = NULL;

static led_strip_handle_t led_strip = NULL;


// =============================================================================
// MAZE
// =============================================================================

/*
 * 0 = free
 * 1 = wall
 *
 *     X
 *   0 1 2 3 4
 *
 * Y 0   W
 *   1   W   W
 *   2       W
 *   3 W W
 *   4       W
 *
 */

static const bool walls[5][5] = {
    {0, 1, 0, 0, 0},
    {0, 1, 0, 1, 0},
    {0, 0, 0, 1, 0},
    {1, 1, 0, 0, 0},
    {0, 0, 0, 1, 0}
};


// =============================================================================
// PLAYER
// =============================================================================

static int player_x = 0;
static int player_y = 0;


// =============================================================================
// MATRIX PIXEL MAPPING
// =============================================================================
//
// Serpentine wiring:
//
//     0   1   2   3   4
//     9   8   7   6   5
//    10  11  12  13  14
//    19  18  17  16  15
//    20  21  22  23  24
//
// =============================================================================

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


// =============================================================================
// MATRIX INITIALIZATION
// =============================================================================

static void matrix_init(void)
{
    /*
     * WS2812 uses GRB pixel order.
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
     * RMT:
     *
     * 10 MHz = 100 ns/tick.
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
// DRAW MAZE
// =============================================================================

static void draw_maze(void)
{
    matrix_clear();


    // -------------------------------------------------------------------------
    // DRAW WALLS
    // -------------------------------------------------------------------------

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            if (walls[y][x]) {

                matrix_set_pixel(
                    pixel_xy(x, y),

                    0,      // Red
                    0,      // Green
                    100     // Blue
                );
            }
        }
    }


    // -------------------------------------------------------------------------
    // DRAW GOAL
    // -------------------------------------------------------------------------

    matrix_set_pixel(
        pixel_xy(4, 4),

        255,    // Red
        0,      // Green
        0       // Blue
    );


    // -------------------------------------------------------------------------
    // DRAW PLAYER
    // -------------------------------------------------------------------------

    matrix_set_pixel(
        pixel_xy(player_x, player_y),

        0,      // Red
        255,    // Green
        0       // Blue
    );


    // -------------------------------------------------------------------------
    // SHOW
    // -------------------------------------------------------------------------

    matrix_show();
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
     * rx_buf[0] is the response during
     * the register-address phase.
     *
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
         * Conservative SPI clock.
         */

        .clock_speed_hz = 1 * 1000 * 1000,

        /*
         * LSM6DSOX SPI mode 0.
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
    // INITIALIZE MATRIX
    // =========================================================================

    matrix_init();


    // =========================================================================
    // INITIALIZE IMU
    // =========================================================================

    imu_init();


    // =========================================================================
    // DRAW INITIAL MAZE
    // =========================================================================

    draw_maze();


    // =========================================================================
    // MAIN GAME LOOP
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
                pdMS_TO_TICKS(180)
            );

            continue;
        }


        // ---------------------------------------------------------------------
        // CONVERT RAW DATA TO m/s²
        // ---------------------------------------------------------------------

        /*
         * Sensor is configured for +/-4g.
         *
         * Sensitivity:
         *
         * 0.122 mg/LSB
         *
         * Therefore:
         *
         * raw * 0.000122 = g
         *
         * g * 9.80665 = m/s²
         */

        float accel_x =
            (float)raw_x *
            ACCEL_SENSITIVITY_G *
            GRAVITY;


        float accel_y =
            (float)raw_y *
            ACCEL_SENSITIVITY_G *
            GRAVITY;


        // ---------------------------------------------------------------------
        // DEBUG
        // ---------------------------------------------------------------------

        /*
         * Uncomment if you want to see accelerometer values.
         *
         * ESP_LOGI(
         *     TAG,
         *     "AX %.2f  AY %.2f  AZ %.2f",
         *     accel_x,
         *     accel_y,
         *     (float)raw_z * ACCEL_SENSITIVITY_G * GRAVITY
         * );
         */


        // ---------------------------------------------------------------------
        // CALCULATE NEW POSITION
        // ---------------------------------------------------------------------

        int new_x = player_x;
        int new_y = player_y;


        /*
         * Same direction mapping as the Arduino version.
         *
         * Accelerometer Y controls display X.
         *
         * Positive Y -> move left.
         * Negative Y -> move right.
         */

        if (accel_y < ACCEL_THRESHOLD) {

            new_x--;
        }


        if (accel_y > -ACCEL_THRESHOLD) {

            new_x++;
        }


        /*
         * Accelerometer X controls display Y.
         *
         * Positive X -> move down.
         * Negative X -> move up.
         */

        if (accel_x < ACCEL_THRESHOLD) {

            new_y++;
        }


        if (accel_x > -ACCEL_THRESHOLD) {

            new_y--;
        }


        // ---------------------------------------------------------------------
        // CLAMP TO MAZE BOUNDARIES
        // ---------------------------------------------------------------------

        if (new_x < 0) {
            new_x = 0;
        }

        if (new_x > 4) {
            new_x = 4;
        }


        if (new_y < 0) {
            new_y = 0;
        }

        if (new_y > 4) {
            new_y = 4;
        }


        // ---------------------------------------------------------------------
        // CHECK WALL
        // ---------------------------------------------------------------------

        if (!walls[new_y][new_x]) {

            player_x = new_x;
            player_y = new_y;
        }


        // ---------------------------------------------------------------------
        // CHECK GOAL
        // ---------------------------------------------------------------------

        if (player_x == 4 &&
            player_y == 4) {

            /*
             * Fill the entire matrix green.
             */

            for (int i = 0; i < NUM_LEDS; i++) {

                matrix_set_pixel(
                    i,

                    0,      // Red
                    255,    // Green
                    0       // Blue
                );
            }


            matrix_show();


            /*
             * Keep victory screen visible
             * for 500 ms.
             */

            vTaskDelay(
                pdMS_TO_TICKS(500)
            );


            /*
             * Restart player at top-left.
             */

            player_x = 0;
            player_y = 0;
        }


        // ---------------------------------------------------------------------
        // DRAW MAZE
        // ---------------------------------------------------------------------

        draw_maze();


        // ---------------------------------------------------------------------
        // GAME UPDATE RATE
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(180)
        );
    }
}
