/*
 * =============================================================================
 * SPACE INVADERS MINI - ESP-IDF v6.0.2
 * =============================================================================
 *
 * ESP32-S3
 *
 * 5x5 Space Invaders game.
 *
 * CONTROLS:
 *   Tilt left/right -> move spaceship.
 *   Button          -> fire.
 *
 * IMU:
 *   Physical Y axis controls horizontal movement.
 *   Positive Y -> RIGHT
 *   Negative Y -> LEFT
 *   Tilt threshold = approximately 15 degrees.
 *
 * DISPLAY:
 *   Entire game is rotated 180 degrees.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Buzzer = GPIO 16
 *
 * IMU:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * ESP-IDF:
 *   v6.0.2
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

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"

#include "led_strip.h"


//=============================================================================
// Pins
//=============================================================================

#define MATRIX_PIN      7
#define BUTTON_PIN      17
#define BUZZER_PIN      16

#define IMU_CS          10
#define IMU_SCK         12
#define IMU_MOSI        11
#define IMU_MISO        13

#define NUM_PIXELS      25

// Approximately 15 degrees for ±2g.
#define TILT_THRESHOLD  4240

// How quickly the player moves while holding a tilt.
#define PLAYER_MOVE_DELAY   180

// Game speeds.
#define BULLET_MOVE_DELAY   100
#define ENEMY_MOVE_DELAY    700


//=============================================================================
// LED Strip
//=============================================================================

static const char *TAG = "SPACE_INVADERS";

static led_strip_handle_t led_strip = NULL;


//=============================================================================
// SPI / IMU
//=============================================================================

static spi_device_handle_t imu_spi = NULL;


//=============================================================================
// Buzzer
//=============================================================================

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_DUTY        2048
#define BUZZER_LEDC_RESOLUTION  LEDC_TIMER_13_BIT


//=============================================================================
// Game State
//=============================================================================

static int playerX = 2;

static int enemyX = 2;
static int enemyY = 0;

static int bulletX = -1;
static int bulletY = -1;

static bool gameOver = false;


//=============================================================================
// Timing
//=============================================================================

static uint64_t lastPlayerMove = 0;
static uint64_t lastBulletMove = 0;
static uint64_t lastEnemyMove = 0;


//=============================================================================
// Utility
//=============================================================================

static uint64_t millis(void)
{
    return (uint64_t)(esp_timer_get_time() / 1000ULL);
}


//=============================================================================
// Buzzer
//=============================================================================

static void buzzer_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode       = BUZZER_LEDC_MODE,
        .timer_num        = BUZZER_LEDC_TIMER,
        .duty_resolution  = BUZZER_LEDC_RESOLUTION,
        .freq_hz          = 1000,
        .clk_cfg          = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(ledc_timer_config(&timer_config));

    ledc_channel_config_t channel_config = {
        .gpio_num       = BUZZER_PIN,
        .speed_mode     = BUZZER_LEDC_MODE,
        .channel        = BUZZER_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = BUZZER_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .flags.output_invert = 0,
    };

    ESP_ERROR_CHECK(ledc_channel_config(&channel_config));
}


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
            BUZZER_LEDC_DUTY
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_CHANNEL
        )
    );

    vTaskDelay(pdMS_TO_TICKS(duration_ms));

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
// Matrix
//=============================================================================

/*
 * Game coordinates:
 *
 *      y = 0
 *
 *      0 1 2 3 4
 *
 *      ...
 *
 *      y = 4
 *
 *
 * Display is rotated 180 degrees.
 */

static int xy_to_index(int x, int y)
{
    int displayX = 4 - x;
    int displayY = 4 - y;

    if ((displayY % 2) == 0) {
        return displayY * 5 + displayX;
    }

    return displayY * 5 + (4 - displayX);
}


static void set_pixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (x < 0 || x > 4 ||
        y < 0 || y > 4) {
        return;
    }

    int index = xy_to_index(x, y);

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


//=============================================================================
// Clear Matrix
//=============================================================================

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


//=============================================================================
// Draw Game
//=============================================================================

static void draw_game(void)
{
    clear_matrix();

    //------------------------------------------------------------------------
    // Player
    //------------------------------------------------------------------------

    set_pixel(
        playerX,
        4,
        0,
        255,
        0
    );

    //------------------------------------------------------------------------
    // Enemy
    //------------------------------------------------------------------------

    if (enemyY >= 0 && enemyY <= 4) {

        set_pixel(
            enemyX,
            enemyY,
            255,
            0,
            0
        );
    }

    //------------------------------------------------------------------------
    // Bullet
    //------------------------------------------------------------------------

    if (bulletY >= 0 && bulletY <= 4) {

        set_pixel(
            bulletX,
            bulletY,
            255,
            255,
            255
        );
    }

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// IMU
//=============================================================================

static void imu_write(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t tx_data[2];

    tx_data[0] = reg & 0x7F;
    tx_data[1] = value;

    spi_transaction_t transaction = {
        .length = 16,
        .tx_buffer = tx_data,
    };

    ESP_ERROR_CHECK(
        spi_device_transmit(
            imu_spi,
            &transaction
        )
    );
}


static int16_t imu_read16(uint8_t reg)
{
    uint8_t tx_data[3] = {
        (uint8_t)(reg | 0x80),
        0,
        0
    };

    uint8_t rx_data[3] = {
        0,
        0,
        0
    };

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

    uint8_t lowByte = rx_data[1];
    uint8_t highByte = rx_data[2];

    return (int16_t)(
        ((uint16_t)highByte << 8) |
        lowByte
    );
}


static void setup_imu(void)
{
    //------------------------------------------------------------------------
    // CS GPIO
    //------------------------------------------------------------------------

    gpio_config_t cs_config = {
        .pin_bit_mask = (1ULL << IMU_CS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&cs_config)
    );

    ESP_ERROR_CHECK(
        gpio_set_level(IMU_CS, 1)
    );

    //------------------------------------------------------------------------
    // SPI Bus
    //------------------------------------------------------------------------

    spi_bus_config_t bus_config = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .data4_io_num = -1,
        .data5_io_num = -1,
        .data6_io_num = -1,
        .data7_io_num = -1,
        .max_transfer_sz = 32,
    };

    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST,
            &bus_config,
            SPI_DMA_DISABLED
        )
    );

    //------------------------------------------------------------------------
    // IMU device
    //------------------------------------------------------------------------

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

    //------------------------------------------------------------------------
    // LSM6DSM configuration
    //
    // 104 Hz
    // ±2g
    //------------------------------------------------------------------------

    imu_write(0x10, 0x40);

    ESP_LOGI(TAG, "IMU initialized");
}


//=============================================================================
// Player Tilt Control
//=============================================================================

static void update_player(void)
{
    int16_t ay = imu_read16(0x2A);

    //------------------------------------------------------------------------
    // Ignore small tilts.
    //------------------------------------------------------------------------

    if (abs(ay) < TILT_THRESHOLD) {
        return;
    }

    //------------------------------------------------------------------------
    // Limit movement speed.
    //------------------------------------------------------------------------

    uint64_t now = millis();

    if (now - lastPlayerMove < PLAYER_MOVE_DELAY) {
        return;
    }

    lastPlayerMove = now;

    /*
     * Physical Y axis:
     *
     * Positive Y -> RIGHT
     * Negative Y -> LEFT
     */

    if (ay < TILT_THRESHOLD) {

        if (playerX < 4) {
            playerX++;

            buzzer_tone(700, 25);
        }
    }
    else if (ay > -TILT_THRESHOLD) {

        if (playerX > 0) {
            playerX--;

            buzzer_tone(700, 25);
        }
    }
}


//=============================================================================
// Fire
//=============================================================================

static void fire_bullet(void)
{
    // Only one bullet at a time.
    if (bulletY != -1) {
        return;
    }

    bulletX = playerX;
    bulletY = 3;

    buzzer_tone(1500, 50);
}


//=============================================================================
// Bullet Collision
//=============================================================================

static bool bullet_hits_enemy(
    int oldY,
    int newY
)
{
    // X must match.
    if (bulletX != enemyX) {
        return false;
    }

    /*
     * Check whether the bullet crossed the enemy.
     */

    if (oldY >= enemyY &&
        newY <= enemyY) {

        return true;
    }

    return false;
}


//=============================================================================
// Bullet Update
//=============================================================================

static void update_bullet(void)
{
    if (bulletY == -1) {
        return;
    }

    int oldY = bulletY;

    // Move bullet upward.
    bulletY--;

    //------------------------------------------------------------------------
    // Collision
    //------------------------------------------------------------------------

    if (bullet_hits_enemy(oldY, bulletY)) {

        buzzer_tone(2000, 100);

        // Remove bullet.
        bulletX = -1;
        bulletY = -1;

        // Respawn enemy.
        enemyX =
            esp_random() % 5;

        enemyY = 0;

        return;
    }

    //------------------------------------------------------------------------
    // Bullet left matrix
    //------------------------------------------------------------------------

    if (bulletY < 0) {

        bulletX = -1;
        bulletY = -1;
    }
}


//=============================================================================
// Enemy Update
//=============================================================================

static void update_enemy(void)
{
    enemyY++;

    //------------------------------------------------------------------------
    // Enemy reaches player's row.
    //------------------------------------------------------------------------

    if (enemyY >= 4) {

        if (enemyX == playerX) {

            gameOver = true;

            clear_matrix();

            // Flash entire matrix red.
            for (int i = 0; i < NUM_PIXELS; i++) {

                ESP_ERROR_CHECK(
                    led_strip_set_pixel(
                        led_strip,
                        i,
                        255,
                        0,
                        0
                    )
                );
            }

            ESP_ERROR_CHECK(
                led_strip_refresh(led_strip)
            );

            buzzer_tone(150, 500);

            vTaskDelay(pdMS_TO_TICKS(1000));

            return;
        }

        //------------------------------------------------------------------------
        // Enemy missed player.
        //------------------------------------------------------------------------

        enemyX =
            esp_random() % 5;

        enemyY = 0;
    }
}


//=============================================================================
// Reset Game
//=============================================================================

static void reset_game(void)
{
    playerX = 2;

    enemyX =
        esp_random() % 5;

    enemyY = 0;

    bulletX = -1;
    bulletY = -1;

    gameOver = false;

    uint64_t now = millis();

    lastPlayerMove = now;
    lastBulletMove = now;
    lastEnemyMove = now;

    draw_game();
}


//=============================================================================
// Matrix Initialization
//=============================================================================

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
        .mem_block_symbols = 0,
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

    ESP_LOGI(TAG, "RGB matrix initialized");
}


//=============================================================================
// Button Initialization
//=============================================================================

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


//=============================================================================
// Setup
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "=== SPACE INVADERS MINI ==="
    );

    //------------------------------------------------------------------------
    // Button
    //------------------------------------------------------------------------

    button_init();

    //------------------------------------------------------------------------
    // Buzzer
    //------------------------------------------------------------------------

    buzzer_init();

    //------------------------------------------------------------------------
    // Matrix
    //------------------------------------------------------------------------

    matrix_init();

    //------------------------------------------------------------------------
    // Brightness
    //------------------------------------------------------------------------

    // ESP-IDF led_strip doesn't provide the same global
    // setBrightness() function as Adafruit_NeoPixel.
    //
    // We therefore keep RGB values deliberately low enough
    // for the desired brightness.
    //
    // The colors below are currently full RGB values, so if
    // you want the same approximate brightness as the Arduino
    // version, we can add software brightness scaling.
    //------------------------------------------------------------------------

    //------------------------------------------------------------------------
    // IMU
    //------------------------------------------------------------------------

    setup_imu();

    //------------------------------------------------------------------------
    // Initial game
    //------------------------------------------------------------------------

    reset_game();

    ESP_LOGI(
        TAG,
        "Space Invaders started."
    );

    //------------------------------------------------------------------------
    // Main game loop
    //------------------------------------------------------------------------

    bool previousButton = true;

    while (1) {

        //=====================================================================
        // Game Over
        //=====================================================================

        if (gameOver) {

            bool buttonState =
                gpio_get_level(BUTTON_PIN);

            if (buttonState == 0) {

                vTaskDelay(
                    pdMS_TO_TICKS(200)
                );

                // Wait for button release.
                while (
                    gpio_get_level(BUTTON_PIN) == 0
                ) {
                    vTaskDelay(
                        pdMS_TO_TICKS(10)
                    );
                }

                reset_game();
            }

            vTaskDelay(
                pdMS_TO_TICKS(20)
            );

            continue;
        }

        //=====================================================================
        // Player
        //=====================================================================

        update_player();

        //=====================================================================
        // Button
        //=====================================================================

        bool buttonState =
            gpio_get_level(BUTTON_PIN);

        if (buttonState == 0 &&
            previousButton == true) {

            fire_bullet();
        }

        previousButton = buttonState;

        //=====================================================================
        // Bullet
        //=====================================================================

        uint64_t now = millis();

        if (now - lastBulletMove >= BULLET_MOVE_DELAY) {

            lastBulletMove = now;

            update_bullet();
        }

        //=====================================================================
        // Enemy
        //=====================================================================

        now = millis();

        if (now - lastEnemyMove >= ENEMY_MOVE_DELAY) {

            lastEnemyMove = now;

            update_enemy();
        }

        //=====================================================================
        // Display
        //=====================================================================

        draw_game();

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
