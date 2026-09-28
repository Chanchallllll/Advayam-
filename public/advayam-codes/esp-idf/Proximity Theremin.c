/*
 * =============================================================================
 * PROXIMITY DISTANCE THEREMIN - ESP-IDF v5+
 * =============================================================================
 * Uses the RPR-0521RS proximity reading to control the pitch of a buzzer.
 *
 * Hand closer  -> higher pitch
 * Hand farther -> lower pitch
 *
 * I2C:
 *   SDA        : GPIO 5
 *   SCL        : GPIO 6
 *
 * Buzzer:
 *   GPIO 16
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

#define BUZZER_PIN              GPIO_NUM_16


/*
 * Proximity calibration range.
 */
#define MIN_PROXIMITY           50
#define MAX_PROXIMITY           2000


/*
 * Audio frequency range.
 */
#define MIN_FREQUENCY           150
#define MAX_FREQUENCY           2500


/*
 * LEDC configuration.
 *
 * The buzzer frequency itself is changed dynamically.
 */
#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_DUTY_RES         LEDC_TIMER_13_BIT

#define BUZZER_DUTY             4096    // ~50% duty cycle


static const char *TAG = "PROXIMITY_THEREMIN";


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;


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
 * Buzzer Initialization
 * =============================================================================
 */

static void buzzer_init(void)
{
    /*
     * Configure LEDC timer.
     *
     * The timer frequency is initially set to 150 Hz,
     * but we will change it dynamically as proximity changes.
     */
    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_LEDC_MODE,
        .timer_num = BUZZER_LEDC_TIMER,
        .duty_resolution = BUZZER_DUTY_RES,
        .freq_hz = MIN_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
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
        .hpoint = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );
}


/*
 * =============================================================================
 * Buzzer On
 * =============================================================================
 */

static void buzzer_on(uint32_t frequency)
{
    /*
     * Change the LEDC timer frequency.
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
     * 13-bit resolution:
     *
     *     8191 = 100%
     *     4096 ≈ 50%
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
}


/*
 * =============================================================================
 * Buzzer Off
 * =============================================================================
 */

static void buzzer_off(void)
{
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


/*
 * =============================================================================
 * Integer Constrain
 * =============================================================================
 */

static int constrain_int(
    int value,
    int minimum,
    int maximum
)
{
    if (value < minimum) {
        return minimum;
    }

    if (value > maximum) {
        return maximum;
    }

    return value;
}


/*
 * =============================================================================
 * Arduino-style map()
 * =============================================================================
 */

static int map_value(
    int value,
    int input_min,
    int input_max,
    int output_min,
    int output_max
)
{
    return (
        (value - input_min) *
        (output_max - output_min) /
        (input_max - input_min)
    ) + output_min;
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
     * Initialize buzzer.
     */
    buzzer_init();


    /*
     * Initialize RPR-0521RS sensor.
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


    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "      PROXIMITY DISTANCE THEREMIN");
    ESP_LOGI(TAG, "========================================");

    ESP_LOGI(
        TAG,
        "Proximity range: %d - %d",
        MIN_PROXIMITY,
        MAX_PROXIMITY
    );

    ESP_LOGI(
        TAG,
        "Frequency range: %d - %d Hz",
        MIN_FREQUENCY,
        MAX_FREQUENCY
    );


    /*
     * Main loop.
     */
    while (1) {

        uint16_t proximity;
        float lux;


        /*
         * Read proximity + ambient light.
         */
        if (rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            ) == ESP_OK) {


            /*
             * Limit proximity to our calibrated range.
             *
             * Equivalent to:
             *
             *     constrain(proximity,
             *               MIN_PROXIMITY,
             *               MAX_PROXIMITY);
             */
            int p = constrain_int(
                proximity,
                MIN_PROXIMITY,
                MAX_PROXIMITY
            );


            /*
             * Convert proximity to frequency.
             *
             * Low proximity  -> low frequency
             * High proximity -> high frequency
             */
            int frequency = map_value(
                p,
                MIN_PROXIMITY,
                MAX_PROXIMITY,
                MIN_FREQUENCY,
                MAX_FREQUENCY
            );


            /*
             * Only make sound when the proximity is above
             * the minimum threshold.
             */
            if (proximity > MIN_PROXIMITY) {

                buzzer_on(frequency);

            } else {

                buzzer_off();
            }


            /*
             * UART debug output.
             */
            ESP_LOGI(
                TAG,
                "Proximity: %u | Frequency: %d Hz",
                proximity,
                frequency
            );


        } else {

            /*
             * Sensor read failed.
             */
            ESP_LOGW(
                TAG,
                "Failed to read proximity sensor"
            );

            buzzer_off();
        }


        /*
         * Equivalent to Arduino:
         *
         *     delay(30);
         */
        vTaskDelay(
            pdMS_TO_TICKS(30)
        );
    }
}
