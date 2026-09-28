/*
 * =============================================================================
 * PROXIMITY-BASED GREETING MACHINE - ESP-IDF v5+
 * =============================================================================
 *
 * When someone approaches the RPR-0521RS proximity sensor:
 *
 *   1. A smiley face appears on the RGB matrix.
 *   2. The buzzer plays a short three-note greeting chime.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * Matrix:
 *   GPIO 7
 *   25 pixels
 *   WS2812 / GRB
 *
 * Buzzer:
 *   GPIO 16
 *
 * Sensor:
 *   RPR-0521RS
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "driver/ledc.h"

#include "led_strip.h"

#include "esp_log.h"
#include "esp_err.h"

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

#define BUZZER_PIN              16

#define PROXIMITY_THRESHOLD     500

#define MATRIX_BRIGHTNESS       20


static const char *TAG = "GREETING";


/*
 * =============================================================================
 * Global Handles
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

    i2c_master_bus_handle_t bus_handle = NULL;


    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );


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
        "I2C initialized: SDA=%d SCL=%d",
        I2C_MASTER_SDA_IO,
        I2C_MASTER_SCL_IO
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

        .mem_block_symbols =
            64,

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
 * Matrix Pixel Mapping
 * =============================================================================
 *
 * The matrix uses the same serpentine wiring as your other programs:
 *
 * Row 0:  -> -> -> -> ->
 * Row 1:  <- <- <- <- <-
 * Row 2:  -> -> -> -> ->
 * Row 3:  <- <- <- <- <-
 * Row 4:  -> -> -> -> ->
 *
 * x = 0..4
 * y = 0..4
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return -1;
    }


    if ((y % 2) == 0) {
        return y * 5 + x;
    }


    return y * 5 + (4 - x);
}


/*
 * =============================================================================
 * Set Pixel
 * =============================================================================
 */

static void set_pixel_xy(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    int index =
        pixel_xy(x, y);


    if (index < 0) {
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
 * Show Smiley
 * =============================================================================
 */

static void show_smiley(void)
{
    /*
     * Clear matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    /*
     * Yellow smiley color.
     *
     * Arduino equivalent:
     *
     * matrix.Color(255, 180, 0)
     */

    const uint8_t red = 255;
    const uint8_t green = 180;
    const uint8_t blue = 0;


    /*
     * Eyes.
     */

    set_pixel_xy(
        1, 1,
        red,
        green,
        blue
    );

    set_pixel_xy(
        3, 1,
        red,
        green,
        blue
    );


    /*
     * Smile.
     */

    set_pixel_xy(
        1, 3,
        red,
        green,
        blue
    );

    set_pixel_xy(
        2, 4,
        red,
        green,
        blue
    );

    set_pixel_xy(
        3, 3,
        red,
        green,
        blue
    );


    /*
     * Send pixels to matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Buzzer / LEDC Initialization
 * =============================================================================
 *
 * Uses the ESP-IDF v5 LEDC driver.
 *
 * The buzzer is connected to GPIO 16.
 * =============================================================================
 */

static void buzzer_init(void)
{
    ledc_timer_config_t timer_config = {
        .speed_mode =
            LEDC_LOW_SPEED_MODE,

        .duty_resolution =
            LEDC_TIMER_10_BIT,

        .timer_num =
            LEDC_TIMER_0,

        .freq_hz =
            1000,

        .clk_cfg =
            LEDC_AUTO_CLK,

        .deconfigure =
            false,
    };


    ESP_ERROR_CHECK(
        ledc_timer_config(
            &timer_config
        )
    );


    ledc_channel_config_t channel_config = {
        .gpio_num =
            BUZZER_PIN,

        .speed_mode =
            LEDC_LOW_SPEED_MODE,

        .channel =
            LEDC_CHANNEL_0,

        .intr_type =
            LEDC_INTR_DISABLE,

        .timer_sel =
            LEDC_TIMER_0,

        .duty =
            0,

        .hpoint =
            0,

        .flags = {
            .output_invert = 0,
        },
    };


    ESP_ERROR_CHECK(
        ledc_channel_config(
            &channel_config
        )
    );


    /*
     * Start with buzzer OFF.
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
 * Play Buzzer Note
 * =============================================================================
 */

static void play_note(
    uint32_t frequency,
    uint32_t duration_ms
)
{
    /*
     * Change the buzzer frequency.
     */

    ESP_ERROR_CHECK(
        ledc_set_freq(
            LEDC_LOW_SPEED_MODE,
            LEDC_TIMER_0,
            frequency
        )
    );


    /*
     * Approximately 50% duty cycle.
     *
     * 10-bit resolution:
     *
     * 1023 / 2 = approximately 512
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


    /*
     * Keep note playing.
     */

    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );


    /*
     * Turn buzzer OFF.
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
 * Greeting Chime
 * =============================================================================
 *
 * Arduino version:
 *
 *   784 Hz  - 150 ms
 *   988 Hz  - 150 ms
 *   1175 Hz - 250 ms
 *
 * =============================================================================
 */

static void greeting_chime(void)
{
    /*
     * G5
     */

    play_note(
        784,
        150
    );

    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    /*
     * B5
     */

    play_note(
        988,
        150
    );

    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    /*
     * D6
     */

    play_note(
        1175,
        250
    );


    /*
     * Make absolutely sure buzzer is OFF.
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
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * -------------------------------------------------------------------------
     * 1. I2C
     * -------------------------------------------------------------------------
     */

    i2c_master_init();


    /*
     * -------------------------------------------------------------------------
     * 2. RPR-0521RS
     * -------------------------------------------------------------------------
     */

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Sensor initialization failed!"
        );


        while (1) {

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    ESP_LOGI(
        TAG,
        "RPR-0521RS initialized"
    );


    /*
     * -------------------------------------------------------------------------
     * 3. Matrix
     * -------------------------------------------------------------------------
     */

    matrix_init();


    /*
     * -------------------------------------------------------------------------
     * 4. Buzzer
     * -------------------------------------------------------------------------
     */

    buzzer_init();


    /*
     * -------------------------------------------------------------------------
     * 5. Initial matrix state
     * -------------------------------------------------------------------------
     */

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "      PROXIMITY GREETING MACHINE"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Proximity threshold: %d",
        PROXIMITY_THRESHOLD
    );


    /*
     * -------------------------------------------------------------------------
     * Person detection state
     * -------------------------------------------------------------------------
     */

    bool personDetected = false;


    /*
     * -------------------------------------------------------------------------
     * Main loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        uint16_t proximity = 0;
        float lux = 0.0f;


        /*
         * Read proximity and ambient light.
         */

        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (ret == ESP_OK) {

            bool detected =
                proximity >
                PROXIMITY_THRESHOLD;


            /*
             * Trigger only when the person transitions from
             * "not detected" -> "detected".
             */

            if (detected &&
                !personDetected) {

                ESP_LOGI(
                    TAG,
                    "Hello! Proximity = %u",
                    proximity
                );


                /*
                 * Show smiley.
                 */

                show_smiley();


                /*
                 * Play greeting.
                 */

                greeting_chime();
            }


            /*
             * Save current detection state.
             */

            personDetected =
                detected;
        }

        else {

            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );
        }


        /*
         * Equivalent to Arduino delay(50).
         */

        vTaskDelay(
            pdMS_TO_TICKS(50)
        );
    }
}
