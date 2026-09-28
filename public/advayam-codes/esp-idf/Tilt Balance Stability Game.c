/*
 * =============================================================================
 * LEVEL BALANCE GAME - ESP-IDF
 * =============================================================================
 *
 * The player must keep the board level.
 *
 * CENTER LED:
 *   Green when stable
 *   Red when tilted
 *
 * WIN:
 *   Keep roll and pitch within +/-3 degrees continuously
 *   for 15 seconds.
 *
 * Hardware:
 *
 * Matrix:
 *   GPIO 7
 *
 * Power:
 *   GPIO 16
 *
 * LSM6DSM / LSM6DSOX SPI:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * =============================================================================
 */

#include <stdio.h>
#include <math.h>
#include <stdbool.h>
#include <stdint.h>

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
// GPIO Configuration
//=============================================================================

#define MATRIX_PIN      7

#define POWER_PIN       16

#define IMU_CS          10
#define IMU_SCK         12
#define IMU_MOSI        11
#define IMU_MISO        13

#define NUM_LEDS        25


//=============================================================================
// Game Configuration
//=============================================================================

#define STABLE_ANGLE    3.0f

// Must remain stable for 15 seconds.
#define STABLE_TIME_MS  15000

// Loop period.
#define LOOP_DELAY_MS   30


static const char *TAG = "LEVEL_GAME";


//=============================================================================
// Hardware Handles
//=============================================================================

static spi_device_handle_t imu_spi;

static led_strip_handle_t led_strip;


//=============================================================================
// Game State
//=============================================================================

static bool timing = false;

static int64_t stable_start_us = 0;


//=============================================================================
// Matrix Mapping
//=============================================================================

/*
 * Matrix wiring is serpentine:
 *
 * Row 0:  0  1  2  3  4
 * Row 1:  9  8  7  6  5
 * Row 2: 10 11 12 13 14
 * Row 3: 19 18 17 16 15
 * Row 4: 20 21 22 23 24
 */

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


//=============================================================================
// Matrix Functions
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

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


static void matrix_set_xy(
    int x,
    int y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }

    int index = pixel_xy(x, y);

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            index,
            r,
            g,
            b
        )
    );
}


//=============================================================================
// Draw Game
//=============================================================================

static void draw_stable(void)
{
    matrix_clear();

    /*
     * Center LED = pixel 12.
     *
     * Green = board is level.
     */

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            12,
            0,
            255,
            0
        )
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


static void draw_unstable(void)
{
    matrix_clear();

    /*
     * Center LED = red when tilted.
     */

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            12,
            255,
            0,
            0
        )
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


static void draw_success(void)
{
    for (int i = 0; i < NUM_LEDS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                0,
                255,
                0
            )
        );
    }

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// LSM6DSM / LSM6DSOX SPI
//=============================================================================

/*
 * LSM6DSM register addresses:
 *
 * WHO_AM_I = 0x0F
 * CTRL1_XL = 0x10
 * OUTX_L_XL = 0x28
 *
 * SPI read:
 *   register | 0x80
 *
 * SPI write:
 *   register & 0x7F
 *
 * Accelerometer configured for:
 *
 *   ODR = 104 Hz
 *   Full scale = +/-4g
 */

static esp_err_t imu_write_reg(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t tx[2];

    tx[0] = reg & 0x7F;
    tx[1] = value;

    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx,
    };

    return spi_device_transmit(
        imu_spi,
        &t
    );
}


static esp_err_t imu_read_regs(
    uint8_t reg,
    uint8_t *data,
    size_t length
)
{
    /*
     * First byte is the register address.
     * The remaining bytes are clocked out by the sensor.
     */

    uint8_t tx[length + 1];
    uint8_t rx[length + 1];

    tx[0] = reg | 0x80;

    for (size_t i = 1; i <= length; i++) {
        tx[i] = 0;
    }

    spi_transaction_t t = {
        .length = (length + 1) * 8,
        .tx_buffer = tx,
        .rx_buffer = rx,
    };

    esp_err_t ret =
        spi_device_transmit(
            imu_spi,
            &t
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
    /*
     * Read WHO_AM_I.
     */

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
     * CTRL1_XL = 0x40
     *
     * ODR_XL = 104 Hz
     * FS_XL  = +/-2g
     *
     * For +/-4g use:
     *
     * 0x48
     */

    ESP_ERROR_CHECK(
        imu_write_reg(
            0x10,
            0x48
        )
    );

    ESP_LOGI(
        TAG,
        "IMU initialized"
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
     * +/-4g sensitivity:
     *
     * 0.122 mg/LSB
     *
     * Convert to m/s².
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

static void calculate_tilt(
    float ax,
    float ay,
    float az,
    float *roll,
    float *pitch
)
{
    /*
     * Same equations as the Arduino program.
     */

    *roll =
        atan2f(
            -ay,
            az
        ) * 180.0f / (float)M_PI;

    *pitch =
        atan2f(
            ax,
            sqrtf(
                ay * ay +
                az * az
            )
        ) * 180.0f / (float)M_PI;
}


//=============================================================================
// SPI Initialization
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

    spi_device_interface_config_t dev_config = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 3,
        .spics_io_num = IMU_CS,
        .queue_size = 1,
    };

    ESP_ERROR_CHECK(
        spi_bus_add_device(
            SPI2_HOST,
            &dev_config,
            &imu_spi
        )
    );
}


//=============================================================================
// LED Matrix Initialization
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
}


//=============================================================================
// Setup
//=============================================================================

static void hardware_init(void)
{
    /*
     * GPIO 16.
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
     * Matrix.
     */

    matrix_init();

    /*
     * SPI IMU.
     */

    spi_init();

    imu_init();
}


//=============================================================================
// Main Application
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Level Balance Game"
    );

    ESP_LOGI(
        TAG,
        "Keep the board level for 15 seconds."
    );

    hardware_init();

    draw_unstable();

    while (1) {

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
                pdMS_TO_TICKS(100)
            );

            continue;
        }

        //---------------------------------------------------------------------
        // Calculate roll and pitch.
        //---------------------------------------------------------------------

        float roll;
        float pitch;

        calculate_tilt(
            ax,
            ay,
            az,
            &roll,
            &pitch
        );

        //---------------------------------------------------------------------
        // Determine whether board is level.
        //---------------------------------------------------------------------

        bool stable =
            fabsf(roll) < STABLE_ANGLE &&
            fabsf(pitch) < STABLE_ANGLE;

        //---------------------------------------------------------------------
        // Stable
        //---------------------------------------------------------------------

        if (stable) {

            if (!timing) {

                stable_start_us =
                    esp_timer_get_time();

                timing = true;

                ESP_LOGI(
                    TAG,
                    "Stable - timer started"
                );
            }

            draw_stable();

            //-----------------------------------------------------------------
            // Check 15-second requirement.
            //-----------------------------------------------------------------

            int64_t elapsed_us =
                esp_timer_get_time() -
                stable_start_us;

            if (elapsed_us >=
                (int64_t)STABLE_TIME_MS * 1000) {

                ESP_LOGI(
                    TAG,
                    "SUCCESS!"
                );

                draw_success();

                vTaskDelay(
                    pdMS_TO_TICKS(1000)
                );

                /*
                 * Start another round.
                 */

                timing = false;

                draw_unstable();
            }
        }

        //---------------------------------------------------------------------
        // Unstable
        //---------------------------------------------------------------------

        else {

            if (timing) {

                ESP_LOGI(
                    TAG,
                    "Tilt detected - timer reset"
                );
            }

            timing = false;

            draw_unstable();
        }

        //---------------------------------------------------------------------
        // Debug output.
        //---------------------------------------------------------------------

        ESP_LOGI(
            TAG,
            "Roll: %.2f deg | Pitch: %.2f deg | %s",
            roll,
            pitch,
            stable ? "STABLE" : "UNSTABLE"
        );

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
