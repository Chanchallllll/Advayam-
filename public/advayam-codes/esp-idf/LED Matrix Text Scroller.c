/*
 * =============================================================================
 * 5x5 RGB MATRIX - SCROLLING TEXT
 * =============================================================================
 *
 * ESP-IDF v5+
 *
 * Scrolls text horizontally across a 5x5 WS2812 RGB matrix.
 *
 * Supported:
 *   A-Z
 *   0-9
 *   Space
 *
 * Matrix:
 *   GPIO 7
 *   25 LEDs
 *   WS2812 / NeoPixel
 *   GRB
 *
 * Example message:
 *   "ADVAYAM"
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <ctype.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "led_strip.h"
#include "esp_log.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define MATRIX_BRIGHTNESS   25

#define SCROLL_DELAY_MS     120

#define MESSAGE             "ADVAYAM"


static const char *TAG = "SCROLL_TEXT";


/*
 * =============================================================================
 * LED Strip
 * =============================================================================
 */

static led_strip_handle_t matrix = NULL;


/*
 * =============================================================================
 * 5x5 Font
 * =============================================================================
 *
 * Each character consists of 5 columns.
 *
 * Bit 4 = top pixel
 * Bit 0 = bottom pixel
 *
 * Example A:
 *
 *   01110
 *   10001
 *   11111
 *   10001
 *   10001
 *
 * =============================================================================
 */

static const uint8_t font[36][5] =
{
    /* A */
    {0x0E, 0x11, 0x1F, 0x11, 0x11},

    /* B */
    {0x1E, 0x11, 0x1E, 0x11, 0x1E},

    /* C */
    {0x0F, 0x10, 0x10, 0x10, 0x0F},

    /* D */
    {0x1E, 0x11, 0x11, 0x11, 0x1E},

    /* E */
    {0x1F, 0x10, 0x1E, 0x10, 0x1F},

    /* F */
    {0x1F, 0x10, 0x1E, 0x10, 0x10},

    /* G */
    {0x0F, 0x10, 0x17, 0x11, 0x0F},

    /* H */
    {0x11, 0x11, 0x1F, 0x11, 0x11},

    /* I */
    {0x1F, 0x04, 0x04, 0x04, 0x1F},

    /* J */
    {0x01, 0x01, 0x01, 0x11, 0x0E},

    /* K */
    {0x11, 0x12, 0x1C, 0x12, 0x11},

    /* L */
    {0x10, 0x10, 0x10, 0x10, 0x1F},

    /* M */
    {0x11, 0x1B, 0x15, 0x11, 0x11},

    /* N */
    {0x11, 0x19, 0x15, 0x13, 0x11},

    /* O */
    {0x0E, 0x11, 0x11, 0x11, 0x0E},

    /* P */
    {0x1E, 0x11, 0x1E, 0x10, 0x10},

    /* Q */
    {0x0E, 0x11, 0x15, 0x12, 0x0D},

    /* R */
    {0x1E, 0x11, 0x1E, 0x12, 0x11},

    /* S */
    {0x0F, 0x10, 0x0E, 0x01, 0x1E},

    /* T */
    {0x1F, 0x04, 0x04, 0x04, 0x04},

    /* U */
    {0x11, 0x11, 0x11, 0x11, 0x0E},

    /* V */
    {0x11, 0x11, 0x11, 0x0A, 0x04},

    /* W */
    {0x11, 0x11, 0x15, 0x1B, 0x11},

    /* X */
    {0x11, 0x0A, 0x04, 0x0A, 0x11},

    /* Y */
    {0x11, 0x0A, 0x04, 0x04, 0x04},

    /* Z */
    {0x1F, 0x02, 0x04, 0x08, 0x1F},

    /* 0 */
    {0x0E, 0x11, 0x11, 0x11, 0x0E},

    /* 1 */
    {0x04, 0x0C, 0x04, 0x04, 0x0E},

    /* 2 */
    {0x0E, 0x01, 0x06, 0x08, 0x1F},

    /* 3 */
    {0x1E, 0x01, 0x06, 0x01, 0x1E},

    /* 4 */
    {0x12, 0x12, 0x1F, 0x02, 0x02},

    /* 5 */
    {0x1F, 0x10, 0x1E, 0x01, 0x1E},

    /* 6 */
    {0x0E, 0x10, 0x1E, 0x11, 0x0E},

    /* 7 */
    {0x1F, 0x01, 0x02, 0x04, 0x04},

    /* 8 */
    {0x0E, 0x11, 0x0E, 0x11, 0x0E},

    /* 9 */
    {0x0E, 0x11, 0x0F, 0x01, 0x0E}
};


/*
 * =============================================================================
 * Pixel Mapping
 * =============================================================================
 *
 * Physical matrix is wired in a serpentine pattern:
 *
 *   Row 0:  0  1  2  3  4  -->
 *   Row 1:  9  8  7  6  5  <--
 *   Row 2: 10 11 12 13 14  -->
 *   Row 3: 19 18 17 16 15  <--
 *   Row 4: 20 21 22 23 24  -->
 *
 * This allows us to work using normal X/Y coordinates.
 *
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5)
    {
        return -1;
    }

    if ((y % 2) == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


/*
 * =============================================================================
 * Set Pixel
 * =============================================================================
 */

static void set_pixel(
    int x,
    int y,
    uint8_t red,
    uint8_t green,
    uint8_t blue
)
{
    int index = pixel_xy(x, y);

    if (index < 0)
    {
        return;
    }

    ESP_ERROR_CHECK(
        led_strip_set_pixel(
            matrix,
            index,
            red,
            green,
            blue
        )
    );
}


/*
 * =============================================================================
 * Clear Matrix
 * =============================================================================
 */

static void clear_matrix(void)
{
    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );
}


/*
 * =============================================================================
 * Font Index
 * =============================================================================
 */

static int font_index(char c)
{
    c = (char)toupper((unsigned char)c);

    if (c >= 'A' && c <= 'Z')
    {
        return c - 'A';
    }

    if (c >= '0' && c <= '9')
    {
        return 26 + (c - '0');
    }

    return -1;
}


/*
 * =============================================================================
 * Draw Font Column
 * =============================================================================
 */

static void draw_column(
    uint8_t column,
    int x
)
{
    if (x < 0 || x >= 5)
    {
        return;
    }

    for (int y = 0; y < 5; y++)
    {
        /*
         * Bit 4 = top
         * Bit 0 = bottom
         */
        if (column & (1 << (4 - y)))
        {
            set_pixel(
                x,
                y,
                0,
                150,
                255
            );
        }
    }
}


/*
 * =============================================================================
 * Draw Text Frame
 * =============================================================================
 *
 * offset controls the horizontal position of the message.
 *
 * Each character occupies:
 *
 *   5 columns + 1 blank column
 *
 * =============================================================================
 */

static void draw_text_frame(
    const char *text,
    int offset
)
{
    clear_matrix();

    int length = strlen(text);

    for (int i = 0; i < length; i++)
    {
        int index = font_index(text[i]);

        /*
         * Spaces are intentionally blank.
         */
        if (index < 0)
        {
            continue;
        }

        /*
         * Each character is 6 pixels wide:
         *
         *  XXXXX.
         */
        int x_base = offset + (i * 6);

        for (int x = 0; x < 5; x++)
        {
            int display_x = x_base + x;

            if (display_x >= 0 &&
                display_x < 5)
            {
                draw_column(
                    font[index][x],
                    display_x
                );
            }
        }
    }

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Scroll Message
 * =============================================================================
 */

static void scroll_message(
    const char *text
)
{
    int length = strlen(text);

    /*
     * Total message width:
     *
     * 6 pixels per character
     */
    int width = length * 6;

    /*
     * Start completely outside the right side.
     *
     * offset = 5
     *
     * End completely outside the left side.
     *
     * offset = -width
     */
    for (
        int offset = 5;
        offset >= -width;
        offset--
    )
    {
        draw_text_frame(
            text,
            offset
        );

        vTaskDelay(
            pdMS_TO_TICKS(SCROLL_DELAY_MS)
        );
    }
}


/*
 * =============================================================================
 * Matrix Initialization
 * =============================================================================
 */

static void matrix_init(void)
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
        .resolution_hz = 10 * 1000 * 1000,
        .mem_block_symbols = 64,

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &matrix
        )
    );


    clear_matrix();

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Main
 * =============================================================================
 */

void app_main(void)
{
    /*
     * Initialize RGB matrix.
     */
    matrix_init();


    ESP_LOGI(
        TAG,
        "5x5 RGB Matrix Scrolling Text"
    );

    ESP_LOGI(
        TAG,
        "Matrix GPIO: %d",
        MATRIX_PIN
    );

    ESP_LOGI(
        TAG,
        "Message: %s",
        MESSAGE
    );


    /*
     * Main loop.
     */
    while (1)
    {
        scroll_message(MESSAGE);

        /*
         * Pause after the complete message
         * has disappeared.
         */
        vTaskDelay(
            pdMS_TO_TICKS(300)
        );
    }
}
