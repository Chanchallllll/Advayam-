/*
 * =============================================================================
 * 5x5 PONG AGAINST AI - ESP-IDF v5/v6
 * =============================================================================
 *
 * ESP32-S3
 *
 * PLAYER:
 *   Tilt left/right to move the paddle on the bottom row.
 *
 * IMU:
 *   LSM6DSM
 *   SPI
 *
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 *   Physical Y axis -> horizontal paddle movement
 *   Positive Y -> right
 *   Negative Y -> left
 *   Threshold ~= 15 degrees
 *
 * RGB MATRIX:
 *   WS2812 / NeoPixel
 *   Data = GPIO 7
 *   25 pixels
 *
 * BUTTON:
 *   GPIO 17
 *   Active LOW
 *
 * BUZZER:
 *   GPIO 16
 *   LEDC PWM
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"

#include "esp_log.h"
#include "esp_random.h"
#include "esp_err.h"
#include "esp_timer.h"

#include "led_strip.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN       7
#define NUM_LEDS         25

#define BUTTON_PIN       17
#define BUZZER_PIN       16

#define IMU_CS           10
#define IMU_SCK          12
#define IMU_MOSI         11
#define IMU_MISO         13


//=============================================================================
// IMU
//=============================================================================

// Approximately sin(15°) * 16384 for ±2g.
#define TILT_THRESHOLD   4240


//=============================================================================
// Game timing
//=============================================================================

#define BALL_MOVE_INTERVAL_MS   300
#define LOOP_DELAY_MS           20


//=============================================================================
// Buzzer
//=============================================================================

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0

#define BUZZER_DUTY             512
#define BUZZER_RESOLUTION       LEDC_TIMER_10_BIT


//=============================================================================
// Logging
//=============================================================================

static const char *TAG = "PONG";


//=============================================================================
// LED Strip
//=============================================================================

static led_strip_handle_t led_strip = NULL;


//=============================================================================
// SPI
//=============================================================================

static spi_device_handle_t imu_spi = NULL;


//=============================================================================
// Game state
//=============================================================================

static int ball_x = 2;
static int ball_y = 3;

static int ball_dx = 1;
static int ball_dy = -1;

static int player_x = 2;
static int cpu_x = 2;

static int64_t last_move_time = 0;

static bool game_over = false;

// Used so one tilt only moves the paddle one position.
static bool tilt_ready = true;


//=============================================================================
// Matrix Coordinate Mapping
//=============================================================================

static int xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


//=============================================================================
// Set Pixel
//=============================================================================

static void set_pixel(int x, int y, uint32_t color)
{
    if (x >= 0 && x < 5 &&
        y >= 0 && y < 5) {

        int index = xy(x, y);

        uint8_t r = (color >> 16) & 0xFF;
        uint8_t g = (color >> 8) & 0xFF;
        uint8_t b = color & 0xFF;

        led_strip_set_pixel(
            led_strip,
            index,
            r,
            g,
            b
        );
    }
}


//=============================================================================
// Matrix Clear
//=============================================================================

static void matrix_clear(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


//=============================================================================
// Draw Game
//=============================================================================

static void draw(void)
{
    matrix_clear();

    //-------------------------------------------------------------------------
    // Ball
    //-------------------------------------------------------------------------

    set_pixel(
        ball_x,
        ball_y,
        0xFFFFFF
    );


    //-------------------------------------------------------------------------
    // CPU Paddle
    //-------------------------------------------------------------------------

    set_pixel(
        cpu_x,
        0,
        0xFF0000
    );

    if (cpu_x > 0) {
        set_pixel(
            cpu_x - 1,
            0,
            0xFF0000
        );
    }

    if (cpu_x < 4) {
        set_pixel(
            cpu_x + 1,
            0,
            0xFF0000
        );
    }


    //-------------------------------------------------------------------------
    // Player Paddle
    //-------------------------------------------------------------------------

    set_pixel(
        player_x,
        4,
        0x00FF00
    );

    if (player_x > 0) {
        set_pixel(
            player_x - 1,
            4,
            0x00FF00
        );
    }

    if (player_x < 4) {
        set_pixel(
            player_x + 1,
            4,
            0x00FF00
        );
    }


    //-------------------------------------------------------------------------
    // Send to LEDs
    //-------------------------------------------------------------------------

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
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
        .flags = {
            .invert_out = false,
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
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

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


//=============================================================================
// Buzzer
//=============================================================================

static void buzzer_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_LEDC_MODE,
        .duty_resolution = BUZZER_RESOLUTION,
        .timer_num = BUZZER_LEDC_TIMER,
        .freq_hz = 1000,
        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = BUZZER_LEDC_MODE,
        .channel = BUZZER_LEDC_CHANNEL,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = BUZZER_LEDC_TIMER,
        .duty = 0,
        .hpoint = 0,
        .flags = {
            .output_invert = 0,
        }
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );
}


//=============================================================================
// Buzzer Tone
//=============================================================================

static void buzzer_tone(uint32_t frequency, uint32_t duration_ms)
{
    if (frequency == 0) {
        return;
    }

    ESP_ERROR_CHECK(
        ledc_set_freq(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_TIMER,
            frequency
        )
    );

    ESP_ERROR_CHECK(
        ledc_set_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL,
            BUZZER_DUTY
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL
        )
    );

    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );

    ESP_ERROR_CHECK(
        ledc_set_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL,
            0
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL
        )
    );
}


//=============================================================================
// IMU SPI
//=============================================================================

static void imu_write(uint8_t reg, uint8_t value)
{
    uint8_t tx_data[2];

    /*
     * LSM6DSM SPI write:
     *
     * Bit 7 = 0 -> write
     * Bits 6:0 = register
     */

    tx_data[0] = reg & 0x7F;
    tx_data[1] = value;

    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx_data,
        .rx_buffer = NULL,
    };

    ESP_ERROR_CHECK(
        spi_device_transmit(
            imu_spi,
            &transaction
        )
    );
}


//=============================================================================
// Read 16-bit IMU register
//=============================================================================

static int16_t imu_read16(uint8_t reg)
{
    uint8_t tx_data[3] = {
        (uint8_t)(reg | 0x80),
        0x00,
        0x00
    };

    uint8_t rx_data[3] = {
        0
    };

    /*
     * LSM6DSM:
     *
     * First byte = register address
     * Following bytes = register data
     *
     * Low byte first
     * High byte second
     */

    spi_transaction_t transaction = {
        .length = 24,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };

    ESP_ERROR_CHECK(
        spi_device_transmit(
            imu_spi,
            &transaction
        )
    );

    uint8_t low = rx_data[1];
    uint8_t high = rx_data[2];

    return (int16_t)(
        ((uint16_t)high << 8) |
        low
    );
}


//=============================================================================
// Initialize IMU
//=============================================================================

static void imu_init(void)
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
        .clock_speed_hz = 1000000,
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


    /*
     * LSM6DSM CTRL1_XL (0x10)
     *
     * 0x40:
     *
     * ODR = 104 Hz
     * FS  = ±2g
     * Normal operating mode
     */

    imu_write(
        0x10,
        0x40
    );

    ESP_LOGI(
        TAG,
        "LSM6DSM initialized"
    );
}


//=============================================================================
// Read Tilt
//=============================================================================

static void read_tilt(void)
{
    int16_t ax = imu_read16(0x28);
    int16_t ay = imu_read16(0x2A);

    (void)ax;


    /*
     * Physical Y -> horizontal paddle movement.
     *
     * Physical Y is NOT inverted.
     *
     * Positive Y -> right
     * Negative Y -> left
     */


    //-------------------------------------------------------------------------
    // Below tilt threshold
    //-------------------------------------------------------------------------

    if (abs(ay) < TILT_THRESHOLD) {

        tilt_ready = true;

        return;
    }


    //-------------------------------------------------------------------------
    // Register only one movement per tilt
    //-------------------------------------------------------------------------

    if (tilt_ready) {

        if (ay > 0 && player_x < 4) {

            player_x++;

            ESP_LOGI(
                TAG,
                "Player paddle -> RIGHT (%d)",
                player_x
            );
        }

        else if (ay < 0 && player_x > 0) {

            player_x--;

            ESP_LOGI(
                TAG,
                "Player paddle -> LEFT (%d)",
                player_x
            );
        }

        tilt_ready = false;
    }
}


//=============================================================================
// Reset Game
//=============================================================================

static void reset_game(void)
{
    ball_x = 2;
    ball_y = 3;

    /*
     * Random initial horizontal direction.
     */

    ball_dx =
        (esp_random() % 2)
        ? 1
        : -1;

    ball_dy = -1;

    player_x = 2;
    cpu_x = 2;

    game_over = false;

    /*
     * Allow first tilt immediately.
     */

    tilt_ready = true;

    draw();

    ESP_LOGI(
        TAG,
        "Game started"
    );
}


//=============================================================================
// Lose Animation
//=============================================================================

static void lose(void)
{
    game_over = true;

    ESP_LOGI(
        TAG,
        "GAME OVER"
    );


    //-------------------------------------------------------------------------
    // Flash the entire matrix red three times
    //-------------------------------------------------------------------------

    for (int i = 0; i < 3; i++) {

        matrix_clear();

        ESP_ERROR_CHECK(
            led_strip_refresh(led_strip)
        );

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );


        for (int j = 0; j < NUM_LEDS; j++) {

            led_strip_set_pixel(
                led_strip,
                j,
                255,
                0,
                0
            );
        }

        ESP_ERROR_CHECK(
            led_strip_refresh(led_strip)
        );

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }


    /*
     * Arduino:
     *
     * tone(BUZZER_PIN, 150, 500);
     *
     * ESP-IDF equivalent.
     */

    buzzer_tone(
        150,
        500
    );
}


//=============================================================================
// Update Ball
//=============================================================================

static void update_ball(void)
{
    int next_x = ball_x + ball_dx;
    int next_y = ball_y + ball_dy;


    //-------------------------------------------------------------------------
    // Side wall collision
    //-------------------------------------------------------------------------

    if (next_x < 0 || next_x > 4) {

        ball_dx = -ball_dx;

        next_x = ball_x + ball_dx;
    }


    //-------------------------------------------------------------------------
    // CPU Paddle
    //-------------------------------------------------------------------------

    if (next_y == 0) {

        if (abs(next_x - cpu_x) <= 1) {

            ball_dy = 1;

            buzzer_tone(
                800,
                40
            );

            next_y = 1;

        } else {

            lose();

            return;
        }
    }


    //-------------------------------------------------------------------------
    // Player Paddle
    //-------------------------------------------------------------------------

    if (next_y == 4) {

        if (abs(next_x - player_x) <= 1) {

            ball_dy = -1;

            buzzer_tone(
                1000,
                40
            );

            next_y = 3;

        } else {

            lose();

            return;
        }
    }


    //-------------------------------------------------------------------------
    // Apply new position
    //-------------------------------------------------------------------------

    ball_x = next_x;
    ball_y = next_y;


    //-------------------------------------------------------------------------
    // Basic CPU AI
    //-------------------------------------------------------------------------

    if (cpu_x < ball_x) {

        cpu_x++;

    } else if (cpu_x > ball_x) {

        cpu_x--;
    }
}


//=============================================================================
// Button Initialization
//=============================================================================

static void button_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&config)
    );
}


//=============================================================================
// Main
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       5x5 PONG AGAINST AI"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    //-------------------------------------------------------------------------
    // Initialize hardware
    //-------------------------------------------------------------------------

    button_init();

    matrix_init();

    buzzer_init();

    imu_init();


    //-------------------------------------------------------------------------
    // Start game
    //-------------------------------------------------------------------------

    reset_game();


    //-------------------------------------------------------------------------
    // Main game loop
    //-------------------------------------------------------------------------

    while (1) {

        //=====================================================================
        // GAME OVER
        //=====================================================================

        if (game_over) {

            /*
             * Button is active LOW.
             */

            if (gpio_get_level(BUTTON_PIN) == 0) {

                /*
                 * Simple debounce.
                 */

                vTaskDelay(
                    pdMS_TO_TICKS(250)
                );

                reset_game();
            }

            vTaskDelay(
                pdMS_TO_TICKS(20)
            );

            continue;
        }


        //=====================================================================
        // Read IMU
        //=====================================================================

        read_tilt();


        //=====================================================================
        // Move ball
        //=====================================================================

        int64_t now =
            esp_timer_get_time() / 1000;

        if ((now - last_move_time) >=
            BALL_MOVE_INTERVAL_MS) {

            last_move_time = now;

            update_ball();

            if (!game_over) {
                draw();
            }
        }


        //=====================================================================
        // Small delay
        //=====================================================================

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
