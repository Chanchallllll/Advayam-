/*
 * =============================================================================
 * SHADOW PUPPETRY BRIGHTNESS DETECTOR - ESP-IDF v5+
 * =============================================================================
 *
 * Detects rapid changes in ambient brightness using the RPR-0521RS.
 *
 * A sudden reduction in light produces a low sound effect.
 * A sudden increase in light produces a higher sound effect.
 *
 * Buzzer:
 *   GPIO 16
 *
 * I2C:
 *   SDA: GPIO 5
 *   SCL: GPIO 6
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"
#include "driver/ledc.h"

#include "esp_log.h"

#include "rpr0521rs.h"


// =============================================================================
// I2C CONFIGURATION
// =============================================================================

#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000


// =============================================================================
// BUZZER CONFIGURATION
// =============================================================================

#define BUZZER_PIN           16

#define BUZZER_LEDC_TIMER    LEDC_TIMER_0
#define BUZZER_LEDC_MODE     LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL  LEDC_CHANNEL_0

#define BUZZER_DUTY_RES      LEDC_TIMER_13_BIT

#define BUZZER_LOW_FREQ      300
#define BUZZER_HIGH_FREQ     1200

#define BUZZER_DUTY          4096


// =============================================================================
// BRIGHTNESS DETECTION
// =============================================================================

#define LUX_CHANGE_THRESHOLD  20.0f

#define LOOP_DELAY_MS         80

#define TONE_DURATION_MS      150


// =============================================================================
// GLOBALS
// =============================================================================

static const char *TAG = "SHADOW_PUPPETRY";

static i2c_master_dev_handle_t sensor_dev = NULL;


// =============================================================================
// I2C INITIALIZATION
// =============================================================================

static void i2c_master_init(void)
{
    /*
     * Configure ESP-IDF I2C master bus.
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


    i2c_master_bus_handle_t bus_handle = NULL;


    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );


    /*
     * RPR-0521RS I2C device configuration.
     */

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


// =============================================================================
// BUZZER INITIALIZATION
// =============================================================================

static void buzzer_init(void)
{
    /*
     * Configure LEDC timer.
     *
     * LEDC generates the square wave that drives the buzzer.
     */

    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_LEDC_MODE,

        .timer_num = BUZZER_LEDC_TIMER,

        .duty_resolution = BUZZER_DUTY_RES,

        .freq_hz = BUZZER_LOW_FREQ,

        .clk_cfg = LEDC_AUTO_CLK,
    };


    ESP_ERROR_CHECK(
        ledc_timer_config(
            &timer_config
        )
    );


    /*
     * Configure buzzer output channel.
     */

    ledc_channel_config_t channel_config = {
        .speed_mode = BUZZER_LEDC_MODE,

        .channel = BUZZER_LEDC_CHANNEL,

        .timer_sel = BUZZER_LEDC_TIMER,

        .intr_type = LEDC_INTR_DISABLE,

        .gpio_num = BUZZER_PIN,

        .duty = 0,

        .hpoint = 0,
    };


    ESP_ERROR_CHECK(
        ledc_channel_config(
            &channel_config
        )
    );
}


// =============================================================================
// BUZZER TONE
// =============================================================================

static void buzzer_tone(
    uint32_t frequency,
    uint32_t duration_ms)
{
    /*
     * Change the LEDC frequency.
     */

    ESP_ERROR_CHECK(
        ledc_set_freq(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_TIMER,
            frequency
        )
    );


    /*
     * 50% duty cycle.
     *
     * 13-bit maximum = 8191
     * 4096 ~= 50%
     */

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


    /*
     * Keep the tone active for the requested duration.
     */

    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );


    /*
     * Turn buzzer off.
     */

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


// =============================================================================
// MAIN
// =============================================================================

void app_main(void)
{
    // =========================================================================
    // INITIALIZE I2C
    // =========================================================================

    i2c_master_init();


    // =========================================================================
    // INITIALIZE BUZZER
    // =========================================================================

    buzzer_init();


    // =========================================================================
    // INITIALIZE RPR-0521RS
    // =========================================================================

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


    // =========================================================================
    // INITIAL STATE
    // =========================================================================

    float previous_lux = 0.0f;


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "   SHADOW PUPPETRY DETECTOR STARTED"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "I2C SDA: GPIO %d",
        I2C_MASTER_SDA_IO
    );

    ESP_LOGI(
        TAG,
        "I2C SCL: GPIO %d",
        I2C_MASTER_SCL_IO
    );

    ESP_LOGI(
        TAG,
        "Buzzer: GPIO %d",
        BUZZER_PIN
    );

    ESP_LOGI(
        TAG,
        "Brightness threshold: %.1f lux",
        LUX_CHANGE_THRESHOLD
    );


    // =========================================================================
    // MAIN LOOP
    // =========================================================================

    while (1)
    {
        uint16_t proximity;
        float lux;


        // ---------------------------------------------------------------------
        // READ RPR-0521RS
        // ---------------------------------------------------------------------

        esp_err_t err =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (err == ESP_OK) {

            // -----------------------------------------------------------------
            // CALCULATE CHANGE
            // -----------------------------------------------------------------

            float change =
                lux - previous_lux;


            ESP_LOGD(
                TAG,
                "Lux: %.2f | Change: %.2f | Proximity: %u",
                lux,
                change,
                proximity
            );


            // -----------------------------------------------------------------
            // SHADOW DETECTED
            // -----------------------------------------------------------------

            if (change < -LUX_CHANGE_THRESHOLD) {

                ESP_LOGI(
                    TAG,
                    "Shadow detected! Lux: %.2f -> %.2f",
                    previous_lux,
                    lux
                );


                /*
                 * Low tone:
                 *
                 * Arduino:
                 *
                 *     tone(BUZZER_PIN, 300, 150);
                 */

                buzzer_tone(
                    BUZZER_LOW_FREQ,
                    TONE_DURATION_MS
                );
            }


            // -----------------------------------------------------------------
            // LIGHT RESTORED
            // -----------------------------------------------------------------

            else if (change > LUX_CHANGE_THRESHOLD) {

                ESP_LOGI(
                    TAG,
                    "Light restored! Lux: %.2f -> %.2f",
                    previous_lux,
                    lux
                );


                /*
                 * High tone:
                 *
                 * Arduino:
                 *
                 *     tone(BUZZER_PIN, 1200, 150);
                 */

                buzzer_tone(
                    BUZZER_HIGH_FREQ,
                    TONE_DURATION_MS
                );
            }


            // -----------------------------------------------------------------
            // SAVE CURRENT LUX
            // -----------------------------------------------------------------

            previous_lux = lux;


        } else {

            ESP_LOGW(
                TAG,
                "Failed to read RPR-0521RS"
            );
        }


        // ---------------------------------------------------------------------
        // SAMPLE EVERY 80 ms
        // ---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(LOOP_DELAY_MS)
        );
    }
}
