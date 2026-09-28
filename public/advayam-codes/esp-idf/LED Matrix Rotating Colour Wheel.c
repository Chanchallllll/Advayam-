#include <stdio.h>
#include <stdint.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_err.h"

#define MATRIX_PIN  7
#define NUM_LEDS    25

#define CONTROL_PIN 16

#define BRIGHTNESS  25

static led_strip_handle_t matrix;


/*
 * Convert HSV to RGB.
 *
 * hue:        0 - 65535
 * saturation: 0 - 255
 * value:      0 - 255
 *
 * Output RGB: 0 - 255
 */
static void hsv_to_rgb(
    uint16_t hue,
    uint8_t saturation,
    uint8_t value,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b
)
{
    if (saturation == 0)
    {
        *r = value;
        *g = value;
        *b = value;
        return;
    }

    uint32_t h = ((uint32_t)hue * 6) / 65536;

    uint32_t f =
        ((uint32_t)hue * 6 * 256) / 65536;

    f &= 0xFF;

    uint32_t p =
        ((uint32_t)value * (255 - saturation)) / 255;

    uint32_t q =
        ((uint32_t)value *
         (255 - ((uint32_t)saturation * f) / 255)) / 255;

    uint32_t t =
        ((uint32_t)value *
         (255 - ((uint32_t)saturation * (255 - f)) / 255)) / 255;

    switch (h)
    {
        case 0:
            *r = value;
            *g = t;
            *b = p;
            break;

        case 1:
            *r = q;
            *g = value;
            *b = p;
            break;

        case 2:
            *r = p;
            *g = value;
            *b = t;
            break;

        case 3:
            *r = p;
            *g = q;
            *b = value;
            break;

        case 4:
            *r = t;
            *g = p;
            *b = value;
            break;

        default:
            *r = value;
            *g = p;
            *b = q;
            break;
    }
}


/*
 * Equivalent to:
 *
 * matrix.ColorHSV(hue, 255, 255)
 *
 * followed by:
 *
 * matrix.setBrightness(25)
 */
static void set_pixel_hsv(
    int index,
    uint16_t hue
)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    hsv_to_rgb(
        hue,
        255,
        255,
        &r,
        &g,
        &b
    );

    /*
     * Adafruit_NeoPixel setBrightness(25)
     * scales the RGB output to approximately 25/255.
     */
    r = ((uint16_t)r * BRIGHTNESS) / 255;
    g = ((uint16_t)g * BRIGHTNESS) / 255;
    b = ((uint16_t)b * BRIGHTNESS) / 255;

    /*
     * WS2812 uses GRB internally, but the ESP-IDF
     * led_strip API takes RGB values here.
     */
    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            index,
            r,
            g,
            b
        )
    );
}


static void matrix_init(void)
{
    /*
     * Equivalent to:
     *
     * matrix.begin();
     * matrix.setBrightness(25);
     * matrix.clear();
     * matrix.show();
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
        .mem_block_symbols = 64,
        .flags = {
            .with_dma = false,
        },
    };

    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );

    /*
     * Clear all 25 LEDs.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );

    /*
     * Equivalent to matrix.show().
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


void app_main(void)
{
    /*
     * Arduino:
     *
     * pinMode(16, OUTPUT);
     * digitalWrite(16, LOW);
     */

    gpio_config_t control_gpio = {
        .pin_bit_mask = (1ULL << CONTROL_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&control_gpio)
    );

    ESP_ERROR_CHECK(
        gpio_set_level(CONTROL_PIN, 0)
    );


    /*
     * Initialize NeoPixel matrix.
     */
    matrix_init();


    /*
     * Arduino:
     *
     * static uint16_t offset = 0;
     */
    uint16_t offset = 0;


    while (1)
    {
        /*
         * Give every pixel a different hue.
         *
         * Arduino:
         *
         * for (int i = 0; i < NUM_LEDS; i++)
         * {
         *     uint16_t hue =
         *         (offset + i * 1024) % 65536;
         *
         *     matrix.setPixelColor(
         *         i,
         *         matrix.ColorHSV(hue, 255, 255)
         *     );
         * }
         */

        for (int i = 0; i < NUM_LEDS; i++)
        {
            uint16_t hue =
                (uint16_t)(
                    offset +
                    (uint16_t)(i * 1024)
                );

            set_pixel_hsv(i, hue);
        }


        /*
         * Arduino:
         *
         * matrix.show();
         */
        ESP_ERROR_CHECK(
            led_strip_refresh(matrix)
        );


        /*
         * Rotate the color pattern.
         *
         * Arduino:
         *
         * offset += 500;
         */
        offset += 500;


        /*
         * Arduino:
         *
         * delay(50);
         */
        vTaskDelay(
            pdMS_TO_TICKS(50)
        );
    }
}
