/*
 * =============================================================================
 * MATRIX ROW SCANNER - ESP-IDF v5+
 * =============================================================================
 *
 * Each button press moves a solid horizontal bar one row upward.
 *
 * Rows:
 *
 *   Row 1
 *   Row 2
 *   Row 3
 *   Row 4
 *   Row 5
 *   then back to Row 1
 *
 * Button:
 *   GPIO 8
 *   Internal pull-up enabled
 *   Button connects GPIO 8 to GND
 *
 * Matrix:
 *   GPIO 7
 *   25 WS2812 pixels
 *
 * Extra:
 *   GPIO 16 configured as output and held LOW
 *
 * Includes software button debouncing.
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "led_strip.h"
#include "esp_log.h"


//=============================================================================
// Configuration
//=============================================================================

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define BUTTON_PIN          8

#define EXTRA_PIN           16

#define MATRIX_BRIGHTNESS   25

#define DEBOUNCE_TIME_MS    50


static const char *TAG = "ROW_SCANNER";


//=============================================================================
// LED Matrix
//=============================================================================

static led_strip_handle_t led_strip = NULL;


//=============================================================================
// Button State
//=============================================================================

/*
 * INPUT_PULLUP:
 *
 * HIGH = button released
 * LOW  = button pressed
 */
static bool last_button_reading = true;

static bool button_state = true;


/*
 * Time when the raw button reading last changed.
 *
 * Uses FreeRTOS ticks instead of millis().
 */
static TickType_t last_debounce_time = 0;


//=============================================================================
// Current Row
//=============================================================================

static int row = 0;


//=============================================================================
// Pixel Mapping
//=============================================================================
/*
 * Matrix is wired in a serpentine pattern:
 *
 *   Row 0:  0  1  2  3  4
 *   Row 1:  9  8  7  6  5
 *   Row 2: 10 11 12 13 14
 *   Row 3: 19 18 17 16 15
 *   Row 4: 20 21 22 23 24
 *
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
// Set Pixel With Brightness Scaling
//=============================================================================

static void set_pixel(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (index < 0 || index >= NUM_LEDS)
    {
        return;
    }


    /*
     * Apply global brightness.
     *
     * ESP-IDF's led_strip driver doesn't have the same
     * setBrightness() function as Adafruit_NeoPixel,
     * so brightness is applied to RGB values here.
     */
    r = ((uint16_t)r * MATRIX_BRIGHTNESS) / 255;
    g = ((uint16_t)g * MATRIX_BRIGHTNESS) / 255;
    b = ((uint16_t)b * MATRIX_BRIGHTNESS) / 255;


    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            led_strip,
            index,
            r,
            g,
            b
        )
    );
}


//=============================================================================
// Draw Current Row
//=============================================================================

static void draw_row(void)
{
    /*
     * Clear all pixels.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    /*
     * Green/cyan color.
     *
     * Arduino equivalent:
     *
     * matrix.Color(0, 255, 80)
     */
    const uint8_t red   = 0;
    const uint8_t green = 255;
    const uint8_t blue  = 80;


    /*
     * Light all five pixels in the selected row.
     */
    for (int x = 0; x < 5; x++)
    {
        int index =
            pixel_xy(x, row);


        set_pixel(
            index,
            red,
            green,
            blue
        );
    }


    /*
     * Send data to the matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// Matrix Initialization
//=============================================================================

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,

        .led_pixel_format = LED_PIXEL_FORMAT_GRB,

        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false
        }
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        .resolution_hz = 10 * 1000 * 1000,

        .mem_block_symbols = 64,

        .flags = {
            .with_dma = false
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
     * Start with matrix off.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// Button Initialization
//=============================================================================

static void button_init(void)
{
    /*
     * Same configuration as your working
     * binary-counter program.
     *
     * GPIO 8:
     *
     * INPUT
     * Internal pull-up enabled
     */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&io_conf)
    );
}


//=============================================================================
// Extra GPIO Initialization
//=============================================================================

static void extra_pin_init(void)
{
    /*
     * Configure GPIO 16 as an output
     * and hold it LOW.
     */
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << EXTRA_PIN),

        .mode = GPIO_MODE_OUTPUT,

        .pull_up_en = GPIO_PULLUP_DISABLE,

        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(&io_conf)
    );


    ESP_ERROR_CHECK(
        gpio_set_level(
            EXTRA_PIN,
            0
        )
    );
}


//=============================================================================
// Main Application
//=============================================================================

void app_main(void)
{
    //-------------------------------------------------------------------------
    // Initialize GPIO 16.
    //-------------------------------------------------------------------------

    extra_pin_init();


    //-------------------------------------------------------------------------
    // Initialize button.
    //-------------------------------------------------------------------------

    button_init();


    //-------------------------------------------------------------------------
    // Initialize LED matrix.
    //-------------------------------------------------------------------------

    matrix_init();


    //-------------------------------------------------------------------------
    // Draw initial row.
    //-------------------------------------------------------------------------

    draw_row();


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "          MATRIX ROW SCANNER"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Matrix GPIO : %d",
        MATRIX_PIN
    );

    ESP_LOGI(
        TAG,
        "Button GPIO : %d",
        BUTTON_PIN
    );

    ESP_LOGI(
        TAG,
        "Extra GPIO  : %d (LOW)",
        EXTRA_PIN
    );

    ESP_LOGI(
        TAG,
        "Initial Row : %d",
        row + 1
    );


    //-------------------------------------------------------------------------
    // Main Loop
    //-------------------------------------------------------------------------

    while (1)
    {
        //=====================================================================
        // Read Button
        //=====================================================================

        int level =
            gpio_get_level(BUTTON_PIN);


        /*
         * Convert GPIO level to logical button state.
         *
         * HIGH = released
         * LOW  = pressed
         */
        bool reading =
            (level == 1);


        //=====================================================================
        // Raw Reading Changed
        //=====================================================================

        if (reading != last_button_reading)
        {
            /*
             * Restart debounce timer.
             */
            last_debounce_time =
                xTaskGetTickCount();
        }


        //=====================================================================
        // Debounce
        //=====================================================================

        TickType_t now =
            xTaskGetTickCount();


        TickType_t elapsed =
            now - last_debounce_time;


        if (pdTICKS_TO_MS(elapsed) >
            DEBOUNCE_TIME_MS)
        {
            /*
             * The button has remained stable
             * long enough.
             */
            if (reading != button_state)
            {
                button_state =
                    reading;


                //=============================================================
                // Button Pressed
                //=============================================================

                if (button_state == false)
                {
                    /*
                     * Move to next row.
                     */
                    row++;


                    /*
                     * Wrap after row 4.
                     */
                    if (row >= 5)
                    {
                        row = 0;
                    }


                    /*
                     * Update matrix.
                     */
                    draw_row();


                    ESP_LOGI(
                        TAG,
                        "Row: %d",
                        row + 1
                    );
                }
            }
        }


        //=====================================================================
        // Save Raw Reading
        //=====================================================================

        last_button_reading =
            reading;


        /*
         * Small task delay.
         *
         * This replaces the constantly-running
         * Arduino loop while still checking the
         * button frequently.
         */
        vTaskDelay(
            pdMS_TO_TICKS(5)
        );
    }
}
