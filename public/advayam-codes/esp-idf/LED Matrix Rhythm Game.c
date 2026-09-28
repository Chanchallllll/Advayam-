/*
 * =============================================================================
 * RHYTHM TAP REVOLUTION - ESP-IDF v6.0.2
 * =============================================================================
 *
 * Falling notes move toward the bottom row.
 *
 * Press the button when a note reaches the bottom.
 *
 * SCORING:
 *   Correct tap       -> score increases
 *   Consecutive hits  -> multiplier increases
 *   Higher score      -> notes fall faster
 *
 * MISSES:
 *   Missing a note slows the falling speed.
 *
 * SPECIAL BEHAVIOUR:
 *   Scores divisible by 5 produce a special buzzer tone.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * MATRIX:
 *   GPIO 7
 *
 * BUTTON:
 *   GPIO 17
 *
 * BUZZER:
 *   GPIO 16
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
#include "esp_err.h"

#include "led_strip.h"


//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN       7
#define BUTTON_PIN       17
#define BUZZER_PIN       16

#define NUM_PIXELS        25


//=============================================================================
// Buzzer configuration
//=============================================================================

#define BUZZER_LEDC_TIMER       LEDC_TIMER_0
#define BUZZER_LEDC_MODE        LEDC_LOW_SPEED_MODE
#define BUZZER_LEDC_CHANNEL     LEDC_CHANNEL_0

#define BUZZER_DUTY             512
#define BUZZER_RESOLUTION       LEDC_TIMER_10_BIT


//=============================================================================
// Game configuration
//=============================================================================

#define MAX_SLOWDOWN             1000

#define INITIAL_SPEED            600
#define MIN_SPEED                100
#define MAX_SPEED                1600


//=============================================================================
// Logging
//=============================================================================

static const char *TAG = "RHYTHM_TAP";


//=============================================================================
// LED strip
//=============================================================================

static led_strip_handle_t led_strip = NULL;


//=============================================================================
// Game variables
//=============================================================================

static int note_x = 0;
static int note_y = 0;

static int score = 0;
static int multiplier = 1;

static uint8_t note_red = 255;
static uint8_t note_green = 255;
static uint8_t note_blue = 255;

static TickType_t last_note_move = 0;

static int miss_count = 0;

static int miss_slowdown = 0;


//=============================================================================
// Matrix coordinate mapping
//=============================================================================

/*
 * Convert game coordinates to physical matrix coordinates.
 *
 * Game coordinates:
 *
 *       y = 4
 *       ↑
 *       |
 *       |
 *       y = 0
 *
 * The entire display is rotated 180 degrees.
 */

static int xy(int x, int y)
{
    int display_x = 4 - x;
    int display_y = 4 - y;

    if (display_y % 2 == 0) {
        return display_y * 5 + display_x;
    }

    return display_y * 5 + (4 - display_x);
}


//=============================================================================
// Matrix clear
//=============================================================================

static void matrix_clear(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


//=============================================================================
// Draw
//=============================================================================

static void draw(void)
{
    matrix_clear();

    int index = xy(note_x, note_y);

    led_strip_set_pixel(
        led_strip,
        index,
        note_red,
        note_green,
        note_blue
    );

    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


//=============================================================================
// Matrix initialization
//=============================================================================

static void matrix_init(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_PIXELS,

        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        }
    };

    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        .resolution_hz = 10 * 1000 * 1000,

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

    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );
}


//=============================================================================
// Buzzer initialization
//=============================================================================

static void buzzer_init(void)
{
    //-------------------------------------------------------------------------
    // LEDC timer
    //-------------------------------------------------------------------------

    ledc_timer_config_t timer_config = {
        .speed_mode = BUZZER_LEDC_MODE,

        .duty_resolution = BUZZER_RESOLUTION,

        .timer_num = BUZZER_LEDC_TIMER,

        .freq_hz = 1000,

        .clk_cfg = LEDC_AUTO_CLK,
    };

    ESP_ERROR_CHECK(
        ledc_timer_config(&timer_config)
    );


    //-------------------------------------------------------------------------
    // LEDC channel
    //-------------------------------------------------------------------------

    ledc_channel_config_t channel_config = {
        .gpio_num = BUZZER_PIN,

        .speed_mode = BUZZER_LEDC_MODE,

        .channel = BUZZER_LEDC_CHANNEL,

        .intr_type = LEDC_INTR_DISABLE,

        .timer_sel = BUZZER_LEDC_TIMER,

        .duty = 0,

        .hpoint = 0,

        .flags = {
            .output_invert = 0,
        }
    };

    ESP_ERROR_CHECK(
        ledc_channel_config(&channel_config)
    );
}


//=============================================================================
// Play buzzer tone
//=============================================================================

static void play_tone(
    uint32_t frequency,
    uint32_t duration_ms
)
{
    if (frequency == 0 || duration_ms == 0) {
        return;
    }


    //-------------------------------------------------------------------------
    // Change frequency
    //-------------------------------------------------------------------------

    ESP_ERROR_CHECK(
        ledc_set_freq(
            BUZZER_LEDC_MODE,
            BUZZER_LEDC_TIMER,
            frequency
        )
    );


    //-------------------------------------------------------------------------
    // Turn buzzer ON
    //-------------------------------------------------------------------------

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


    //-------------------------------------------------------------------------
    // Hold tone
    //-------------------------------------------------------------------------

    vTaskDelay(
        pdMS_TO_TICKS(duration_ms)
    );


    //-------------------------------------------------------------------------
    // Turn buzzer OFF
    //-------------------------------------------------------------------------

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


//=============================================================================
// Get falling speed
//=============================================================================

static int get_speed(void)
{
    /*
     * Higher score makes notes fall faster.
     *
     * Arduino:
     *
     *     int speed = 600 - score * 15;
     *
     * Misses make the game slower.
     */

    int speed =
        INITIAL_SPEED -
        score * 15;

    speed += miss_slowdown;


    //-------------------------------------------------------------------------
    // Clamp speed
    //-------------------------------------------------------------------------

    if (speed < MIN_SPEED) {
        speed = MIN_SPEED;
    }

    if (speed > MAX_SPEED) {
        speed = MAX_SPEED;
    }

    return speed;
}


//=============================================================================
// Generate random note
//=============================================================================

static void new_note(void)
{
    /*
     * Random X coordinate: 0...4
     */

    note_x =
        esp_random() % 5;


    /*
     * Start at the top of the game coordinate system.
     */

    note_y = 0;


    //-------------------------------------------------------------------------
    // Random RGB colour
    //-------------------------------------------------------------------------

    note_red =
        esp_random() & 0xFF;

    note_green =
        esp_random() & 0xFF;

    note_blue =
        esp_random() & 0xFF;


    /*
     * Start movement timer.
     */

    last_note_move =
        xTaskGetTickCount();
}


//=============================================================================
// Successful tap
//=============================================================================

static void successful_tap(void)
{
    score++;


    //-------------------------------------------------------------------------
    // Increase multiplier
    //-------------------------------------------------------------------------

    multiplier++;

    if (multiplier > 10) {
        multiplier = 10;
    }


    //-------------------------------------------------------------------------
    // Remove some slowdown
    //-------------------------------------------------------------------------

    if (miss_slowdown > 0) {

        miss_slowdown -= 50;

        if (miss_slowdown < 0) {
            miss_slowdown = 0;
        }
    }


    //-------------------------------------------------------------------------
    // Normal multiplier tone
    //-------------------------------------------------------------------------

    int tone_frequency =
        500 +
        multiplier * 100;

    play_tone(
        tone_frequency,
        80
    );


    //-------------------------------------------------------------------------
    // Serial output
    //-------------------------------------------------------------------------

    ESP_LOGI(
        TAG,
        "Score: %d | Multiplier: x%d | Speed: %d ms",
        score,
        multiplier,
        get_speed()
    );


    //-------------------------------------------------------------------------
    // Score milestone
    //-------------------------------------------------------------------------

    if (score % 5 == 0) {

        int duration =
            180 -
            score * 3;

        if (duration < 40) {
            duration = 40;
        }


        play_tone(
            1200 + score * 20,
            duration
        );


        ESP_LOGI(
            TAG,
            "*** SCORE MILESTONE! ***"
        );
    }
}


//=============================================================================
// Missed note
//=============================================================================

static void missed(void)
{
    //-------------------------------------------------------------------------
    // Reset multiplier
    //-------------------------------------------------------------------------

    multiplier = 1;


    //-------------------------------------------------------------------------
    // Increase miss counter
    //-------------------------------------------------------------------------

    miss_count++;


    //-------------------------------------------------------------------------
    // Slow game down
    //-------------------------------------------------------------------------

    miss_slowdown += 100;

    if (miss_slowdown > MAX_SLOWDOWN) {
        miss_slowdown = MAX_SLOWDOWN;
    }


    //-------------------------------------------------------------------------
    // Miss sound
    //-------------------------------------------------------------------------

    play_tone(
        180,
        150
    );


    ESP_LOGI(
        TAG,
        "Miss! Slowdown: +%d ms | Misses: %d",
        miss_slowdown,
        miss_count
    );
}


//=============================================================================
// Button initialization
//=============================================================================

static void button_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask = (1ULL << BUTTON_PIN),

        .mode = GPIO_MODE_INPUT,

        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,

        .intr_type = GPIO_INTR_DISABLE,
    };

    ESP_ERROR_CHECK(
        gpio_config(&config)
    );
}


//=============================================================================
// Setup
//=============================================================================

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       RHYTHM TAP REVOLUTION"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    //-------------------------------------------------------------------------
    // Initialize hardware
    //-------------------------------------------------------------------------

    button_init();

    buzzer_init();

    matrix_init();


    //-------------------------------------------------------------------------
    // Start first note
    //-------------------------------------------------------------------------

    new_note();

    draw();


    ESP_LOGI(
        TAG,
        "Game started"
    );


    //-------------------------------------------------------------------------
    // Main game loop
    //-------------------------------------------------------------------------

    bool old_button = true;


    while (1) {

        TickType_t now =
            xTaskGetTickCount();


        //=====================================================================
        // Read button
        //=====================================================================

        bool button =
            (gpio_get_level(BUTTON_PIN) != 0);


        /*
         * Button is active LOW.
         *
         * old_button == true
         * button == false
         *
         * means a new press.
         */

        if (!button && old_button) {

            //===============================================================
            // Correct timing
            //===============================================================

            if (note_y >= 3) {

                successful_tap();

                new_note();
            }

            //===============================================================
            // Button pressed too early
            //===============================================================

            else {

                missed();

                new_note();
            }
        }


        old_button = button;


        //=====================================================================
        // Move falling note
        //=====================================================================

        int speed_ms =
            get_speed();


        if (
            (now - last_note_move) >=
            pdMS_TO_TICKS(speed_ms)
        ) {

            last_note_move = now;

            note_y++;


            //=================================================================
            // Note reached bottom without being hit
            //=================================================================

            if (note_y > 4) {

                missed();

                new_note();
            }
        }


        //=====================================================================
        // Draw current frame
        //=====================================================================

        draw();


        //=====================================================================
        // Small loop delay
        //=====================================================================

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}
