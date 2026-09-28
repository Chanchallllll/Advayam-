#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_err.h"

#include "driver/gpio.h"
#include "led_strip.h"


// ============================================================
// CONFIGURATION
// ============================================================

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define TOUCH_PIN           8

#define MATRIX_BRIGHTNESS   50


static const char *TAG = "strobe";


// ============================================================
// LED STRIP
// ============================================================

static led_strip_handle_t led_strip;


// ============================================================
// STROBE FREQUENCIES
// ============================================================

static const int frequencies[] = {
    1,
    2,
    3,
    5,
    7,
    10,
    12,
    15
};

#define NUM_FREQUENCIES \
    (sizeof(frequencies) / sizeof(frequencies[0]))


// ============================================================
// STATE
// ============================================================

static int frequency_index = 0;

static bool strobe_enabled = false;

static bool flash_state = false;

static bool last_touch_state = true;


// ============================================================
// SET MATRIX COLOR
// ============================================================

static void set_matrix(uint8_t r, uint8_t g, uint8_t b)
{
    // Apply brightness
    r = ((uint16_t)r * MATRIX_BRIGHTNESS) / 255;
    g = ((uint16_t)g * MATRIX_BRIGHTNESS) / 255;
    b = ((uint16_t)b * MATRIX_BRIGHTNESS) / 255;

    for (int i = 0; i < NUM_LEDS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                r,
                g,
                b
            )
        );
    }

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


// ============================================================
// INITIALIZE LED STRIP
// ============================================================

static void init_led_strip(void)
{
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

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


// ============================================================
// INITIALIZE TOUCH INPUT
// ============================================================

static void init_touch_sensor(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << TOUCH_PIN),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&io_conf)
    );
}


// ============================================================
// HANDLE TOUCH
// ============================================================

static void handle_touch(void)
{
    int level = gpio_get_level(TOUCH_PIN);

    // HIGH = not touched
    // LOW  = touched
    bool touch_state = (level == 1);


    // Detect HIGH -> LOW transition
    if (last_touch_state && !touch_state) {

        // ----------------------------------------
        // Currently OFF
        // ----------------------------------------

        if (!strobe_enabled) {

            strobe_enabled = true;

            frequency_index = 0;

            flash_state = false;

            ESP_LOGI(
                TAG,
                "Strobe: ON, %d Hz",
                frequencies[frequency_index]
            );
        }

        // ----------------------------------------
        // Currently ON
        // ----------------------------------------

        else {

            frequency_index++;

            // Finished all frequencies
            if (frequency_index >= NUM_FREQUENCIES) {

                strobe_enabled = false;

                frequency_index = 0;

                flash_state = false;

                set_matrix(0, 0, 0);

                ESP_LOGI(
                    TAG,
                    "Strobe: OFF"
                );
            }

            else {

                ESP_LOGI(
                    TAG,
                    "Strobe: ON, %d Hz",
                    frequencies[frequency_index]
                );
            }
        }
    }

    last_touch_state = touch_state;
}


// ============================================================
// MAIN
// ============================================================

void app_main(void)
{
    init_touch_sensor();

    init_led_strip();

    ESP_LOGI(
        TAG,
        "LED Matrix Strobe"
    );

    ESP_LOGI(
        TAG,
        "Touch sensor on GPIO %d",
        TOUCH_PIN
    );

    ESP_LOGI(
        TAG,
        "Touch to enable/change frequency."
    );


    // Make sure LEDs start OFF
    set_matrix(0, 0, 0);


    // --------------------------------------------------------
    // FreeRTOS periodic loop
    // --------------------------------------------------------

    TickType_t last_wake_time = xTaskGetTickCount();

    TickType_t last_flash_time = last_wake_time;


    while (1) {

        // ====================================================
        // TOUCH
        // ====================================================

        handle_touch();


        // ====================================================
        // STROBE
        // ====================================================

        if (strobe_enabled) {

            int frequency =
                frequencies[frequency_index];


            // ON + OFF = one complete cycle
            //
            // Therefore transition interval is:
            //
            // 1000 / (frequency * 2)

            uint32_t interval_ms =
                1000 / (frequency * 2);


            TickType_t now =
                xTaskGetTickCount();


            if ((now - last_flash_time) >=
                pdMS_TO_TICKS(interval_ms)) {

                last_flash_time = now;

                flash_state = !flash_state;


                if (flash_state) {

                    // ON
                    set_matrix(
                        255,
                        255,
                        255
                    );

                }
                else {

                    // OFF
                    set_matrix(
                        0,
                        0,
                        0
                    );
                }
            }
        }

        else {

            // Keep timer synchronized while OFF
            last_flash_time =
                xTaskGetTickCount();
        }


        // ====================================================
        // GIVE CPU TIME TO IDLE TASK
        // ====================================================

        vTaskDelayUntil(
            &last_wake_time,
            pdMS_TO_TICKS(10)
        );
    }
}
