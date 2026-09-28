/*
 * =============================================================================
 * VIRTUAL DICE - ESP-IDF
 * =============================================================================
 *
 * Shake or tilt the board to roll the die.
 *
 * When the board becomes flat again, the die settles on a number from 1 to 6.
 *
 * Hardware:
 *
 * 5x5 WS2812 Matrix:
 *   GPIO 7
 *
 * LSM6DSM / LSM6DSOX SPI:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * GPIO 16:
 *   Kept LOW.
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"
#include "esp_timer.h"
#include "esp_random.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "led_strip.h"


//=============================================================================
// GPIO
//=============================================================================

#define MATRIX_PIN      7
#define POWER_PIN       16

#define IMU_CS          10
#define IMU_SCK         12
#define IMU_MOSI        11
#define IMU_MISO        13

#define NUM_LEDS        25


//=============================================================================
// Dice Configuration
//=============================================================================

// Tilt required to start rolling.
#define ROLL_TILT_THRESHOLD       5.0f

// Board must be below this tilt to be considered flat.
#define FLAT_TILT_THRESHOLD       2.0f

// Minimum rolling time before the die can settle.
#define MIN_ROLL_TIME_MS          500

// Time between changing displayed dice values.
#define ROLL_UPDATE_MS            100


static const char *TAG = "VIRTUAL_DICE";


//=============================================================================
// Hardware Handles
//=============================================================================

static spi_device_handle_t imu_spi;

static led_strip_handle_t led_strip;


//=============================================================================
// Game State
//=============================================================================

static bool rolling = false;

static int64_t roll_start_us = 0;
static int64_t last_roll_update_us = 0;


//=============================================================================
// Matrix Mapping
//=============================================================================

/*
 * Physical 5x5 matrix is wired in serpentine order:
 *
 * Row 0:
 *   0  1  2  3  4
 *
 * Row 1:
 *   9  8  7  6  5
 *
 * Row 2:
 *   10 11 12 13 14
 *
 * etc.
 */

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


//=============================================================================
// Matrix
//=============================================================================

static void matrix_clear(void)
{
    for (int i = 0; i < NUM_LEDS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                0,
                0,
                0
            )
        );
    }
}


static void matrix_show(void)
{
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// Show Dice Number
//=============================================================================

/*
 * Displays the number using the first N LEDs.
 *
 * This preserves the behavior of your original Arduino program.
 *
 * 1 -> one LED
 * 2 -> two LEDs
 * ...
 * 6 -> six LEDs
 */

static void show_number(int number)
{
    matrix_clear();

    if (number < 1) {
        number = 1;
    }

    if (number > 6) {
        number = 6;
    }

    for (int i = 0; i < number; i++) {

        int x = i % 5;
        int y = i / 5;

        int index = pixel_xy(x, y);

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                index,
                255,
                255,
                255
            )
        );
    }

    matrix_show();
}


//=============================================================================
// SPI Register Write
//=============================================================================

static esp_err_t imu_write_reg(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t tx[2];

    tx[0] = reg & 0x7F;
    tx[1] = value;

    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx,
    };

    return spi_device_transmit(
        imu_spi,
        &transaction
    );
}


//=============================================================================
// SPI Register Read
//=============================================================================

static esp_err_t imu_read_regs(
    uint8_t reg,
    uint8_t *data,
    size_t length
)
{
    uint8_t tx[length + 1];
    uint8_t rx[length + 1];

    tx[0] = reg | 0x80;

    for (size_t i = 1; i <= length; i++) {
        tx[i] = 0;
    }

    spi_transaction_t transaction = {
        .length = (length + 1) * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t ret =
        spi_device_transmit(
            imu_spi,
            &transaction
        );

    if (ret != ESP_OK) {
        return ret;
    }

    for (size_t i = 0; i < length; i++) {
        data[i] = rx[i + 1];
    }

    return ESP_OK;
}


//=============================================================================
// Initialize IMU
//=============================================================================

static void imu_init(void)
{
    uint8_t who_am_i = 0;

    ESP_ERROR_CHECK(
        imu_read_regs(
            0x0F,
            &who_am_i,
            1
        )
    );

    ESP_LOGI(
        TAG,
        "IMU WHO_AM_I = 0x%02X",
        who_am_i
    );

    /*
     * CTRL1_XL = 0x48
     *
     * ODR = 104 Hz
     * Accelerometer range = +/-4g
     */

    ESP_ERROR_CHECK(
        imu_write_reg(
            0x10,
            0x48
        )
    );

    ESP_LOGI(
        TAG,
        "IMU configured: 104 Hz, +/-4g"
    );
}


//=============================================================================
// Read Accelerometer
//=============================================================================

static esp_err_t imu_read_accel(
    float *ax,
    float *ay,
    float *az
)
{
    uint8_t data[6];

    esp_err_t ret =
        imu_read_regs(
            0x28,
            data,
            6
        );

    if (ret != ESP_OK) {
        return ret;
    }

    int16_t raw_x =
        (int16_t)(
            ((uint16_t)data[1] << 8) |
            data[0]
        );

    int16_t raw_y =
        (int16_t)(
            ((uint16_t)data[3] << 8) |
            data[2]
        );

    int16_t raw_z =
        (int16_t)(
            ((uint16_t)data[5] << 8) |
            data[4]
        );

    /*
     * LSM6DSM/LSM6DSOX:
     *
     * +/-4g = 0.122 mg/LSB
     */

    const float sensitivity =
        0.122f / 1000.0f * 9.80665f;

    *ax = raw_x * sensitivity;
    *ay = raw_y * sensitivity;
    *az = raw_z * sensitivity;

    return ESP_OK;
}


//=============================================================================
// Calculate Tilt
//=============================================================================

static float calculate_tilt(
    float ax,
    float ay
)
{
    /*
     * Same concept as the original Arduino program:
     *
     * tilt = sqrt(ax² + ay²)
     *
     * When the board is flat, gravity is mostly on Z,
     * so X and Y are close to zero.
     */

    return sqrtf(
        ax * ax +
        ay * ay
    );
}


//=============================================================================
// Initialize SPI
//=============================================================================

static void spi_init(void)
{
    spi_bus_config_t bus_config = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST,
            &bus_config,
            SPI_DMA_CH_AUTO
        )
    );

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 3,
        .spics_io_num = IMU_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(
        spi_bus_add_device(
            SPI2_HOST,
            &device_config,
            &imu_spi
        )
    );
}


//=============================================================================
// Initialize LED Matrix
//=============================================================================

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 0,
        .flags.with_dma = false,
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );

    matrix_clear();
    matrix_show();
}


//=============================================================================
// Hardware Initialization
//=============================================================================

static void hardware_init(void)
{
    /*
     * GPIO 16 kept LOW.
     */

    gpio_config_t power_config = {
        .pin_bit_mask = 1ULL << POWER_PIN,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&power_config)
    );

    gpio_set_level(
        POWER_PIN,
        0
    );

    /*
     * Initialize LED matrix.
     */

    matrix_init();

    /*
     * Initialize SPI and IMU.
     */

    spi_init();

    imu_init();
}


//=============================================================================
// Main
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Virtual Dice"
    );

    hardware_init();

    /*
     * Start with a 1.
     */

    show_number(1);

    /*
     * Random generator is provided by ESP-IDF hardware RNG,
     * so no analogRead() seed is required.
     */

    while (1) {

        //---------------------------------------------------------------------
        // Read accelerometer.
        //---------------------------------------------------------------------

        float ax;
        float ay;
        float az;

        if (imu_read_accel(
                &ax,
                &ay,
                &az
            ) != ESP_OK) {

            ESP_LOGW(
                TAG,
                "Failed to read accelerometer"
            );

            vTaskDelay(
                pdMS_TO_TICKS(50)
            );

            continue;
        }

        //---------------------------------------------------------------------
        // Calculate tilt.
        //---------------------------------------------------------------------

        float tilt =
            calculate_tilt(
                ax,
                ay
            );

        int64_t now_us =
            esp_timer_get_time();

        //---------------------------------------------------------------------
        // Start rolling.
        //---------------------------------------------------------------------

        if (tilt > ROLL_TILT_THRESHOLD &&
            !rolling) {

            rolling = true;

            roll_start_us = now_us;
            last_roll_update_us = now_us;

            ESP_LOGI(
                TAG,
                "Dice rolling..."
            );
        }

        //---------------------------------------------------------------------
        // Rolling.
        //---------------------------------------------------------------------

        if (rolling) {

            /*
             * Change the displayed number periodically.
             */

            if (
                now_us -
                last_roll_update_us >=
                (int64_t)ROLL_UPDATE_MS * 1000
            ) {

                last_roll_update_us = now_us;

                int number =
                    (esp_random() % 6) + 1;

                show_number(number);
            }

            //-----------------------------------------------------------------
            // Check if board has become flat.
            //-----------------------------------------------------------------

            bool minimum_time_finished =
                now_us -
                roll_start_us >=
                (int64_t)MIN_ROLL_TIME_MS * 1000;

            if (
                tilt < FLAT_TILT_THRESHOLD &&
                minimum_time_finished
            ) {

                /*
                 * Final dice value.
                 */

                int final_number =
                    (esp_random() % 6) + 1;

                show_number(
                    final_number
                );

                ESP_LOGI(
                    TAG,
                    "Dice result: %d",
                    final_number
                );

                rolling = false;
            }
        }

        //---------------------------------------------------------------------
        // Run at approximately 10 ms intervals.
        //---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
