/*
 * =============================================================================
 * BUTTON TOGGLE WITH AUDIBLE FEEDBACK - ESP-IDF
 * =============================================================================
 *
 * Each button press toggles the LED matrix ON or OFF.
 *
 * A short buzzer click provides audible feedback
 * for each button press.
 *
 * Hardware:
 *   Matrix  = GPIO 7
 *   Button  = GPIO 17
 *   Buzzer  = GPIO 16
 *
 * Matrix brightness:
 *   25 / 255 = approximately 10%
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "led_strip.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN      7
#define NUM_LEDS        25

#define BUTTON_PIN      8
#define BUZZER_PIN      16


//=============================================================================
// Settings
//=============================================================================

/*
 * Same brightness as Arduino:
 *
 * matrix.setBrightness(25);
 *
 * Arduino uses 0-255.
 */
#define MATRIX_BRIGHTNESS 25


//=============================================================================
// Globals
//=============================================================================

static const char *TAG = "BUTTON_TOGGLE";

static led_strip_handle_t led_strip = NULL;

static bool last_button = true;
static bool led_state = false;


//=============================================================================
// Brightness Scaling
//=============================================================================

static uint8_t scale_brightness(uint8_t value)
{
    return (uint8_t)(
        ((uint16_t)value * MATRIX_BRIGHTNESS) / 255
    );
}


//=============================================================================
// Matrix
//=============================================================================

static void set_matrix(
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    r = scale_brightness(r);
    g = scale_brightness(g);
    b = scale_brightness(b);

    for (int i = 0; i < NUM_LEDS; i++)
    {
        led_strip_set_pixel(
            led_strip,
            i,
            r,
            g,
            b
        );
    }

    led_strip_refresh(led_strip);
}


//=============================================================================
// Buzzer Click
//=============================================================================

/*
 * Generate a short 1200 Hz square-wave click.
 *
 * This avoids LEDC entirely and therefore does not require:
 *
 *     driver/ledc.h
 *
 * The Arduino code uses:
 *
 *     tone(BUZZER_PIN, 1200, 50);
 *
 * So this generates approximately the same 1200 Hz / 50 ms tone.
 */
static void click_sound(void)
{
    const uint32_t frequency = 1200;
    const uint32_t duration_ms = 50;

    const uint32_t half_period_us =
        1000000UL / (frequency * 2);

    const uint32_t total_us =
        duration_ms * 1000UL;

    uint32_t elapsed = 0;

    while (elapsed < total_us)
    {
        gpio_set_level(
            BUZZER_PIN,
            1
        );

        esp_rom_delay_us(
            half_period_us
        );

        gpio_set_level(
            BUZZER_PIN,
            0
        );

        esp_rom_delay_us(
            half_period_us
        );

        elapsed +=
            half_period_us * 2;
    }

    gpio_set_level(
        BUZZER_PIN,
        0
    );
}


//=============================================================================
// Matrix Initialization
//=============================================================================

static void setup_matrix(void)
{
    /*
     * Same LED strip configuration as your working ESP-IDF code.
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

    led_strip_clear(led_strip);

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// GPIO Initialization
//=============================================================================

static void setup_gpio(void)
{
    /*
     * Button:
     *
     * INPUT_PULLUP equivalent.
     *
     * HIGH = not pressed
     * LOW  = pressed
     */

    gpio_config_t button_config = {
        .pin_bit_mask =
            (1ULL << BUTTON_PIN),

        .mode =
            GPIO_MODE_INPUT,

        .pull_up_en =
            GPIO_PULLUP_ENABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&button_config)
    );


    /*
     * Buzzer output.
     */

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
}


//=============================================================================
// APP MAIN
//=============================================================================

void app_main(void)
{
    printf("\n");
    printf("=====================================\n");
    printf("      BUTTON TOGGLE\n");
    printf("=====================================\n");


    //-------------------------------------------------------------------------
    // Initialize GPIO
    //-------------------------------------------------------------------------

    setup_gpio();


    //-------------------------------------------------------------------------
    // Initialize LED matrix
    //-------------------------------------------------------------------------

    setup_matrix();


    //-------------------------------------------------------------------------
    // Initial LED state = OFF
    //-------------------------------------------------------------------------

    led_state = false;

    set_matrix(
        0,
        0,
        0
    );


    ESP_LOGI(
        TAG,
        "Button Toggle"
    );

    ESP_LOGI(
        TAG,
        "Press button to toggle LED."
    );


    //=========================================================================
    // Main Loop
    //=========================================================================

    while (1)
    {
        /*
         * Read button.
         *
         * GPIO_PULLUP means:
         *
         * HIGH = released
         * LOW  = pressed
         */

        bool button =
            gpio_get_level(BUTTON_PIN);


        //---------------------------------------------------------------------
        // Detect button press
        //---------------------------------------------------------------------

        if (
            last_button == true &&
            button == false
        )
        {
            //-----------------------------------------------------------------
            // Toggle state
            //-----------------------------------------------------------------

            led_state =
                !led_state;


            //-----------------------------------------------------------------
            // LED ON
            //-----------------------------------------------------------------

            if (led_state)
            {
                set_matrix(
                    0,
                    255,
                    0
                );

                ESP_LOGI(
                    TAG,
                    "LED: ON"
                );
            }


            //-----------------------------------------------------------------
            // LED OFF
            //-----------------------------------------------------------------

            else
            {
                set_matrix(
                    0,
                    0,
                    0
                );

                ESP_LOGI(
                    TAG,
                    "LED: OFF"
                );
            }


            //-----------------------------------------------------------------
            // Audible confirmation
            //-----------------------------------------------------------------

            click_sound();


            //-----------------------------------------------------------------
            // Same debounce delay as Arduino code
            //-----------------------------------------------------------------

            vTaskDelay(
                pdMS_TO_TICKS(150)
            );
        }


        //---------------------------------------------------------------------
        // Save previous button state
        //---------------------------------------------------------------------

        last_button =
            button;


        //---------------------------------------------------------------------
        // Small loop delay
        //---------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
