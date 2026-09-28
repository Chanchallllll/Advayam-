/*
 * =============================================================================
 * REACTION TIMER GAME - ESP-IDF v5+
 * =============================================================================
 *
 * The ESP32 waits for the user to press the button, then waits for a random
 * amount of time before sounding the buzzer.
 *
 * When the buzzer sounds, the player must press the button as quickly as
 * possible. Reaction time is measured in milliseconds and printed to the
 * Serial Monitor.
 *
 * Hardware:
 *
 *   Button : GPIO 17
 *   Buzzer : GPIO 16
 *
 * Button configuration:
 *   INPUT_PULLUP
 *   Pressed = LOW
 *
 * Buzzer:
 *   LEDC PWM
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "esp_log.h"
#include "esp_random.h"
#include "esp_timer.h"

/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define BUTTON_PIN          GPIO_NUM_17
#define BUZZER_PIN          GPIO_NUM_16

#define BUZZER_FREQUENCY    1000

#define RANDOM_DELAY_MIN    1000
#define RANDOM_DELAY_MAX    4000


static const char *TAG = "REACTION_TIMER";


/*
 * =============================================================================
 * Buzzer Configuration
 * =============================================================================
 *
 * LEDC is used instead of Arduino tone().
 *
 * ESP-IDF 5.x LEDC configuration.
 * =============================================================================
 */

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_DUTY_RES    LEDC_TIMER_10_BIT

#define BUZZER_DUTY             512


/*
 * =============================================================================
 * Buzzer Start
 * =============================================================================
 */

static void buzzer_on(uint32_t frequency)
{
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
}


/*
 * =============================================================================
 * Buzzer Stop
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
 * Buzzer Initialization
 * =============================================================================
 */

static void buzzer_init(void)
{
    /*
     * Configure LEDC timer.
     */
    ledc_timer_config_t timer_config = {
        .speed_mode       = BUZZER_LEDC_MODE,
        .duty_resolution  = BUZZER_LEDC_DUTY_RES,
        .timer_num        = BUZZER_LEDC_TIMER,
        .freq_hz          = BUZZER_FREQUENCY,
        .clk_cfg          = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    /*
     * Configure LEDC output channel.
     */
    ledc_channel_config_t channel_config = {
        .gpio_num       = BUZZER_PIN,
        .speed_mode     = BUZZER_LEDC_MODE,
        .channel        = BUZZER_LEDC_CHANNEL,
        .intr_type      = LEDC_INTR_DISABLE,
        .timer_sel      = BUZZER_LEDC_TIMER,
        .duty           = 0,
        .hpoint         = 0,
        .flags.output_invert = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );


    buzzer_off();
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

        .intr_type = GPIO_INTR_DISABLE
    };

    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );
}


/*
 * =============================================================================
 * Wait For Button Release
 * =============================================================================
 */

static void wait_for_release(void)
{
    while (gpio_get_level(BUTTON_PIN) == 0) {

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}


/*
 * =============================================================================
 * Wait For Button Press
 * =============================================================================
 */

static void wait_for_press(void)
{
    while (gpio_get_level(BUTTON_PIN) == 1) {

        /*
         * Small delay prevents unnecessary CPU usage.
         */
        vTaskDelay(
            pdMS_TO_TICKS(1)
        );
    }
}


/*
 * =============================================================================
 * Generate Random Delay
 * =============================================================================
 */

static uint32_t random_delay_ms(void)
{
    /*
     * esp_random() provides hardware-generated random numbers.
     *
     * Range:
     *   1000 ms -> 3999 ms
     */

    uint32_t range =
        RANDOM_DELAY_MAX -
        RANDOM_DELAY_MIN;

    return RANDOM_DELAY_MIN +
           (esp_random() % range);
}


/*
 * =============================================================================
 * Main Application
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
     * Make sure buzzer starts OFF.
     */
    buzzer_off();


    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "       REACTION TIMER"
    );

    ESP_LOGI(
        TAG,
        "================================"
    );

    ESP_LOGI(
        TAG,
        "Press the button to start."
    );


    /*
     * Main game loop.
     */
    while (1) {

        /*
         * ================================================================
         * WAIT FOR START BUTTON
         * ================================================================
         */

        wait_for_press();


        /*
         * Make sure the player releases the button before starting.
         */
        wait_for_release();


        ESP_LOGI(
            TAG,
            "Get ready..."
        );


        /*
         * ================================================================
         * RANDOM WAIT
         * ================================================================
         */

        uint32_t delay_ms =
            random_delay_ms();

        vTaskDelay(
            pdMS_TO_TICKS(delay_ms)
        );


        /*
         * ================================================================
         * GO!
         * ================================================================
         */

        ESP_LOGI(
            TAG,
            "GO!"
        );


        /*
         * Start buzzer.
         */
        buzzer_on(
            BUZZER_FREQUENCY
        );


        /*
         * Record exact start time.
         *
         * esp_timer_get_time() would give microsecond resolution,
         * but millis()-style timing is sufficient for this game.
         *
         * FreeRTOS tick timing is not used here so the reaction
         * measurement isn't limited by vTaskDelay().
         */

        int64_t start_time_us =
            esp_timer_get_time();


        /*
         * ================================================================
         * WAIT FOR PLAYER RESPONSE
         * ================================================================
         */

        wait_for_press();


        /*
         * Record reaction time immediately.
         */
        int64_t reaction_time_us =
            esp_timer_get_time() -
            start_time_us;


        /*
         * Stop buzzer.
         */
        buzzer_off();


        /*
         * Convert microseconds to milliseconds.
         */
        uint32_t reaction_time_ms =
            (uint32_t)(
                reaction_time_us / 1000
            );


        /*
         * ================================================================
         * DISPLAY RESULT
         * ================================================================
         */

        ESP_LOGI(
            TAG,
            "Reaction time: %lu ms",
            (unsigned long)reaction_time_ms
        );


        /*
         * Wait until button is released before allowing another round.
         */
        wait_for_release();


        /*
         * Short pause before next game.
         */
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );


        ESP_LOGI(
            TAG,
            "Press button to play again."
        );
    }
}
