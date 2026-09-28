/*
 * =============================================================================
 * TUG OF WAR - PLAYER VS CPU
 * =============================================================================
 *
 * ESP-IDF v5+
 *
 * PLAYER:
 *   Press the button repeatedly to pull the marker toward the player.
 *
 * CPU:
 *   Automatically pulls toward its side.
 *
 * WIN CONDITION:
 *   Player must keep the marker on position 0 or 1
 *   continuously for 3 seconds.
 *
 * GPIO:
 *   RGB Matrix : GPIO 7
 *   Button     : GPIO 8
 *   Buzzer     : GPIO 16
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

#include "led_strip.h"
#include "esp_log.h"
#include "esp_timer.h"


//=============================================================================
// Configuration
//=============================================================================

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define BUTTON_PIN          8
#define BUZZER_PIN          16

#define MATRIX_BRIGHTNESS   10


static const char *TAG = "TUG_OF_WAR";


//=============================================================================
// LED Strip
//=============================================================================

static led_strip_handle_t matrix = NULL;


//=============================================================================
// Game Variables
//=============================================================================

static int marker = 2;


// CPU starts at 450 ms.
static uint32_t cpu_interval = 450;

// CPU cannot become faster than this.
static const uint32_t MIN_CPU_INTERVAL = 280;

// CPU speed increase after each player press.
static const uint32_t CPU_SPEEDUP = 8;


// Time of last CPU movement.
static int64_t last_cpu = 0;


//=============================================================================
// Player Hold
//=============================================================================

static bool holding_player_side = false;

static int64_t player_hold_start = 0;

// Player must hold for 3 seconds.
static const uint32_t PLAYER_HOLD_TIME = 3000;


//=============================================================================
// Button State
//=============================================================================

static bool last_button_state = true;


//=============================================================================
// Matrix Coordinate Mapping
//=============================================================================

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}


//=============================================================================
// Set Pixel
//=============================================================================

static void set_pixel(
    int index,
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    if (index < 0 || index >= NUM_LEDS)
        return;

    /*
     * Apply global brightness manually.
     */
    r = ((uint16_t)r * MATRIX_BRIGHTNESS) / 255;
    g = ((uint16_t)g * MATRIX_BRIGHTNESS) / 255;
    b = ((uint16_t)b * MATRIX_BRIGHTNESS) / 255;

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


//=============================================================================
// Matrix Drawing
//=============================================================================

static void draw(void)
{
    /*
     * Clear matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    //-------------------------------------------------------------------------
    // Player side - GREEN
    //-------------------------------------------------------------------------

    for (int x = 0; x < marker; x++)
    {
        set_pixel(
            pixel_xy(x, 2),
            0,
            255,
            0
        );
    }


    //-------------------------------------------------------------------------
    // CPU side - RED
    //-------------------------------------------------------------------------

    for (int x = marker + 1; x < 5; x++)
    {
        set_pixel(
            pixel_xy(x, 2),
            255,
            0,
            0
        );
    }


    //-------------------------------------------------------------------------
    // Marker - WHITE
    //-------------------------------------------------------------------------

    set_pixel(
        pixel_xy(marker, 2),
        255,
        255,
        255
    );


    /*
     * Send data to matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//=============================================================================
// Buzzer
//=============================================================================

static void buzzer_init(void)
{
    /*
     * Configure LEDC timer.
     */
    ledc_timer_config_t timer_config = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 2000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    /*
     * Configure buzzer channel.
     */
    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .intr_type = LEDC_INTR_DISABLE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );
}


//=============================================================================
// Beep
//=============================================================================

static void beep(
    uint32_t frequency,
    uint32_t duration_ms
)
{
    if (frequency == 0)
    {
        vTaskDelay(
            pdMS_TO_TICKS(duration_ms)
        );

        return;
    }


    /*
     * Change buzzer frequency.
     */
    ESP_ERROR_CHECK(
        ledc_set_freq(
            LEDC_LOW_SPEED_MODE,
            LEDC_TIMER_0,
            frequency
        )
    );


    /*
     * 50% duty cycle.
     *
     * 10-bit resolution = 0..1023
     */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            512
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        )
    );


    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );


    /*
     * Turn buzzer off.
     */
    ESP_ERROR_CHECK(
        ledc_set_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0,
            0
        )
    );

    ESP_ERROR_CHECK(
        ledc_update_duty(
            LEDC_LOW_SPEED_MODE,
            LEDC_CHANNEL_0
        )
    );
}


//=============================================================================
// Reset Game
//=============================================================================

static void reset_game(void)
{
    marker = 2;

    cpu_interval = 450;

    last_cpu = esp_timer_get_time();

    holding_player_side = false;

    player_hold_start = 0;

    draw();
}


//=============================================================================
// End Game
//=============================================================================

static void end_game(bool player_won)
{
    uint8_t r;
    uint8_t g;
    uint8_t b;

    if (player_won)
    {
        r = 0;
        g = 255;
        b = 0;
    }
    else
    {
        r = 255;
        g = 0;
        b = 0;
    }


    //-------------------------------------------------------------------------
    // Flash result 3 times
    //-------------------------------------------------------------------------

    for (int i = 0; i < 3; i++)
    {
        /*
         * Turn all pixels on.
         */
        for (int j = 0; j < NUM_LEDS; j++)
        {
            set_pixel(
                j,
                r,
                g,
                b
            );
        }

        ESP_ERROR_CHECK(
            led_strip_refresh(matrix)
        );


        /*
         * Player = high beep.
         * CPU = low beep.
         */
        if (player_won)
            beep(1200, 120);
        else
            beep(150, 120);


        /*
         * Turn LEDs off.
         */
        ESP_ERROR_CHECK(
            led_strip_clear(matrix)
        );

        ESP_ERROR_CHECK(
            led_strip_refresh(matrix)
        );


        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }


    vTaskDelay(
        pdMS_TO_TICKS(500)
    );


    reset_game();
}


//=============================================================================
// Player Pull
//=============================================================================

static void player_pull(void)
{
    //-------------------------------------------------------------------------
    // Pull toward player.
    //-------------------------------------------------------------------------

    if (marker > 0)
    {
        marker--;

        beep(
            900,
            30
        );


        //-------------------------------------------------------------------------
        // Make CPU faster.
        //-------------------------------------------------------------------------

        if (cpu_interval > MIN_CPU_INTERVAL)
        {
            if (cpu_interval > CPU_SPEEDUP)
            {
                cpu_interval -= CPU_SPEEDUP;
            }
        }
    }


    //-------------------------------------------------------------------------
    // Reaching position 0 starts the hold timer.
    //-------------------------------------------------------------------------

    if (marker <= 0)
    {
        marker = 0;


        if (!holding_player_side)
        {
            holding_player_side = true;

            player_hold_start =
                esp_timer_get_time();


            ESP_LOGI(
                TAG,
                "PLAYER SIDE! Hold for 3 seconds!"
            );


            beep(
                1100,
                50
            );
        }
    }
}


//=============================================================================
// Check Player Hold
//=============================================================================

static bool check_player_hold(void)
{
    if (!holding_player_side)
        return false;


    //-------------------------------------------------------------------------
    // CPU moved marker away from player.
    //-------------------------------------------------------------------------

    if (marker > 1)
    {
        holding_player_side = false;

        player_hold_start = 0;


        ESP_LOGI(
            TAG,
            "Hold interrupted!"
        );


        return false;
    }


    //-------------------------------------------------------------------------
    // Check 3-second hold.
    //-------------------------------------------------------------------------

    int64_t elapsed_us =
        esp_timer_get_time() - player_hold_start;


    if (elapsed_us >=
        ((int64_t)PLAYER_HOLD_TIME * 1000))
    {
        ESP_LOGI(
            TAG,
            "PLAYER WINS!"
        );


        end_game(true);

        return true;
    }


    return false;
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
            &matrix
        )
    );


    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


//=============================================================================
// Button Initialization
//=============================================================================

static void button_init(void)
{
    /*
     * Same approach as your working binary counter:
     *
     * GPIO 8
     * INPUT
     * Internal pull-up
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
// Main
//=============================================================================

void app_main(void)
{
    //-------------------------------------------------------------------------
    // Initialize button.
    //-------------------------------------------------------------------------

    button_init();


    //-------------------------------------------------------------------------
    // Initialize buzzer.
    //-------------------------------------------------------------------------

    buzzer_init();


    //-------------------------------------------------------------------------
    // Initialize matrix.
    //-------------------------------------------------------------------------

    matrix_init();


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       TUG OF WAR - PLAYER VS CPU"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "Button: GPIO %d",
        BUTTON_PIN
    );

    ESP_LOGI(
        TAG,
        "Matrix: GPIO %d",
        MATRIX_PIN
    );

    ESP_LOGI(
        TAG,
        "Buzzer: GPIO %d",
        BUZZER_PIN
    );


    reset_game();


    //-------------------------------------------------------------------------
    // Main Game Loop
    //-------------------------------------------------------------------------

    while (1)
    {
        //=====================================================================
        // BUTTON
        //=====================================================================

        int button_level =
            gpio_get_level(BUTTON_PIN);


        /*
         * With INPUT_PULLUP:
         *
         * HIGH = released
         * LOW  = pressed
         */
        bool button_state =
            (button_level == 1);


        /*
         * Detect HIGH -> LOW transition.
         */
        if (last_button_state &&
            !button_state)
        {
            player_pull();
        }


        last_button_state =
            button_state;


        //=====================================================================
        // CPU
        //=====================================================================

        int64_t now =
            esp_timer_get_time();


        int64_t cpu_elapsed =
            now - last_cpu;


        if (cpu_elapsed >=
            ((int64_t)cpu_interval * 1000))
        {
            last_cpu = now;


            /*
             * CPU pulls toward position 4.
             */
            if (marker < 4)
            {
                marker++;


                /*
                 * Break player's hold.
                 */
                if (marker > 1 &&
                    holding_player_side)
                {
                    holding_player_side = false;

                    player_hold_start = 0;


                    ESP_LOGI(
                        TAG,
                        "CPU broke the hold!"
                    );
                }
            }


            //=================================================================
            // CPU WINS
            //=================================================================

            if (marker >= 4)
            {
                marker = 4;


                ESP_LOGI(
                    TAG,
                    "CPU WINS!"
                );


                end_game(false);

                continue;
            }
        }


        //=====================================================================
        // PLAYER HOLD
        //=====================================================================

        if (check_player_hold())
        {
            continue;
        }


        //=====================================================================
        // DISPLAY
        //=====================================================================

        draw();


        /*
         * Similar to Arduino delay(10).
         */
        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
