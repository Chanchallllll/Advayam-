/*
 * =============================================================================
 * WHACK-A-MOLE - ESP-IDF v5+
 * =============================================================================
 *
 * PLAYER:
 *   Tilt -> move the cursor.
 *   Button -> whack the target.
 *
 * IMU:
 *   LSM6DSM over SPI
 *
 * Physical axes:
 *
 *   Positive Y -> RIGHT
 *   Negative Y -> LEFT
 *
 *   Positive X -> DOWN
 *   Negative X -> UP
 *
 * DISPLAY:
 *   Entire matrix is rotated 180 degrees.
 *
 * HARDWARE:
 *
 *   Matrix:
 *       GPIO 7
 *
 *   Button:
 *       GPIO 17
 *
 *   Buzzer:
 *       GPIO 16
 *
 *   LSM6DSM SPI:
 *       CS   = GPIO 10
 *       SCK  = GPIO 12
 *       MOSI = GPIO 11
 *       MISO = GPIO 13
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "led_strip.h"

#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define MATRIX_PIN          7
#define NUM_PIXELS          25

#define BUTTON_PIN          17
#define BUZZER_PIN          16


/*
 * LSM6DSM SPI pins
 */

#define IMU_CS              10
#define IMU_SCK             12
#define IMU_MOSI            11
#define IMU_MISO            13


/*
 * SPI host.
 *
 * SPI2 is used so the application does not interfere with SPI0/flash.
 */

#define IMU_SPI_HOST        SPI2_HOST


/*
 * LSM6DSM registers
 */

#define LSM6DSM_CTRL1_XL    0x10

#define LSM6DSM_OUTX_L_XL   0x28
#define LSM6DSM_OUTX_H_XL   0x29

#define LSM6DSM_OUTY_L_XL   0x2A
#define LSM6DSM_OUTY_H_XL   0x2B


/*
 * Approximately 15 degrees of tilt for ±2g.
 *
 * The original Arduino program used 4240.
 */

#define TILT_THRESHOLD      4240


/*
 * Cursor movement speed while holding a tilt.
 */

#define CURSOR_MOVE_DELAY   180


/*
 * Target timeout.
 */

#define TARGET_TIME         2500


/*
 * Matrix brightness.
 */

#define MATRIX_BRIGHTNESS   10


static const char *TAG = "WHACK_A_MOLE";


/*
 * =============================================================================
 * Global Hardware Handles
 * =============================================================================
 */

static led_strip_handle_t matrix = NULL;

static spi_device_handle_t imu_spi = NULL;


/*
 * =============================================================================
 * Game State
 * =============================================================================
 */

static int cursor_x = 2;
static int cursor_y = 2;

static int target_x = 0;
static int target_y = 0;

static int score = 0;

static int64_t target_start = 0;
static int64_t last_cursor_move = 0;


/*
 * =============================================================================
 * Matrix Coordinate Mapping
 * =============================================================================
 *
 * The display is rotated 180 degrees.
 *
 * Game coordinate:
 *
 *       0   1   2   3   4
 *     +---+---+---+---+---+
 *   0 |   |   |   |   |   |
 *     +---+---+---+---+---+
 *   1 |   |   |   |   |   |
 *     +---+---+---+---+---+
 *   2 |   |   |   |   |   |
 *     +---+---+---+---+---+
 *   3 |   |   |   |   |   |
 *     +---+---+---+---+---+
 *   4 |   |   |   |   |   |
 *     +---+---+---+---+---+
 *
 *
 * Physical display is rotated 180 degrees.
 *
 * =============================================================================
 */

static int xy_to_index(int x, int y)
{
    /*
     * Rotate game coordinates 180 degrees.
     */

    int display_x = 4 - x;
    int display_y = 4 - y;


    /*
     * Matrix uses serpentine wiring.
     */

    if ((display_y % 2) == 0) {
        return display_y * 5 + display_x;
    }

    return display_y * 5 + (4 - display_x);
}


/*
 * =============================================================================
 * Set Matrix Pixel
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
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }

    int index = xy_to_index(x, y);

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
 * Draw Game
 * =============================================================================
 */

static void draw(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    /*
     * Target = RED
     */

    set_pixel(
        target_x,
        target_y,
        255,
        0,
        0
    );


    /*
     * Cursor = GREEN
     *
     * If cursor is directly over the target,
     * the cursor will overwrite the target visually.
     */

    set_pixel(
        cursor_x,
        cursor_y,
        0,
        255,
        0
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Buzzer
 * =============================================================================
 *
 * Arduino tone() is replaced with the ESP-IDF LEDC PWM peripheral.
 *
 * A short blocking tone is sufficient for this game.
 * =============================================================================
 */

static void buzzer_tone(
    uint32_t frequency,
    uint32_t duration_ms
)
{
    /*
     * LEDC configuration is done lazily here.
     *
     * This keeps the rest of the application simple.
     */

    static bool initialized = false;

    if (!initialized) {

        ledc_timer_config_t timer_config = {
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .duty_resolution = LEDC_TIMER_10_BIT,
            .timer_num = LEDC_TIMER_0,
            .freq_hz = 2000,
            .clk_cfg = LEDC_AUTO_CLK,
        };

        ESP_ERROR_CHECK(
            ledc_timer_config(&timer_config)
        );


        ledc_channel_config_t channel_config = {
            .gpio_num = BUZZER_PIN,
            .speed_mode = LEDC_LOW_SPEED_MODE,
            .channel = LEDC_CHANNEL_0,
            .intr_type = LEDC_INTR_DISABLE,
            .timer_sel = LEDC_TIMER_0,
            .duty = 0,
            .hpoint = 0,
            .flags = {
                .output_invert = 0,
            },
        };

        ESP_ERROR_CHECK(
            ledc_channel_config(&channel_config)
        );

        initialized = true;
    }


    /*
     * Change frequency.
     */

    ESP_ERROR_CHECK(
        ledc_set_freq(
            LEDC_LOW_SPEED_MODE,
            LEDC_TIMER_0,
            frequency
        )
    );


    /*
     * 50% duty cycle.
     */

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            512
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        )
    );


    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );


    /*
     * Turn buzzer off.
     */

    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            0
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
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


    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * SPI Initialization
 * =============================================================================
 */

static void imu_spi_init(void)
{
    /*
     * Configure SPI bus.
     */

    spi_bus_config_t bus_config = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 8,
    };


    ESP_ERROR_CHECK(
        spi_bus_initialize(
            IMU_SPI_HOST,
            &bus_config,
            SPI_DMA_CH_AUTO
        )
    );


    /*
     * Configure LSM6DSM.
     *
     * SPI mode 3 is used by the LSM6DSM.
     */

    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 3,
        .spics_io_num = IMU_CS,
        .queue_size = 1,
    };


    ESP_ERROR_CHECK(
        spi_bus_add_device(
            IMU_SPI_HOST,
            &device_config,
            &imu_spi
        )
    );
}


/*
 * =============================================================================
 * LSM6DSM Register Write
 * =============================================================================
 */

static esp_err_t imu_write_register(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t tx_data[2];

    /*
     * Bit 7 = 0 means WRITE.
     */

    tx_data[0] = reg & 0x7F;
    tx_data[1] = value;


    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx_data,
    };


    return spi_device_transmit(
        imu_spi,
        &transaction
    );
}


/*
 * =============================================================================
 * LSM6DSM Register Read
 * =============================================================================
 */

static esp_err_t imu_read_registers(
    uint8_t reg,
    uint8_t *data,
    size_t length
)
{
    /*
     * Bit 7 = 1 means READ.
     */

    uint8_t tx_data[8] = {0};

    if (length > 7) {
        return ESP_ERR_INVALID_SIZE;
    }


    tx_data[0] = reg | 0x80;


    spi_transaction_t transaction = {
        .length = (length + 1) * 8,
        .tx_buffer = tx_data,
        .rx_buffer = tx_data,
    };


    esp_err_t ret =
        spi_device_transmit(
            imu_spi,
            &transaction
        );


    if (ret != ESP_OK) {
        return ret;
    }


    /*
     * First byte is the register address.
     * Actual data begins at byte 1.
     */

    for (size_t i = 0; i < length; i++) {
        data[i] = tx_data[i + 1];
    }


    return ESP_OK;
}


/*
 * =============================================================================
 * Initialize LSM6DSM
 * =============================================================================
 */

static void imu_init(void)
{
    imu_spi_init();


    /*
     * CTRL1_XL:
     *
     * ODR_XL = 104 Hz
     * FS_XL  = ±2g
     *
     * 0x40 corresponds to 104 Hz / ±2g.
     */

    ESP_ERROR_CHECK(
        imu_write_register(
            LSM6DSM_CTRL1_XL,
            0x40
        )
    );


    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    ESP_LOGI(
        TAG,
        "LSM6DSM initialized"
    );
}


/*
 * =============================================================================
 * Read Accelerometer
 * =============================================================================
 */

static esp_err_t imu_read_accel(
    int16_t *ax,
    int16_t *ay
)
{
    uint8_t data[4];


    /*
     * Read:
     *
     * OUTX_L_XL
     * OUTX_H_XL
     * OUTY_L_XL
     * OUTY_H_XL
     */

    esp_err_t ret =
        imu_read_registers(
            LSM6DSM_OUTX_L_XL,
            data,
            4
        );


    if (ret != ESP_OK) {
        return ret;
    }


    /*
     * LSM6DSM sends low byte first.
     */

    *ax =
        (int16_t)(
            ((uint16_t)data[1] << 8) |
            data[0]
        );


    *ay =
        (int16_t)(
            ((uint16_t)data[3] << 8) |
            data[2]
        );


    return ESP_OK;
}


/*
 * =============================================================================
 * Read Tilt
 * =============================================================================
 */

static void read_tilt(void)
{
    int16_t ax;
    int16_t ay;


    if (imu_read_accel(&ax, &ay) != ESP_OK) {

        ESP_LOGW(
            TAG,
            "Failed to read LSM6DSM"
        );

        return;
    }


    /*
     * Ignore small movements.
     */

    if (abs(ax) < TILT_THRESHOLD &&
        abs(ay) < TILT_THRESHOLD) {

        return;
    }


    int64_t now =
        esp_timer_get_time() / 1000;


    /*
     * Cursor movement rate.
     */

    if (now - last_cursor_move <
        CURSOR_MOVE_DELAY) {

        return;
    }


    last_cursor_move = now;


    /*
     * Choose whichever axis has the stronger tilt.
     *
     * This prevents diagonal tilts from moving both axes.
     */

    if (abs(ay) > abs(ax)) {

        /*
         * Y AXIS = HORIZONTAL
         *
         * Positive Y -> LEFT
         * Negative Y -> RIGHT
         */

        if (ay < TILT_THRESHOLD) {

            if (cursor_x < 4) {

                cursor_x++;

                buzzer_tone(
                    700,
                    20
                );
            }

        }
        else if (ay > -TILT_THRESHOLD) {

            if (cursor_x > 0) {

                cursor_x--;

                buzzer_tone(
                    700,
                    20
                );
            }
        }

    }
    else {

        /*
         * X AXIS = VERTICAL
         *
         * Positive X -> DOWN
         * Negative X -> UP
         */

        if (ax > TILT_THRESHOLD) {

            if (cursor_y < 4) {

                cursor_y++;

                buzzer_tone(
                    700,
                    20
                );
            }

        }
        else if (ax < -TILT_THRESHOLD) {

            if (cursor_y > 0) {

                cursor_y--;

                buzzer_tone(
                    700,
                    20
                );
            }
        }
    }
}


/*
 * =============================================================================
 * New Target
 * =============================================================================
 */

static void new_target(void)
{
    target_x =
        esp_random() % 5;

    target_y =
        esp_random() % 5;


    target_start =
        esp_timer_get_time() / 1000;


    draw();


    ESP_LOGI(
        TAG,
        "New target: (%d, %d)",
        target_x,
        target_y
    );
}


/*
 * =============================================================================
 * Button Initialization
 * =============================================================================
 */

static void button_init(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );
}


/*
 * =============================================================================
 * Buzzer Initialization
 * =============================================================================
 */

static void buzzer_init(void)
{
    /*
     * GPIO will be configured by LEDC.
     *
     * Start with the pin LOW.
     */

    gpio_config_t buzzer_config = {
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(&buzzer_config)
    );


    gpio_set_level(
        BUZZER_PIN,
        0
    );
}


/*
 * =============================================================================
 * Main
 * =============================================================================
 */

void app_main(void)
{
    /*
     * Initialize button.
     */

    button_init();


    /*
     * Initialize buzzer.
     */

    buzzer_init();


    /*
     * Initialize matrix.
     */

    matrix_init();


    /*
     * Initialize LSM6DSM.
     */

    imu_init();


    /*
     * Seed target randomness.
     */

    uint32_t seed =
        esp_random();

    srand(seed);


    /*
     * Start game.
     */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "          WHACK-A-MOLE"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Matrix: GPIO %d",
        MATRIX_PIN
    );

    ESP_LOGI(
        TAG,
        "Button: GPIO %d",
        BUTTON_PIN
    );

    ESP_LOGI(
        TAG,
        "Buzzer: GPIO %d",
        BUZZER_PIN
    );

    ESP_LOGI(
        TAG,
        "LSM6DSM SPI: CS=%d SCK=%d MOSI=%d MISO=%d",
        IMU_CS,
        IMU_SCK,
        IMU_MOSI,
        IMU_MISO
    );


    new_target();


    /*
     * Button state.
     *
     * Pull-up means:
     *
     * HIGH = released
     * LOW  = pressed
     */

    bool old_button = true;


    /*
     * Main game loop.
     */

    while (1) {

        /*
         * =============================================================
         * TILT
         * =============================================================
         */

        read_tilt();


        /*
         * =============================================================
         * BUTTON
         * =============================================================
         */

        int button_level =
            gpio_get_level(BUTTON_PIN);

        bool button =
            (button_level == 1);


        /*
         * Detect HIGH -> LOW.
         */

        if (old_button &&
            !button) {

            /*
             * Player hit target.
             */

            if (cursor_x == target_x &&
                cursor_y == target_y) {

                score++;


                buzzer_tone(
                    1200,
                    80
                );


                ESP_LOGI(
                    TAG,
                    "HIT! Score: %d",
                    score
                );


                new_target();

            }

            /*
             * Player missed.
             */

            else {

                buzzer_tone(
                    150,
                    150
                );


                ESP_LOGI(
                    TAG,
                    "MISS!"
                );
            }
        }


        old_button = button;


        /*
         * =============================================================
         * TARGET TIMEOUT
         * =============================================================
         */

        int64_t now =
            esp_timer_get_time() / 1000;


        if (now - target_start >=
            TARGET_TIME) {

            ESP_LOGI(
                TAG,
                "Target timeout - MISS!"
            );


            buzzer_tone(
                200,
                150
            );


            new_target();
        }


        /*
         * =============================================================
         * DISPLAY
         * =============================================================
         */

        draw();


        /*
         * Equivalent to Arduino delay(10).
         */

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
