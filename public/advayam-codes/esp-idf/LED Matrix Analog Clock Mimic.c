/*
 * =============================================================================
 * 5x5 RGB MATRIX WI-FI ANALOG CLOCK
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Matrix:
 *     GPIO 7
 *     25 x WS2812 LEDs
 *
 * Clock:
 *
 *     RED   = Second hand
 *     GREEN = Minute hand
 *     BLUE  = Hour hand
 *     WHITE = Center
 *
 * Time:
 *     Obtained from NTP over Wi-Fi.
 *
 * Time zone:
 *     India Standard Time (UTC + 5:30)
 *
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_event.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "esp_netif_sntp.h"

#include "nvs_flash.h"

#include "led_strip.h"


/* =============================================================================
 * Hardware Configuration
 * =============================================================================
 */

#define MATRIX_PIN      7
#define NUM_LEDS        25


/* =============================================================================
 * Wi-Fi Configuration
 * =============================================================================
 *
 * Replace these locally with your Wi-Fi credentials.
 *
 * Better option later:
 *     Move these into menuconfig rather than keeping credentials in source.
 *
 * =============================================================================
 */

#define WIFI_SSID       "YOUR_SSID_HERE"
#define WIFI_PASSWORD   "YOUR_PASSWORD_HERE"


/* =============================================================================
 * Time Configuration
 * =============================================================================
 *
 * India Standard Time:
 *
 *     UTC + 5:30
 *
 * We use POSIX TZ notation:
 *
 *     IST-5:30
 *
 * The negative sign is intentional in POSIX TZ syntax.
 *
 * =============================================================================
 */

#define TIMEZONE        "IST-5:30"


/* =============================================================================
 * Matrix Configuration
 * =============================================================================
 */

#define MATRIX_BRIGHTNESS    25


/* =============================================================================
 * Globals
 * =============================================================================
 */

static const char *TAG = "WIFI_CLOCK";

static led_strip_handle_t led_strip = NULL;

static bool wifi_connected = false;


/* =============================================================================
 * Clock Coordinate Mapping
 * =============================================================================
 *
 * Physical LED arrangement:
 *
 *     0  1  2  3  4
 *     9  8  7  6  5
 *    10 11 12 13 14
 *    19 18 17 16 15
 *    20 21 22 23 24
 *
 * This is the standard serpentine arrangement.
 *
 * x = 0..4
 * y = 0..4
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (y % 2 == 0) {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


/* =============================================================================
 * Set Pixel With 180-Degree Rotation
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
    /*
     * Ignore pixels outside the 5x5 matrix.
     */
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5) {
        return;
    }


    /*
     * Rotate the visual clock face by 180 degrees.
     */
    int flipped_x = 4 - x;
    int flipped_y = 4 - y;


    int pixel =
        pixel_xy(
            flipped_x,
            flipped_y
        );


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            pixel,
            red,
            green,
            blue
        )
    );
}


/* =============================================================================
 * Clock Perimeter
 * =============================================================================
 *
 * 16 positions around the perimeter.
 *
 * Index:
 *
 *              0  1  2
 *              15   3
 *           14       4
 *           13       5
 *           12       6
 *             11 10 9 8 7
 *
 * More precisely, using the arrays below:
 *
 *     0  = (2,0)
 *     1  = (3,0)
 *     2  = (4,0)
 *     3  = (4,1)
 *     ...
 *
 * =============================================================================
 */

static const int clock_x[16] =
{
    2, 3, 4, 4,
    4, 4, 4, 3,
    2, 1, 0, 0,
    0, 0, 0, 1
};


static const int clock_y[16] =
{
    0, 0, 0, 1,
    2, 3, 4, 4,
    4, 4, 4, 3,
    2, 1, 0, 0
};


/* =============================================================================
 * Draw Clock Hand
 * =============================================================================
 */

static void draw_hand(
    int perimeter_index,
    int length,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    int x =
        clock_x[perimeter_index];

    int y =
        clock_y[perimeter_index];


    int dx = 0;
    int dy = 0;


    /*
     * Direction from perimeter toward center.
     */
    if (x < 2) {
        dx = 1;
    }
    else if (x > 2) {
        dx = -1;
    }


    if (y < 2) {
        dy = 1;
    }
    else if (y > 2) {
        dy = -1;
    }


    /*
     * Draw the requested hand length.
     */
    for (int i = 0; i < length; i++) {

        int px =
            x + dx * i;

        int py =
            y + dy * i;


        set_pixel_xy(
            px,
            py,
            red,
            green,
            blue
        );
    }
}


/* =============================================================================
 * Draw Clock
 * =============================================================================
 */

static void draw_clock(
    int hour,
    int minute,
    int second
)
{
    /*
     * Clear previous frame.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    /* -------------------------------------------------------------------------
     * Colors
     * -------------------------------------------------------------------------
     */

    const uint8_t second_red   = 255;
    const uint8_t second_green = 0;
    const uint8_t second_blue  = 0;


    const uint8_t minute_red   = 0;
    const uint8_t minute_green = 255;
    const uint8_t minute_blue  = 0;


    const uint8_t hour_red   = 0;
    const uint8_t hour_green = 0;
    const uint8_t hour_blue  = 255;


    const uint8_t center_red   = 255;
    const uint8_t center_green = 255;
    const uint8_t center_blue  = 255;


    /* -------------------------------------------------------------------------
     * SECOND HAND
     * -------------------------------------------------------------------------
     *
     * 60 seconds -> 16 perimeter positions.
     *
     * Reverse the index because the visual clock is rotated 180 degrees.
     * This preserves clockwise movement.
     */

    float second_position =
        second * 16.0f / 60.0f;


    int second_index =
        (int)roundf(second_position) % 16;


    second_index =
        (16 - second_index) % 16;


    set_pixel_xy(
        clock_x[second_index],
        clock_y[second_index],
        second_red,
        second_green,
        second_blue
    );


    /* -------------------------------------------------------------------------
     * MINUTE HAND
     * -------------------------------------------------------------------------
     *
     * Include seconds so the minute hand moves smoothly between minute marks.
     */

    float minute_position =
        (minute + second / 60.0f) *
        16.0f /
        60.0f;


    int minute_index =
        (int)roundf(minute_position) % 16;


    minute_index =
        (16 - minute_index) % 16;


    draw_hand(
        minute_index,
        2,
        minute_red,
        minute_green,
        minute_blue
    );


    /* -------------------------------------------------------------------------
     * HOUR HAND
     * -------------------------------------------------------------------------
     *
     * 12 hours -> 16 perimeter positions.
     *
     * Include minutes so the hour hand progresses during the hour.
     */

    float hour12 =
        (hour % 12) +
        minute / 60.0f;


    float hour_position =
        hour12 *
        16.0f /
        12.0f;


    int hour_index =
        (int)roundf(hour_position) % 16;


    hour_index =
        (16 - hour_index) % 16;


    draw_hand(
        hour_index,
        1,
        hour_red,
        hour_green,
        hour_blue
    );


    /* -------------------------------------------------------------------------
     * CENTER
     * -------------------------------------------------------------------------
     */

    set_pixel_xy(
        2,
        2,
        center_red,
        center_green,
        center_blue
    );


    /* -------------------------------------------------------------------------
     * Send frame to matrix
     * -------------------------------------------------------------------------
     */

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/* =============================================================================
 * LED Matrix Initialization
 * =============================================================================
 */

static void led_strip_init_board(void)
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

        .resolution_hz =
            10 * 1000 * 1000,

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


    /*
     * Clear the matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "RGB matrix initialized on GPIO %d",
        MATRIX_PIN
    );
}


/* =============================================================================
 * Wi-Fi Event Handler
 * =============================================================================
 */

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START) {

        ESP_LOGI(
            TAG,
            "Wi-Fi started"
        );

        ESP_ERROR_CHECK(
            esp_wifi_connect()
        );
    }


    else if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED
    ) {

        wifi_connected = false;

        ESP_LOGW(
            TAG,
            "Wi-Fi disconnected - reconnecting..."
        );

        ESP_ERROR_CHECK(
            esp_wifi_connect()
        );
    }


    else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    ) {

        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;


        wifi_connected = true;


        ESP_LOGI(
            TAG,
            "Wi-Fi connected"
        );


        ESP_LOGI(
            TAG,
            "IP address: " IPSTR,
            IP2STR(&event->ip_info.ip)
        );
    }
}


/* =============================================================================
 * Wi-Fi Initialization
 * =============================================================================
 */

static void wifi_init(void)
{
    /*
     * Initialize TCP/IP stack.
     */
    ESP_ERROR_CHECK(
        esp_netif_init()
    );


    /*
     * Create default event loop.
     */
    ESP_ERROR_CHECK(
        esp_event_loop_create_default()
    );


    /*
     * Create default Wi-Fi station interface.
     */
    esp_netif_create_default_wifi_sta();


    /*
     * Initialize Wi-Fi driver.
     */
    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /*
     * Register Wi-Fi/IP event handlers.
     */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );


    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );


    /*
     * Configure station.
     */
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,

            .threshold.authmode =
                WIFI_AUTH_WPA2_PSK,

            .pmf_cfg = {
                .capable = true,
                .required = false,
            },
        },
    };


    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );


    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );


    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(
        TAG,
        "Connecting to Wi-Fi: %s",
        WIFI_SSID
    );


    /*
     * Wait until we actually have an IP address.
     */
    while (!wifi_connected) {

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }
}


/* =============================================================================
 * NTP / SNTP Initialization
 * =============================================================================
 */

static void time_init(void)
{
    /*
     * Configure timezone.
     *
     * "IST-5:30" means UTC+5:30 in POSIX TZ syntax.
     */
    setenv(
        "TZ",
        TIMEZONE,
        1
    );

    tzset();


    /*
     * Configure SNTP.
     *
     * ESP-IDF will synchronize system time from NTP.
     */
    esp_sntp_config_t config =
        ESP_NETIF_SNTP_DEFAULT_CONFIG(
            "pool.ntp.org"
        );


    ESP_ERROR_CHECK(
        esp_netif_sntp_init(&config)
    );


    ESP_LOGI(
        TAG,
        "Waiting for NTP time..."
    );


    /*
     * Wait for synchronization.
     */
    while (!esp_netif_sntp_sync_wait(
        pdMS_TO_TICKS(1000)
    )) {

        ESP_LOGI(
            TAG,
            "Waiting for NTP synchronization..."
        );
    }


    ESP_LOGI(
        TAG,
        "Time synchronized"
    );


    /*
     * Print current local time.
     */
    time_t now;

    struct tm timeinfo;


    time(&now);

    localtime_r(
        &now,
        &timeinfo
    );


    ESP_LOGI(
        TAG,
        "Current time: %02d:%02d:%02d",
        timeinfo.tm_hour,
        timeinfo.tm_min,
        timeinfo.tm_sec
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
    ESP_LOGI(TAG, " 5x5 RGB Wi-Fi Analog Clock");
    ESP_LOGI(TAG, "==============================================");


    /* -------------------------------------------------------------------------
     * Initialize NVS
     * -------------------------------------------------------------------------
     *
     * Wi-Fi uses NVS for persistent configuration/state.
     */

    esp_err_t ret =
        nvs_flash_init();


    if (ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {

        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ESP_ERROR_CHECK(
            nvs_flash_init()
        );
    }
    else {
        ESP_ERROR_CHECK(ret);
    }


    /* -------------------------------------------------------------------------
     * Initialize LED matrix
     * -------------------------------------------------------------------------
     */

    led_strip_init_board();


    /* -------------------------------------------------------------------------
     * Connect Wi-Fi
     * -------------------------------------------------------------------------
     */

    wifi_init();


    /* -------------------------------------------------------------------------
     * Synchronize time
     * -------------------------------------------------------------------------
     */

    time_init();


    ESP_LOGI(
        TAG,
        "Clock started"
    );


    /* -------------------------------------------------------------------------
     * Main Clock Loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        time_t now;

        struct tm timeinfo;


        /*
         * Get current Unix time.
         */
        time(&now);


        /*
         * Convert to local time.
         *
         * Because TZ is set to IST-5:30, this produces
         * India Standard Time.
         */
        localtime_r(
            &now,
            &timeinfo
        );


        int hour =
            timeinfo.tm_hour;

        int minute =
            timeinfo.tm_min;

        int second =
            timeinfo.tm_sec;


        ESP_LOGI(
            TAG,
            "%02d:%02d:%02d",
            hour,
            minute,
            second
        );


        /*
         * Draw clock.
         */
        draw_clock(
            hour,
            minute,
            second
        );


        /*
         * Update once per second.
         */
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}
