/*
 * =============================================================================
 * PARTICLE FOUNTAIN - ESP-IDF
 * =============================================================================
 *
 * Creates a particle fountain from the bottom-center of the 5x5 RGB matrix.
 *
 * Particles:
 *   - Start at the bottom-center
 *   - Rise upward
 *   - Drift left/right
 *   - Slow down due to gravity
 *   - Gradually fade
 *   - Restart when they leave the matrix
 *
 * Hardware:
 *   RGB Matrix data = GPIO 7
 *   Matrix size     = 5x5 / 25 LEDs
 *
 * ESP-IDF v5/v6 compatible.
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_random.h"

#include "led_strip.h"

/*
 * =============================================================================
 * Hardware Configuration
 * =============================================================================
 */

#define MATRIX_PIN      7
#define NUM_LEDS        25
#define NUM_PIXELS      25

#define MATRIX_WIDTH    5
#define MATRIX_HEIGHT   5

/*
 * Number of simulated particles.
 */
#define PARTICLES       8

/*
 * Matrix brightness.
 *
 * Equivalent to:
 *
 *     matrix.setBrightness(25);
 *
 * in the Arduino version.
 *
 * IMPORTANT:
 * The ESP-IDF led_strip component does not have an equivalent global
 * setBrightness() function, so brightness is applied directly to the
 * RGB values below.
 */
#define MATRIX_BRIGHTNESS 25


static const char *TAG = "PARTICLE_FOUNTAIN";

static led_strip_handle_t led_strip = NULL;


/*
 * =============================================================================
 * Particle Structure
 * =============================================================================
 */

typedef struct
{
    float x;
    float y;

    float dx;
    float dy;

    uint8_t brightness;

} Particle;


static Particle particles[PARTICLES];


/*
 * =============================================================================
 * Matrix Coordinate Mapping
 * =============================================================================
 *
 * Physical LED arrangement:
 *
 *     20 21 22 23 24
 *     19 18 17 16 15
 *     10 11 12 13 14
 *      9  8  7  6  5
 *      0  1  2  3  4
 *
 * This is a serpentine matrix.
 *
 * y = 0 -> left to right
 * y = 1 -> right to left
 * y = 2 -> left to right
 * etc.
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if ((y % 2) == 0)
    {
        return y * MATRIX_WIDTH + x;
    }

    return y * MATRIX_WIDTH +
           (MATRIX_WIDTH - 1 - x);
}


/*
 * =============================================================================
 * Random Float Helper
 * =============================================================================
 *
 * Returns a pseudo-random floating-point value between min and max.
 * =============================================================================
 */

static float random_float(float min, float max)
{
    uint32_t random_value = esp_random();

    float normalized =
        (float)random_value / (float)UINT32_MAX;

    return min + normalized * (max - min);
}


/*
 * =============================================================================
 * Reset Particle
 * =============================================================================
 */

static void reset_particle(int i)
{
    /*
     * Start at bottom center.
     *
     * Same as Arduino:
     *
     *     x = 2;
     *     y = 0;
     */
    particles[i].x = 2.0f;
    particles[i].y = 0.0f;


    /*
     * Random horizontal drift.
     *
     * Arduino:
     *
     *     random(-10, 11) / 20.0;
     *
     * Equivalent range:
     *
     *     -0.5 ... +0.5
     */
    particles[i].dx =
        random_float(-0.5f, 0.5f);


    /*
     * Initial upward velocity.
     *
     * Arduino:
     *
     *     random(10, 20) / 10.0;
     *
     * Range:
     *
     *     1.0 ... 2.0
     */
    particles[i].dy =
        random_float(1.0f, 2.0f);


    /*
     * Initial brightness.
     *
     * Arduino:
     *
     *     random(150, 256);
     *
     * Range:
     *
     *     150 ... 255
     */
    particles[i].brightness =
        (uint8_t)random_float(150.0f, 256.0f);


    /*
     * Optional debug output.
     */
    ESP_LOGD(
        TAG,
        "Particle %d reset: x=%.2f y=%.2f dx=%.2f dy=%.2f brightness=%d",
        i,
        particles[i].x,
        particles[i].y,
        particles[i].dx,
        particles[i].dy,
        particles[i].brightness
    );
}


/*
 * =============================================================================
 * LED Strip Initialization
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

        /*
         * 10 MHz RMT resolution.
         */
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


/*
 * =============================================================================
 * Apply Brightness
 * =============================================================================
 *
 * Arduino's:
 *
 *     matrix.setBrightness(25)
 *
 * scales the RGB output globally.
 *
 * Here we perform the same scaling ourselves.
 * =============================================================================
 */

static uint8_t scale_brightness(uint8_t value)
{
    return (uint8_t)(
        ((uint16_t)value * MATRIX_BRIGHTNESS) / 255
    );
}


/*
 * =============================================================================
 * Main Particle Fountain
 * =============================================================================
 */

static void particle_fountain(void)
{
    /*
     * Clear previous frame.
     */
    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    /*
     * Update every particle.
     */
    for (int i = 0; i < PARTICLES; i++)
    {
        /*
         * ---------------------------------------------------------
         * Update position
         * ---------------------------------------------------------
         */

        particles[i].x += particles[i].dx;
        particles[i].y += particles[i].dy;


        /*
         * ---------------------------------------------------------
         * Gravity
         * ---------------------------------------------------------
         *
         * Same as Arduino:
         *
         *     particles[i].dy -= 0.08;
         * ---------------------------------------------------------
         */

        particles[i].dy -= 0.08f;


        /*
         * ---------------------------------------------------------
         * Fade particle
         * ---------------------------------------------------------
         *
         * Same behaviour as Arduino:
         *
         *     if brightness > 10
         *         brightness -= 8
         *     else
         *         brightness = 0
         * ---------------------------------------------------------
         */

        if (particles[i].brightness > 10)
        {
            particles[i].brightness -= 8;
        }
        else
        {
            particles[i].brightness = 0;
        }


        /*
         * ---------------------------------------------------------
         * Convert floating point position to pixel coordinate
         * ---------------------------------------------------------
         */

        int x = (int)roundf(
            particles[i].x
        );

        int y = (int)roundf(
            particles[i].y
        );


        /*
         * ---------------------------------------------------------
         * Draw particle if it is inside the matrix
         * ---------------------------------------------------------
         */

        if (x >= 0 &&
            x < MATRIX_WIDTH &&
            y >= 0 &&
            y < MATRIX_HEIGHT)
        {
            uint8_t b =
                particles[i].brightness;


            /*
             * Original Arduino colour:
             *
             *     R = brightness
             *     G = brightness / 3
             *     B = 0
             *
             * This produces an orange/yellow particle.
             */

            uint8_t red =
                scale_brightness(b);

            uint8_t green =
                scale_brightness(b / 3);

            uint8_t blue = 0;


            int led_index =
                pixel_xy(x, y);


            ESP_ERROR_CHECK(
                led_strip_set_pixel(
                    led_strip,
                    led_index,
                    red,
                    green,
                    blue
                )
            );
        }


        /*
         * ---------------------------------------------------------
         * Restart finished particles
         * ---------------------------------------------------------
         */

        if (particles[i].y > MATRIX_HEIGHT ||
            particles[i].x < -1.0f ||
            particles[i].x > MATRIX_WIDTH ||
            particles[i].brightness == 0)
        {
            reset_particle(i);
        }
    }


    /*
     * Send frame to matrix.
     */
    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );
}


/*
 * =============================================================================
 * Application Entry Point
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(
        TAG,
        "=== PARTICLE FOUNTAIN STARTED ==="
    );


    /*
     * Initialize WS2812 matrix.
     */
    led_strip_init_board();


    /*
     * Initialize particles.
     */
    for (int i = 0; i < PARTICLES; i++)
    {
        reset_particle(i);
    }


    /*
     * Main animation loop.
     *
     * Original Arduino:
     *
     *     delay(80);
     *
     * becomes:
     *
     *     vTaskDelay(pdMS_TO_TICKS(80));
     */
    while (1)
    {
        particle_fountain();

        vTaskDelay(
            pdMS_TO_TICKS(80)
        );
    }
}
