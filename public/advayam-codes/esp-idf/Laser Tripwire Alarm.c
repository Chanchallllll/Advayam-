/*
 * =============================================================================
 * LASER TRIPWIRE / BEAM BREAK ALARM - ESP-IDF v5+
 * =============================================================================
 *
 * Uses the RPR-0521RS ambient-light sensor as a beam-break detector.
 *
 * A steady light source (laser / LED) illuminates the sensor.
 * When the beam is blocked, lux drops below the baseline threshold
 * and the buzzer sounds.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
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
#include "driver/gpio.h"

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

#define BUZZER_PIN              16

/*
 * Trigger when light drops by this percentage.
 *
 * 30 means:
 *
 *     trigger = baseline * 0.70
 *
 * So a 30% or greater reduction in light triggers the alarm.
 */
#define BEAM_BREAK_PERCENT      30

/*
 * Number of samples used to establish the initial baseline.
 */
#define BASELINE_SAMPLES        20

/*
 * Delay between baseline samples.
 */
#define BASELINE_DELAY_MS       100

/*
 * Main sensor polling interval.
 */
#define SENSOR_DELAY_MS         50


static const char *TAG = "BEAM_TRIPWIRE";


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static i2c_master_dev_handle_t sensor_dev = NULL;


/*
 * =============================================================================
 * Buzzer State
 * =============================================================================
 */

static volatile bool buzzer_active = false;


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


    /*
     * RPR-0521RS I2C device.
     */
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
 * Buzzer
 * =============================================================================
 *
 * Instead of tone()/noTone(), generate the buzzer waveform directly using
 * GPIO.
 *
 * This avoids the Arduino API and also avoids the legacy ESP-IDF LEDC API.
 * =============================================================================
 */

static void buzzer_task(void *arg)
{
    while (1) {

        if (buzzer_active) {

            /*
             * Approximately 2 kHz.
             *
             * Period = 500 us
             * Half period = 250 us
             */
            gpio_set_level(BUZZER_PIN, 1);

            esp_rom_delay_us(250);

            gpio_set_level(BUZZER_PIN, 0);

            esp_rom_delay_us(250);

        } else {

            gpio_set_level(BUZZER_PIN, 0);

            vTaskDelay(
                pdMS_TO_TICKS(10)
            );
        }
    }
}


/*
 * =============================================================================
 * Buzzer Initialization
 * =============================================================================
 */

static void buzzer_init(void)
{
    gpio_config_t buzzer_config = {
        .pin_bit_mask =
            (1ULL << BUZZER_PIN),

        .mode =
            GPIO_MODE_OUTPUT,

        .pull_up_en =
            GPIO_PULLUP_DISABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE,
    };


    ESP_ERROR_CHECK(
        gpio_config(&buzzer_config)
    );


    gpio_set_level(
        BUZZER_PIN,
        0
    );


    /*
     * Start buzzer waveform task.
     */
    xTaskCreate(
        buzzer_task,
        "buzzer_task",
        2048,
        NULL,
        5,
        NULL
    );
}


/*
 * =============================================================================
 * Establish Light Baseline
 * =============================================================================
 */

static float establish_baseline(void)
{
    float baseline_lux = 0.0f;

    int valid_samples = 0;


    ESP_LOGI(
        TAG,
        "Establishing light baseline..."
    );


    for (int i = 0; i < BASELINE_SAMPLES; i++) {

        uint16_t proximity = 0;
        float lux = 0.0f;


        if (rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            ) == ESP_OK) {

            baseline_lux += lux;

            valid_samples++;


            ESP_LOGI(
                TAG,
                "Baseline sample %d/%d: Lux = %.2f",
                i + 1,
                BASELINE_SAMPLES,
                lux
            );
        }
        else {

            ESP_LOGW(
                TAG,
                "Failed to read baseline sample"
            );
        }


        vTaskDelay(
            pdMS_TO_TICKS(BASELINE_DELAY_MS)
        );
    }


    if (valid_samples == 0) {

        ESP_LOGE(
            TAG,
            "Could not obtain any valid light readings!"
        );

        return 0.0f;
    }


    baseline_lux /=
        (float)valid_samples;


    return baseline_lux;
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
     * 1. Initialize I2C
     * -------------------------------------------------------------------------
     */

    i2c_master_init();


    /*
     * -------------------------------------------------------------------------
     * 2. Initialize RPR-0521RS
     * -------------------------------------------------------------------------
     */

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "RPR-0521RS initialization failed!"
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
     * 3. Initialize buzzer
     * -------------------------------------------------------------------------
     */

    buzzer_init();


    /*
     * -------------------------------------------------------------------------
     * 4. Establish baseline
     * -------------------------------------------------------------------------
     */

    float baselineLux =
        establish_baseline();


    if (baselineLux <= 0.0f) {

        ESP_LOGE(
            TAG,
            "Invalid baseline lux!"
        );

        while (1) {

            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    /*
     * Calculate the light level at which the alarm triggers.
     *
     * Example:
     *
     * baseline = 1000 lux
     * break percentage = 30%
     *
     * trigger = 700 lux
     */
    float triggerLevel =
        baselineLux *
        (1.0f -
         ((float)BEAM_BREAK_PERCENT / 100.0f));


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       LASER TRIPWIRE ARMED"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Baseline Lux : %.2f",
        baselineLux
    );

    ESP_LOGI(
        TAG,
        "Trigger Lux  : %.2f",
        triggerLevel
    );

    ESP_LOGI(
        TAG,
        "Trigger Drop : %d%%",
        BEAM_BREAK_PERCENT
    );


    /*
     * -------------------------------------------------------------------------
     * 5. Main loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        uint16_t proximity = 0;
        float lux = 0.0f;


        /*
         * Read RPR-0521RS.
         */
        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity,
                &lux
            );


        if (ret == ESP_OK) {

            /*
             * Beam broken.
             */
            if (lux < triggerLevel) {

                buzzer_active = true;


                ESP_LOGW(
                    TAG,
                    "BEAM BROKEN! Lux = %.2f | Trigger = %.2f",
                    lux,
                    triggerLevel
                );
            }

            /*
             * Beam present.
             */
            else {

                buzzer_active = false;


                ESP_LOGI(
                    TAG,
                    "Beam OK | Lux = %.2f",
                    lux
                );
            }
        }

        else {

            ESP_LOGW(
                TAG,
                "Failed to read proximity/light sensor"
            );
        }


        vTaskDelay(
            pdMS_TO_TICKS(SENSOR_DELAY_MS)
        );
    }
}
