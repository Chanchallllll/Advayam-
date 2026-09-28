/*
 * =============================================================================
 * WI-FI SIGNAL STRENGTH MATRIX - ESP-IDF
 * =============================================================================
 *
 * Connects to Wi-Fi and displays the signal strength as 1 to 4 bars
 * on the 5x5 WS2812B RGB matrix.
 *
 * Hardware:
 *   Matrix DATA = GPIO 7
 *
 * Matrix:
 *   25 LEDs
 *   GRB
 *   WS2812
 *
 * Brightness:
 *   25 / 255 (~10%)
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "nvs_flash.h"

#include "led_strip.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN      7
#define NUM_LEDS        25

#define MATRIX_BRIGHTNESS 25


//=============================================================================
// Wi-Fi Credentials
//=============================================================================

#define WIFI_SSID       "YOUR_WiFi_SSID"
#define WIFI_PASSWORD   "YOUR_WiFi_PASSWORD"


//=============================================================================
// Globals
//=============================================================================

static const char *TAG = "WIFI_MATRIX";

static led_strip_handle_t led_strip = NULL;


//=============================================================================
// Brightness
//=============================================================================

static uint8_t scale_brightness(uint8_t value)
{
    return (uint8_t)(
        ((uint16_t)value * MATRIX_BRIGHTNESS) / 255
    );
}


//=============================================================================
// Matrix Coordinate Mapping
//=============================================================================

/*
 * Same serpentine mapping as the Arduino code.
 */

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


//=============================================================================
// Set Pixel
//=============================================================================

static void set_pixel(
    int x,
    int y,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (
        x < 0 ||
        x >= 5 ||
        y < 0 ||
        y >= 5
    )
    {
        return;
    }

    r = scale_brightness(r);
    g = scale_brightness(g);
    b = scale_brightness(b);

    led_strip_set_pixel(
        led_strip,
        pixel_xy(x, y),
        r,
        g,
        b
    );
}


//=============================================================================
// Draw Signal Bars
//=============================================================================

static void draw_signal(int bars)
{
    /*
     * Clear matrix.
     */

    led_strip_clear(led_strip);


    /*
     * Original Arduino color:
     *
     * matrix.Color(0, 200, 50)
     *
     * Keep exactly the same color.
     */

    const uint8_t r = 0;
    const uint8_t g = 200;
    const uint8_t b = 50;


    /*
     * Four ascending signal bars.
     *
     * Bar 0 = height 1
     * Bar 1 = height 2
     * Bar 2 = height 3
     * Bar 3 = height 4
     *
     * X positions:
     *
     * 1, 2, 3, 4
     */

    for (int bar = 0; bar < 4; bar++)
    {
        int height =
            bar + 1;


        /*
         * Only draw bars that are available
         * according to the RSSI.
         */

        if (bar < bars)
        {
            for (int h = 0; h < height; h++)
            {
                set_pixel(
                    bar + 1,
                    h,
                    r,
                    g,
                    b
                );
            }
        }
    }


    /*
     * Send data to WS2812 matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// Wi-Fi Event Handler
//=============================================================================

static void wifi_event_handler(
    void *arg,
    esp_event_base_t event_base,
    int32_t event_id,
    void *event_data
)
{
    if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_START
    )
    {
        ESP_LOGI(
            TAG,
            "Wi-Fi started"
        );

        esp_wifi_connect();
    }

    else if (
        event_base == WIFI_EVENT &&
        event_id == WIFI_EVENT_STA_DISCONNECTED
    )
    {
        ESP_LOGW(
            TAG,
            "Wi-Fi disconnected, reconnecting..."
        );

        esp_wifi_connect();
    }

    else if (
        event_base == IP_EVENT &&
        event_id == IP_EVENT_STA_GOT_IP
    )
    {
        ip_event_got_ip_t *event =
            (ip_event_got_ip_t *)event_data;

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


//=============================================================================
// Wi-Fi Initialization
//=============================================================================

static void wifi_init(void)
{
    /*
     * Initialize NVS.
     *
     * ESP-IDF Wi-Fi requires NVS.
     */

    esp_err_t ret =
        nvs_flash_init();

    if (
        ret == ESP_ERR_NVS_NO_FREE_PAGES ||
        ret == ESP_ERR_NVS_NEW_VERSION_FOUND
    )
    {
        ESP_ERROR_CHECK(
            nvs_flash_erase()
        );

        ESP_ERROR_CHECK(
            nvs_flash_init()
        );
    }


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

    wifi_init_config_t cfg =
        WIFI_INIT_CONFIG_DEFAULT();

    ESP_ERROR_CHECK(
        esp_wifi_init(&cfg)
    );


    /*
     * Register Wi-Fi event handlers.
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
     * Configure Wi-Fi credentials.
     */

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASSWORD,
        },
    };


    /*
     * Set station mode.
     */

    ESP_ERROR_CHECK(
        esp_wifi_set_mode(
            WIFI_MODE_STA
        )
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
}


//=============================================================================
// Matrix Initialization
//=============================================================================

static void matrix_init(void)
{
    /*
     * EXACT same LED strip configuration
     * used in your previous working ESP-IDF code.
     */

    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
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


    /*
     * Start with matrix OFF.
     */

    led_strip_clear(
        led_strip
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(
            led_strip
        )
    );
}


//=============================================================================
// APP MAIN
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "Wi-Fi Signal Strength Matrix"
    );


    //-------------------------------------------------------------------------
    // Initialize LED matrix
    //-------------------------------------------------------------------------

    matrix_init();


    //-------------------------------------------------------------------------
    // Initialize Wi-Fi
    //-------------------------------------------------------------------------

    wifi_init();


    ESP_LOGI(
        TAG,
        "Connecting to Wi-Fi..."
    );


    //=========================================================================
    // Main Loop
    //=========================================================================

    while (1)
    {
        wifi_ap_record_t ap_info;


        //---------------------------------------------------------------------
        // Check Wi-Fi connection
        //---------------------------------------------------------------------

        wifi_mode_t mode;

        esp_wifi_get_mode(
            &mode
        );


        //---------------------------------------------------------------------
        // Read RSSI
        //---------------------------------------------------------------------

        esp_err_t err =
            esp_wifi_sta_get_ap_info(
                &ap_info
            );


        if (err == ESP_OK)
        {
            int rssi =
                ap_info.rssi;


            int bars;


            //-----------------------------------------------------------------
            // Same RSSI thresholds as Arduino version
            //-----------------------------------------------------------------

            if (rssi >= -55)
            {
                bars = 4;
            }

            else if (rssi >= -67)
            {
                bars = 3;
            }

            else if (rssi >= -80)
            {
                bars = 2;
            }

            else
            {
                bars = 1;
            }


            //-----------------------------------------------------------------
            // Serial output
            //-----------------------------------------------------------------

            ESP_LOGI(
                TAG,
                "RSSI: %d dBm | Bars: %d",
                rssi,
                bars
            );


            //-----------------------------------------------------------------
            // Draw matrix
            //-----------------------------------------------------------------

            draw_signal(
                bars
            );
        }

        else
        {
            /*
             * Not connected.
             *
             * Clear matrix.
             */

            ESP_LOGW(
                TAG,
                "Wi-Fi not connected"
            );

            led_strip_clear(
                led_strip
            );

            led_strip_refresh(
                led_strip
            );
        }


        //---------------------------------------------------------------------
        // Same 2-second update interval as Arduino
        //---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(2000)
        );
    }
}