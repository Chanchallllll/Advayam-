/*
 * =============================================================================
 * TASK WATCHDOG TIMER DEMO
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Monitors the main application task using the ESP32 Task Watchdog Timer.
 *
 * User LED:
 *     GPIO 45
 *
 * The LED toggles every time the application successfully executes its
 * heartbeat loop and feeds the watchdog.
 *
 * Watchdog timeout:
 *     3 seconds
 *
 * Normal behavior:
 *
 *     LED toggles
 *     "Watchdog heartbeat" is printed
 *     Watchdog is reset
 *     Wait 500 ms
 *     Repeat
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_task_wdt.h"


/* =============================================================================
 * Configuration
 * =============================================================================
 */

#define USER_LED       GPIO_NUM_45

#define WDT_TIMEOUT    3


static const char *TAG = "WDT_DEMO";


/* =============================================================================
 * GPIO Initialization
 * =============================================================================
 */

static void user_led_init(void)
{
    gpio_config_t io_config = {
        .pin_bit_mask = (1ULL << USER_LED),

        .mode = GPIO_MODE_OUTPUT,

        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&io_config)
    );


    /*
     * Start with LED OFF.
     */
    ESP_ERROR_CHECK(
        gpio_set_level(USER_LED, 0)
    );


    ESP_LOGI(
        TAG,
        "User LED initialized on GPIO %d",
        USER_LED
    );
}


/* =============================================================================
 * Task Watchdog Initialization
 * =============================================================================
 */

static void watchdog_init(void)
{
    /*
     * ESP-IDF 6.x uses the configuration structure.
     *
     * timeout_ms:
     *     Watchdog timeout in milliseconds.
     *
     * idle_core_mask:
     *     We are not using the idle-task monitoring feature here.
     *
     * trigger_panic:
     *     If true, a watchdog timeout triggers the ESP-IDF panic
     *     handler, which normally results in a backtrace and reset.
     */
    esp_task_wdt_config_t wdt_config = {
        .timeout_ms = WDT_TIMEOUT * 1000,

        .idle_core_mask = 0,

        .trigger_panic = true,
    };


    esp_err_t ret =
        esp_task_wdt_init(&wdt_config);


    /*
     * ESP_ERR_INVALID_STATE means the Task Watchdog was already
     * initialized by ESP-IDF.
     *
     * This can happen if another component/application configuration
     * initialized it before app_main().
     *
     * In that case, we can continue and simply subscribe our task.
     */
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {

        ESP_LOGE(
            TAG,
            "Failed to initialize Task Watchdog: %s",
            esp_err_to_name(ret)
        );

        abort();
    }


    /*
     * Register the current task with the watchdog.
     *
     * NULL means:
     *     current task
     *
     * This is the ESP-IDF equivalent of:
     *
     *     esp_task_wdt_add(NULL);
     */
    ESP_ERROR_CHECK(
        esp_task_wdt_add(NULL)
    );


    ESP_LOGI(
        TAG,
        "Task Watchdog Started"
    );

    ESP_LOGI(
        TAG,
        "Timeout: %d seconds",
        WDT_TIMEOUT
    );
}


/* =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " Task Watchdog Timer Demo");
    ESP_LOGI(TAG, "==============================================");


    /*
     * Initialize GPIO.
     */
    user_led_init();


    /*
     * Initialize and subscribe the current task
     * to the Task Watchdog.
     */
    watchdog_init();


    ESP_LOGI(
        TAG,
        "Watchdog heartbeat loop started"
    );


    /* -------------------------------------------------------------------------
     * Main Loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        /*
         * Toggle User LED.
         *
         * Read the current GPIO level and invert it.
         */
        int current_level =
            gpio_get_level(USER_LED);

        ESP_ERROR_CHECK(
            gpio_set_level(
                USER_LED,
                !current_level
            )
        );


        /*
         * Successful loop execution.
         */
        ESP_LOGI(
            TAG,
            "Watchdog heartbeat"
        );


        /*
         * Feed / reset the Task Watchdog.
         *
         * Because this task is subscribed to the watchdog,
         * it must periodically call esp_task_wdt_reset().
         */
        ESP_ERROR_CHECK(
            esp_task_wdt_reset()
        );


        /*
         * Wait 500 ms.
         *
         * This is the ESP-IDF equivalent of:
         *
         *     delay(500);
         */
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}
