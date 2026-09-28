/*
 * =============================================================================
 * NTP CLOCK - IST - UPSIDE-DOWN SCROLLING TIME
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Displays the current time as HH:MM:SS and scrolls it from right to left
 * across a 5x5 WS2812 RGB matrix.
 *
 * Matrix:
 *
 *     Data = GPIO 7
 *     LEDs = 25
 *
 * Physical matrix wiring:
 *
 *     20 21 22 23 24
 *     19 18 17 16 15
 *     10 11 12 13 14
 *      9  8  7  6  5
 *      0  1  2  3  4
 *
 * The numbers are vertically flipped so that the displayed text is
 * upside-down without being horizontally mirrored.
 *
 * Time:
 *
 *     NTP
 *     India Standard Time
 *     UTC + 5:30
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
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

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define MATRIX_BRIGHTNESS   25


/* =============================================================================
 * Wi-Fi Configuration
 * =============================================================================
 *
 * Replace these with your own credentials.
 *
 * For a cleaner project, these can later be moved into menuconfig.
 * =============================================================================
 */

#define WIFI_SSID       "YOUR WIFI SSID"
#define WIFI_PASSWORD   "YOUR WIFI PASSWORD"


/* =============================================================================
 * Time Configuration
 * =============================================================================
 *
 * India Standard Time = UTC + 5:30
 *
 * POSIX TZ notation uses the opposite sign convention:
 *
 *     IST-5:30
 *
 * =============================================================================
 */

#define TIMEZONE        "IST-5:30"


/* =============================================================================
 * Globals
 * =============================================================================
 */

static const char *TAG = "NTP_SCROLL";

static led_strip_handle_t led_strip = NULL;

static bool wifi_connected = false;


/* =============================================================================
 * 5x5 Number Font
 * =============================================================================
 *
 * Each number consists of 5 rows x 5 columns.
 *
 * Bit 4 = left-most pixel
 * Bit 0 = right-most pixel
 *
 * =============================================================================
 */

static const uint8_t numbers[10][5] = {

    /* 0 */
    {
        0b01110,
        0b10001,
        0b10001,
        0b10001,
        0b01110
    },

    /* 1 */
    {
        0b00100,
        0b01100,
        0b00100,
        0b00100,
        0b01110
    },

    /* 2 */
    {
        0b01110,
        0b10001,
        0b00010,
        0b00100,
        0b11111
    },

    /* 3 */
    {
        0b11110,
        0b00001,
        0b01110,
        0b00001,
        0b11110
    },

    /* 4 */
    {
        0b10010,
        0b10010,
        0b11111,
        0b00010,
        0b00010
    },

    /* 5 */
    {
        0b11111,
        0b10000,
        0b11110,
        0b00001,
        0b11110
    },

    /* 6 */
    {
        0b01110,
        0b10000,
        0b11110,
        0b10001,
        0b01110
    },

    /* 7 */
    {
        0b11111,
        0b00001,
        0b00010,
        0b00100,
        0b00100
    },

    /* 8 */
    {
        0b01110,
        0b10001,
        0b01110,
        0b10001,
        0b01110
    },

    /* 9 */
    {
        0b01110,
        0b10001,
        0b01111,
        0b00001,
        0b01110
    }
};


/* =============================================================================
 * Convert X/Y Coordinates to LED Number
 * =============================================================================
 *
 * Physical matrix:
 *
 *     20 21 22 23 24
 *     19 18 17 16 15
 *     10 11 12 13 14
 *      9  8  7  6  5
 *      0  1  2  3  4
 *
 * This is a serpentine layout.
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    /*
     * Even rows:
     *
     *     left -> right
     */
    if ((y % 2) == 0) {
        return y * 5 + x;
    }


    /*
     * Odd rows:
     *
     *     right -> left
     */
    return y * 5 + (4 - x);
}


/* =============================================================================
 * Set Pixel Using X/Y Coordinates
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


    int pixel =
        pixel_xy(x, y);


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
 * Draw Number
 * =============================================================================
 *
 * The number is flipped vertically but NOT horizontally.
 *
 * This creates the same upside-down appearance as the Arduino version.
 *
 * x_offset:
 *
 *     Horizontal position of the number.
 *
 * =============================================================================
 */

static void draw_number(
    int number,
    int x_offset,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    /*
     * Safety check.
     */
    if (number < 0 || number > 9) {
        return;
    }


    for (int y = 0; y < 5; y++) {

        /*
         * Reverse the row order.
         *
         * Original:
         *
         *     row 0
         *     row 1
         *     row 2
         *     row 3
         *     row 4
         *
         * Becomes:
         *
         *     row 4
         *     row 3
         *     row 2
         *     row 1
         *     row 0
         */
        uint8_t row =
            numbers[number][4 - y];


        for (int x = 0; x < 5; x++) {

            /*
             * Keep horizontal orientation unchanged.
             *
             * Bit 4 is the left-most pixel.
             */
            if (row & (1 << (4 - x))) {

                set_pixel_xy(
                    x + x_offset,
                    y,
                    red,
                    green,
                    blue
                );
            }
        }
    }
}


/* =============================================================================
 * Draw Colon
 * =============================================================================
 */

static void draw_colon(
    int x_offset,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    /*
     * Top dot.
     */
    set_pixel_xy(
        x_offset + 2,
        1,
        red,
        green,
        blue
    );


    /*
     * Bottom dot.
     */
    set_pixel_xy(
        x_offset + 2,
        3,
        red,
        green,
        blue
    );
}


/* =============================================================================
 * Draw One Character
 * =============================================================================
 */

static void draw_character(
    char character,
    int x_offset,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    if (character >= '0' &&
        character <= '9') {

        draw_number(
            character - '0',
            x_offset,
            red,
            green,
            blue
        );
    }

    else if (character == ':') {

        draw_colon(
            x_offset,
            red,
            green,
            blue
        );
    }
}


/* =============================================================================
 * Scroll Text
 * =============================================================================
 *
 * Each character occupies:
 *
 *     5 pixels + 1 pixel spacing = 6 pixels
 *
 * For "HH:MM:SS":
 *
 *     8 characters x 6 = 48 pixels
 *
 * The text begins completely outside the right side and scrolls until it
 * has completely left the matrix on the left side.
 *
 * =============================================================================
 */

static void scroll_text(
    const char *text
)
{
    int text_length =
        strlen(text);


    int text_width =
        text_length * 6;


    /*
     * Color used by the original Arduino program:
     *
     *     R = 0
     *     G = 150
     *     B = 255
     *
     * Apply brightness scaling equivalent to:
     *
     *     matrix.setBrightness(25)
     */
    const uint8_t base_red = 0;
    const uint8_t base_green = 150;
    const uint8_t base_blue = 255;


    const uint8_t red =
        (uint8_t)(
            base_red *
            MATRIX_BRIGHTNESS /
            255
        );


    const uint8_t green =
        (uint8_t)(
            base_green *
            MATRIX_BRIGHTNESS /
            255
        );


    const uint8_t blue =
        (uint8_t)(
            base_blue *
            MATRIX_BRIGHTNESS /
            255
        );


    /*
     * Start just outside the right edge.
     *
     * x = 5 means the first character is immediately to the right
     * of the 5x5 matrix.
     */
    for (
        int offset = 5;
        offset > -text_width;
        offset--
    ) {

        /*
         * Clear previous frame.
         */
        ESP_ERROR_CHECK(
            led_strip_clear(led_strip)
        );


        /*
         * Draw each character.
         */
        for (int i = 0; i < text_length; i++) {

            int x_position =
                offset + i * 6;


            draw_character(
                text[i],
                x_position,
                red,
                green,
                blue
            );
        }


        /*
         * Send frame to WS2812 matrix.
         */
        ESP_ERROR_CHECK(
            led_strip_refresh(led_strip)
        );


        /*
         * Scroll speed:
         *
         * 150 ms per pixel.
         */
        vTaskDelay(
            pdMS_TO_TICKS(150)
        );
    }
}


/* =============================================================================
 * LED Matrix Initialization
 * =============================================================================
 */

static void led_strip_init_board(void)
{
    led_strip_config_t strip_config = {

        .strip_gpio_num =
            MATRIX_PIN,

        .max_leds =
            NUM_LEDS,

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
     * Start with LEDs OFF.
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
    /*
     * Wi-Fi station started.
     */
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


    /*
     * Wi-Fi disconnected.
     */
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


    /*
     * DHCP assigned an IP address.
     */
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
     * Create Wi-Fi station interface.
     */
    esp_netif_create_default_wifi_sta();


    /*
     * Initialize Wi-Fi driver.
     */
    wifi_init_config_t wifi_init_cfg =
        WIFI_INIT_CONFIG_DEFAULT();


    ESP_ERROR_CHECK(
        esp_wifi_init(&wifi_init_cfg)
    );


    /*
     * Register Wi-Fi event handler.
     */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            WIFI_EVENT,
            ESP_EVENT_ANY_ID,
            &wifi_event_handler,
            NULL
        )
    );


    /*
     * Register IP event handler.
     */
    ESP_ERROR_CHECK(
        esp_event_handler_register(
            IP_EVENT,
            IP_EVENT_STA_GOT_IP,
            &wifi_event_handler,
            NULL
        )
    );


    /*
     * Configure Wi-Fi station.
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


    /*
     * Set station mode.
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_mode(WIFI_MODE_STA)
    );


    /*
     * Apply configuration.
     */
    ESP_ERROR_CHECK(
        esp_wifi_set_config(
            WIFI_IF_STA,
            &wifi_config
        )
    );


    /*
     * Start Wi-Fi.
     */
    ESP_ERROR_CHECK(
        esp_wifi_start()
    );


    ESP_LOGI(
        TAG,
        "Connecting to Wi-Fi: %s",
        WIFI_SSID
    );


    /*
     * Wait until DHCP gives us an IP.
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
     * Configure India Standard Time.
     */
    setenv(
        "TZ",
        TIMEZONE,
        1
    );

    tzset();


    /*
     * Configure SNTP.
     */
    esp_sntp_config_t sntp_config =
        ESP_NETIF_SNTP_DEFAULT_CONFIG(
            "pool.ntp.org"
        );


    ESP_ERROR_CHECK(
        esp_netif_sntp_init(
            &sntp_config
        )
    );


    ESP_LOGI(
        TAG,
        "Waiting for NTP time..."
    );


    /*
     * Wait until system time has been synchronized.
     */
    while (!esp_netif_sntp_sync_wait(
        pdMS_TO_TICKS(1000)
    )) {

        ESP_LOGI(
            TAG,
            "Waiting..."
        );
    }


    ESP_LOGI(
        TAG,
        "NTP synchronized"
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
    ESP_LOGI(TAG, " NTP Clock - Upside Down Scrolling");
    ESP_LOGI(TAG, "==============================================");


    /* -------------------------------------------------------------------------
     * Initialize NVS
     * -------------------------------------------------------------------------
     *
     * Wi-Fi requires NVS.
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
     * Connect to Wi-Fi
     * -------------------------------------------------------------------------
     */

    wifi_init();


    /* -------------------------------------------------------------------------
     * Synchronize NTP
     * -------------------------------------------------------------------------
     */

    time_init();


    ESP_LOGI(
        TAG,
        "Clock started"
    );


    /* -------------------------------------------------------------------------
     * Main Loop
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
         * Convert UTC/system time to local IST.
         */
        localtime_r(
            &now,
            &timeinfo
        );


        /*
         * Format HH:MM:SS.
         */
        char time_string[9];


        strftime(
            time_string,
            sizeof(time_string),
            "%H:%M:%S",
            &timeinfo
        );


        /*
         * Print current time.
         */
        ESP_LOGI(
            TAG,
            "Time: %s",
            time_string
        );


        /*
         * Scroll the time across the matrix.
         */
        scroll_text(
            time_string
        );


        /*
         * Small pause before displaying the next time.
         */
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}
