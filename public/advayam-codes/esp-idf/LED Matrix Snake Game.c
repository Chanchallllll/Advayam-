/*
 * =============================================================================
 * MATRIX SNAKE GAME - ESP-IDF
 * =============================================================================
 *
 * 5x5 WS2812B Snake game.
 *
 * Matrix DATA = GPIO 7
 * Button      = GPIO 17
 * IMU CS      = GPIO 10
 * IMU SCK     = GPIO 12
 * IMU MOSI    = GPIO 11
 * IMU MISO    = GPIO 13
 * Buzzer      = GPIO 16
 *
 * Brightness = 5%
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/spi_master.h"
#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_timer.h"
#include "esp_random.h"

#include "led_strip.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN  7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define NUM_PIXELS 25

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13


//=============================================================================
// Game
//=============================================================================

#define TILT_THRESHOLD 4240

#define MOVE_DELAY 350

#define MATRIX_BRIGHTNESS 5


static const char *TAG = "snake";


//=============================================================================
// LSM6DSOX Registers
//=============================================================================

#define LSM6DSOX_WHO_AM_I       0x0F
#define LSM6DSOX_CTRL1_XL       0x10
#define LSM6DSOX_OUTX_L_A       0x28

#define LSM6DSOX_WHO_AM_I_VAL   0x6C


//=============================================================================
// Handles
//=============================================================================

static spi_device_handle_t imu_spi = NULL;
static led_strip_handle_t led_strip = NULL;


//=============================================================================
// Snake State
//=============================================================================

static int snakeX[25];
static int snakeY[25];

static int snakeLength;

static int appleX;
static int appleY;

static int dirX = 1;
static int dirY = 0;

static bool gameOver = false;

static int64_t lastMove;


//=============================================================================
// Brightness
//=============================================================================

static uint8_t scaleBrightness(uint8_t value)
{
    return (uint8_t)(
        ((uint16_t)value * MATRIX_BRIGHTNESS) / 100
    );
}


//=============================================================================
// Matrix
//=============================================================================

static int xyToIndex(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}


static void pixel(
    int x,
    int y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (x < 0 || x > 4 || y < 0 || y > 4)
        return;

    r = scaleBrightness(r);
    g = scaleBrightness(g);
    b = scaleBrightness(b);

    led_strip_set_pixel(
        led_strip,
        xyToIndex(x, y),
        r,
        g,
        b
    );
}


static void matrixClear(void)
{
    led_strip_clear(led_strip);
}


static void matrixShow(void)
{
    led_strip_refresh(led_strip);
}


//=============================================================================
// Draw Game
//=============================================================================

static void drawGame(void)
{
    matrixClear();


    //-------------------------------------------------------------------------
    // Apple
    //-------------------------------------------------------------------------

    pixel(
        appleX,
        appleY,
        255,
        0,
        0
    );


    //-------------------------------------------------------------------------
    // Snake
    //-------------------------------------------------------------------------

    for (int i = 0; i < snakeLength; i++)
    {
        if (i == 0)
        {
            // Head
            pixel(
                snakeX[i],
                snakeY[i],
                237,
                187,
                69
            );
        }
        else
        {
            // Body
            pixel(
                snakeX[i],
                snakeY[i],
                0,
                255,
                0
            );
        }
    }


    matrixShow();
}


//=============================================================================
// Buzzer
//=============================================================================

/*
 * Simple GPIO buzzer.
 *
 * This avoids driver/ledc.h completely and therefore works with
 * ESP-IDF versions where the LEDC header/API differs.
 *
 * It generates a square wave by manually toggling the GPIO.
 */

static void buzzerTone(
    uint32_t frequency,
    uint32_t duration_ms
)
{
    if (frequency == 0)
        return;


    uint32_t half_period_us =
        1000000UL / (frequency * 2);


    uint32_t total_us =
        duration_ms * 1000UL;


    uint32_t elapsed = 0;


    while (elapsed < total_us)
    {
        gpio_set_level(
            BUZZER_PIN,
            1
        );


        esp_rom_delay_us(
            half_period_us
        );


        gpio_set_level(
            BUZZER_PIN,
            0
        );


        esp_rom_delay_us(
            half_period_us
        );


        elapsed +=
            half_period_us * 2;
    }


    gpio_set_level(
        BUZZER_PIN,
        0
    );
}


//=============================================================================
// Startup Animation
//=============================================================================

static void startupFlash(void)
{
    //-------------------------------------------------------------------------
    // First flash
    //-------------------------------------------------------------------------

    drawGame();

    buzzerTone(
        1200,
        60
    );

    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    //-------------------------------------------------------------------------
    // OFF
    //-------------------------------------------------------------------------

    matrixClear();
    matrixShow();

    vTaskDelay(
        pdMS_TO_TICKS(170)
    );


    //-------------------------------------------------------------------------
    // Second flash
    //-------------------------------------------------------------------------

    drawGame();

    buzzerTone(
        1200,
        60
    );

    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    //-------------------------------------------------------------------------
    // OFF
    //-------------------------------------------------------------------------

    matrixClear();
    matrixShow();

    vTaskDelay(
        pdMS_TO_TICKS(170)
    );
}


//=============================================================================
// LSM6DSOX SPI Read
//=============================================================================

static esp_err_t lsm6dsox_read_reg(
    uint8_t reg,
    uint8_t *data,
    size_t size
)
{
    uint8_t tx_data[2 + size];
    uint8_t rx_data[2 + size];


    tx_data[0] =
        reg | 0x80;


    memset(
        &tx_data[1],
        0,
        size
    );


    spi_transaction_t t = {
        .length = (1 + size) * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };


    gpio_set_level(
        IMU_CS,
        0
    );


    esp_err_t err =
        spi_device_transmit(
            imu_spi,
            &t
        );


    gpio_set_level(
        IMU_CS,
        1
    );


    if (err == ESP_OK)
    {
        memcpy(
            data,
            &rx_data[1],
            size
        );
    }


    return err;
}


//=============================================================================
// LSM6DSOX SPI Write
//=============================================================================

static esp_err_t lsm6dsox_write_reg(
    uint8_t reg,
    uint8_t val
)
{
    uint8_t tx_data[2];


    tx_data[0] =
        reg & ~0x80;

    tx_data[1] =
        val;


    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };


    gpio_set_level(
        IMU_CS,
        0
    );


    esp_err_t err =
        spi_device_transmit(
            imu_spi,
            &t
        );


    gpio_set_level(
        IMU_CS,
        1
    );


    return err;
}


//=============================================================================
// IMU Setup
//=============================================================================

static void setupIMU(void)
{
    //-------------------------------------------------------------------------
    // CS
    //-------------------------------------------------------------------------

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << IMU_CS),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(
            &io_conf
        )
    );


    gpio_set_level(
        IMU_CS,
        1
    );


    //-------------------------------------------------------------------------
    // SPI Bus
    //-------------------------------------------------------------------------

    spi_bus_config_t buscfg = {
        .mosi_io_num = IMU_MOSI,
        .miso_io_num = IMU_MISO,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };


    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 10 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1,
        .queue_size = 7,
    };


    ESP_ERROR_CHECK(
        spi_bus_initialize(
            SPI2_HOST,
            &buscfg,
            SPI_DMA_CH_AUTO
        )
    );


    ESP_ERROR_CHECK(
        spi_bus_add_device(
            SPI2_HOST,
            &devcfg,
            &imu_spi
        )
    );


    vTaskDelay(
        pdMS_TO_TICKS(50)
    );


    //-------------------------------------------------------------------------
    // WHO_AM_I
    //-------------------------------------------------------------------------

    uint8_t whoami = 0;


    ESP_ERROR_CHECK(
        lsm6dsox_read_reg(
            LSM6DSOX_WHO_AM_I,
            &whoami,
            1
        )
    );


    if (whoami != LSM6DSOX_WHO_AM_I_VAL)
    {
        ESP_LOGE(
            TAG,
            "LSM6DSOX not found! Expected 0x%02X, got 0x%02X",
            LSM6DSOX_WHO_AM_I_VAL,
            whoami
        );


        while (1)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    //-------------------------------------------------------------------------
    // Accelerometer
    //
    // 104 Hz
    // ±2g
    //-------------------------------------------------------------------------

    ESP_ERROR_CHECK(
        lsm6dsox_write_reg(
            LSM6DSOX_CTRL1_XL,
            0x40
        )
    );


    ESP_LOGI(
        TAG,
        "LSM6DSOX initialized"
    );
}


//=============================================================================
// Read Acceleration
//=============================================================================

static void readAcceleration(
    int16_t *x,
    int16_t *y,
    int16_t *z
)
{
    uint8_t data[6];


    if (
        lsm6dsox_read_reg(
            LSM6DSOX_OUTX_L_A,
            data,
            6
        ) != ESP_OK
    )
    {
        *x = 0;
        *y = 0;
        *z = 0;

        return;
    }


    *x =
        (int16_t)(
            ((uint16_t)data[1] << 8) |
            data[0]
        );


    *y =
        (int16_t)(
            ((uint16_t)data[3] << 8) |
            data[2]
        );


    *z =
        (int16_t)(
            ((uint16_t)data[5] << 8) |
            data[4]
        );
}


//=============================================================================
// Tilt
//=============================================================================

static void readTilt(void)
{
    int16_t ax;
    int16_t ay;
    int16_t az;


    readAcceleration(
        &ax,
        &ay,
        &az
    );


    /*
     * Physical Y -> Snake X
     *
     * NOT INVERTED
     */

    if (
        abs(ay) >= TILT_THRESHOLD &&
        abs(ay) > abs(ax)
    )
    {
        dirY = 0;


        if (
            ay > 0 &&
            dirX != -1
        )
        {
            dirX = 1;
        }


        if (
            ay < 0 &&
            dirX != 1
        )
        {
            dirX = -1;
        }
    }


    /*
     * Physical X -> Snake Y
     *
     * INVERTED
     */

    else if (
        abs(ax) >= TILT_THRESHOLD
    )
    {
        dirX = 0;


        if (
            ax > 0 &&
            dirY != 1
        )
        {
            dirY = -1;
        }


        if (
            ax < 0 &&
            dirY != -1
        )
        {
            dirY = 1;
        }
    }
}


//=============================================================================
// Snake Collision
//=============================================================================

static bool snakeContains(
    int x,
    int y
)
{
    for (
        int i = 0;
        i < snakeLength;
        i++
    )
    {
        if (
            snakeX[i] == x &&
            snakeY[i] == y
        )
        {
            return true;
        }
    }


    return false;
}


//=============================================================================
// Apple
//=============================================================================

static void placeApple(void)
{
    do
    {
        appleX =
            rand() % 5;

        appleY =
            rand() % 5;

    } while (
        snakeContains(
            appleX,
            appleY
        )
    );
}


//=============================================================================
// Reset Game
//=============================================================================

static void resetGame(void)
{
    snakeLength = 3;


    snakeX[0] = 2;
    snakeY[0] = 2;

    snakeX[1] = 1;
    snakeY[1] = 2;

    snakeX[2] = 0;
    snakeY[2] = 2;


    dirX = 1;
    dirY = 0;


    placeApple();


    gameOver = false;


    startupFlash();


    lastMove =
        esp_timer_get_time() / 1000;


    drawGame();
}


//=============================================================================
// Game Over
//=============================================================================

static void showGameOver(void)
{
    matrixClear();


    uint8_t red =
        scaleBrightness(255);


    for (
        int i = 0;
        i < NUM_PIXELS;
        i++
    )
    {
        led_strip_set_pixel(
            led_strip,
            i,
            red,
            0,
            0
        );
    }


    matrixShow();


    buzzerTone(
        150,
        400
    );
}


//=============================================================================
// Move Snake
//=============================================================================

static void moveSnake(void)
{
    int newX =
        snakeX[0] + dirX;

    int newY =
        snakeY[0] + dirY;


    //-------------------------------------------------------------------------
    // Wrap
    //-------------------------------------------------------------------------

    if (newX < 0)
        newX = 4;

    if (newX > 4)
        newX = 0;

    if (newY < 0)
        newY = 4;

    if (newY > 4)
        newY = 0;


    //-------------------------------------------------------------------------
    // Collision
    //-------------------------------------------------------------------------

    if (
        snakeContains(
            newX,
            newY
        )
    )
    {
        gameOver = true;

        showGameOver();

        return;
    }


    //-------------------------------------------------------------------------
    // Apple
    //-------------------------------------------------------------------------

    bool ateApple =
        (
            newX == appleX &&
            newY == appleY
        );


    if (ateApple)
    {
        snakeLength++;


        buzzerTone(
            1200,
            80
        );


        if (snakeLength >= 25)
        {
            gameOver = true;

            return;
        }
    }


    //-------------------------------------------------------------------------
    // Shift body
    //-------------------------------------------------------------------------

    for (
        int i = snakeLength - 1;
        i > 0;
        i--
    )
    {
        snakeX[i] =
            snakeX[i - 1];

        snakeY[i] =
            snakeY[i - 1];
    }


    snakeX[0] = newX;
    snakeY[0] = newY;


    if (ateApple)
        placeApple();


    drawGame();
}


//=============================================================================
// Matrix Setup
//=============================================================================

static void setupMatrix(void)
{
    /*
     * This is the exact led_strip configuration
     * from your working code.
     */

    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_PIXELS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );


    led_strip_clear(
        led_strip
    );


    led_strip_refresh(
        led_strip
    );
}


//=============================================================================
// Button Setup
//=============================================================================

static void setupButton(void)
{
    gpio_config_t button_config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(
            &button_config
        )
    );
}


//=============================================================================
// Buzzer Setup
//=============================================================================

static void setupBuzzer(void)
{
    gpio_config_t buzzer_config = {
        .pin_bit_mask = (1ULL << BUZZER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(
            &buzzer_config
        )
    );


    gpio_set_level(
        BUZZER_PIN,
        0
    );
}


//=============================================================================
// Main
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Starting Snake..."
    );


    setupButton();

    setupBuzzer();

    setupMatrix();

    setupIMU();


    srand(
        (unsigned int)esp_random()
    );


    resetGame();


    ESP_LOGI(
        TAG,
        "Snake started."
    );


    while (1)
    {
        //=====================================================================
        // Game Over
        //=====================================================================

        if (gameOver)
        {
            if (
                gpio_get_level(BUTTON_PIN) == 0
            )
            {
                vTaskDelay(
                    pdMS_TO_TICKS(200)
                );


                resetGame();
            }


            vTaskDelay(
                pdMS_TO_TICKS(10)
            );


            continue;
        }


        //=====================================================================
        // Tilt
        //=====================================================================

        readTilt();


        //=====================================================================
        // Movement
        //=====================================================================

        int64_t now =
            esp_timer_get_time() / 1000;


        if (
            now - lastMove >= MOVE_DELAY
        )
        {
            lastMove = now;

            moveSnake();
        }


        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
