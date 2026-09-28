/*
 * =============================================================================
 * 5x5 TETROMINO ANIMATION - ESP-IDF v5+
 * =============================================================================
 *
 * Simple falling/side-moving tetromino blocks on a 5x5 RGB matrix.
 *
 * Matrix:
 *   GPIO 7
 *   25 WS2812 LEDs
 *
 * Shapes:
 *   I
 *   T
 *   L
 *   O
 *
 * Behaviour:
 *   - Random tetromino is selected.
 *   - Block enters from the left.
 *   - Block moves toward the right.
 *   - Block stops when it reaches the edge or a settled block.
 *   - Settled blocks are dim gray.
 *   - Moving block is colored.
 *   - When the top row becomes full, the board clears.
 *
 * Matrix wiring:
 *
 *   Row 0:  0 -> 1 -> 2 -> 3 -> 4
 *   Row 1:  9 <- 8 <- 7 <- 6 <- 5
 *   Row 2: 10 -> 11 -> 12 -> 13 -> 14
 *   Row 3: 19 <- 18 <- 17 <- 16 <- 15
 *   Row 4: 20 -> 21 -> 22 -> 23 -> 24
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"

#include "led_strip.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

#define MATRIX_PIN          7
#define NUM_LEDS            25

#define MATRIX_BRIGHTNESS   25

#define MOVE_DELAY_MS       250
#define LOCK_DELAY_MS       300
#define CLEAR_DELAY_MS      500


static const char *TAG = "TETROMINO";


/*
 * =============================================================================
 * LED Matrix
 * =============================================================================
 */

static led_strip_handle_t matrix = NULL;


/*
 * =============================================================================
 * Board
 * =============================================================================
 *
 * board[y][x] == true means a block has settled at that position.
 * =============================================================================
 */

static bool board[5][5];


/*
 * =============================================================================
 * Tetromino Shapes
 * =============================================================================
 */

static const int I_SHAPE[4][2] =
{
    {0, 0},
    {1, 0},
    {2, 0},
    {3, 0}
};


static const int T_SHAPE[4][2] =
{
    {1, 0},
    {0, 1},
    {1, 1},
    {2, 1}
};


static const int L_SHAPE[4][2] =
{
    {0, 0},
    {0, 1},
    {0, 2},
    {1, 2}
};


static const int O_SHAPE[4][2] =
{
    {0, 0},
    {1, 0},
    {0, 1},
    {1, 1}
};


/*
 * =============================================================================
 * Pixel Mapping
 * =============================================================================
 */

static int pixel_xy(int x, int y)
{
    if (x < 0 || x > 4 ||
        y < 0 || y > 4) {
        return -1;
    }


    /*
     * Even rows run left -> right.
     */

    if ((y % 2) == 0) {
        return y * 5 + x;
    }


    /*
     * Odd rows run right -> left.
     */

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
    uint8_t r,
    uint8_t g,
    uint8_t b
)
{
    int index = pixel_xy(x, y);

    if (index < 0 || index >= NUM_LEDS) {
        return;
    }


    /*
     * Apply global brightness.
     *
     * Equivalent to:
     *
     * matrix.setBrightness(25);
     */

    r = (r * MATRIX_BRIGHTNESS) / 255;
    g = (g * MATRIX_BRIGHTNESS) / 255;
    b = (b * MATRIX_BRIGHTNESS) / 255;


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


    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Draw Board
 * =============================================================================
 */

static void draw_board(
    int block_x,
    int block_y,
    const int shape[][2],
    int size,
    uint8_t block_r,
    uint8_t block_g,
    uint8_t block_b
)
{
    /*
     * Clear matrix.
     */

    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    /*
     * ---------------------------------------------------------
     * Draw settled blocks.
     * ---------------------------------------------------------
     *
     * Settled blocks are dim gray.
     */

    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            if (board[y][x]) {

                set_pixel(
                    x,
                    y,
                    80,
                    80,
                    80
                );
            }
        }
    }


    /*
     * ---------------------------------------------------------
     * Draw moving block.
     * ---------------------------------------------------------
     */

    for (int i = 0; i < size; i++) {

        int x =
            block_x + shape[i][0];

        int y =
            block_y + shape[i][1];


        if (x >= 0 && x < 5 &&
            y >= 0 && y < 5) {

            set_pixel(
                x,
                y,
                block_r,
                block_g,
                block_b
            );
        }
    }


    /*
     * Send data to LEDs.
     */

    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Check Horizontal Movement
 * =============================================================================
 */

static bool can_move_right(
    int block_x,
    int block_y,
    const int shape[][2],
    int size
)
{
    for (int i = 0; i < size; i++) {

        int x =
            block_x + shape[i][0];

        int y =
            block_y + shape[i][1];


        /*
         * Right edge.
         */

        if (x >= 5) {
            return false;
        }


        /*
         * Check collision with settled blocks.
         */

        if (x >= 0 &&
            y >= 0 &&
            y < 5 &&
            board[y][x]) {

            return false;
        }
    }


    return true;
}


/*
 * =============================================================================
 * Place Block
 * =============================================================================
 */

static void place_block(
    int block_x,
    int block_y,
    const int shape[][2],
    int size
)
{
    for (int i = 0; i < size; i++) {

        int x =
            block_x + shape[i][0];

        int y =
            block_y + shape[i][1];


        /*
         * Only store pixels that are actually
         * inside the matrix.
         */

        if (x >= 0 && x < 5 &&
            y >= 0 && y < 5) {

            board[y][x] = true;
        }
    }
}


/*
 * =============================================================================
 * Check Crowded Board
 * =============================================================================
 *
 * The original Arduino program clears the board when the top row
 * is completely occupied.
 * =============================================================================
 */

static bool board_is_crowded(void)
{
    for (int x = 0; x < 5; x++) {

        if (!board[0][x]) {
            return false;
        }
    }


    return true;
}


/*
 * =============================================================================
 * Clear Board
 * =============================================================================
 */

static void clear_board(void)
{
    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            board[y][x] = false;
        }
    }


    ESP_ERROR_CHECK(
        led_strip_clear(matrix)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(matrix)
    );
}


/*
 * =============================================================================
 * Select Random Shape
 * =============================================================================
 */

static const int (*select_shape(
    int choice
))[2]
{
    switch (choice) {

        case 0:
            return I_SHAPE;

        case 1:
            return T_SHAPE;

        case 2:
            return L_SHAPE;

        default:
            return O_SHAPE;
    }
}


/*
 * =============================================================================
 * Get Block Color
 * =============================================================================
 */

static void get_block_color(
    int choice,
    uint8_t *r,
    uint8_t *g,
    uint8_t *b
)
{
    switch (choice) {

        /*
         * I = cyan
         */

        case 0:

            *r = 0;
            *g = 200;
            *b = 255;

            break;


        /*
         * T = purple
         */

        case 1:

            *r = 180;
            *g = 0;
            *b = 255;

            break;


        /*
         * L = orange
         */

        case 2:

            *r = 255;
            *g = 100;
            *b = 0;

            break;


        /*
         * O = yellow
         */

        default:

            *r = 255;
            *g = 255;
            *b = 0;

            break;
    }
}


/*
 * =============================================================================
 * Initialize Board
 * =============================================================================
 */

static void initialize_board(void)
{
    for (int y = 0; y < 5; y++) {

        for (int x = 0; x < 5; x++) {

            board[y][x] = false;
        }
    }
}


/*
 * =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    /*
     * Initialize LED matrix.
     */

    matrix_init();


    /*
     * Initialize board.
     */

    initialize_board();


    /*
     * Seed random number generator.
     *
     * We don't need an ADC here, so use a combination of
     * system timer and address entropy.
     */

    srand(
        (unsigned int)(
            (uintptr_t)&matrix
        )
    );


    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "       5x5 TETROMINO ANIMATION"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );


    /*
     * =========================================================
     * Main game loop
     * =========================================================
     */

    while (1) {

        /*
         * -----------------------------------------------------
         * Select random shape.
         * -----------------------------------------------------
         */

        int choice =
            rand() % 4;


        const int (*shape)[2] =
            select_shape(choice);


        /*
         * -----------------------------------------------------
         * Get shape colour.
         * -----------------------------------------------------
         */

        uint8_t r;
        uint8_t g;
        uint8_t b;


        get_block_color(
            choice,
            &r,
            &g,
            &b
        );


        /*
         * -----------------------------------------------------
         * Start block from the left.
         * -----------------------------------------------------
         *
         * -3 allows the larger shapes to enter from outside
         * the left side of the matrix.
         */

        int block_x = -3;


        /*
         * Random vertical position.
         *
         * Same behavior as the original Arduino program.
         */

        int block_y =
            rand() % 4;


        ESP_LOGI(
            TAG,
            "New block: %d | Start Y: %d",
            choice,
            block_y
        );


        /*
         * -----------------------------------------------------
         * Move block from left -> right.
         * -----------------------------------------------------
         */

        while (
            can_move_right(
                block_x + 1,
                block_y,
                shape,
                4
            )
        )
        {
            block_x++;


            draw_board(
                block_x,
                block_y,
                shape,
                4,
                r,
                g,
                b
            );


            vTaskDelay(
                pdMS_TO_TICKS(MOVE_DELAY_MS)
            );
        }


        /*
         * -----------------------------------------------------
         * Lock block.
         * -----------------------------------------------------
         */

        place_block(
            block_x,
            block_y,
            shape,
            4
        );


        /*
         * Draw final position.
         */

        draw_board(
            block_x,
            block_y,
            shape,
            4,
            r,
            g,
            b
        );


        vTaskDelay(
            pdMS_TO_TICKS(LOCK_DELAY_MS)
        );


        /*
         * -----------------------------------------------------
         * Check whether board is crowded.
         * -----------------------------------------------------
         */

        if (board_is_crowded()) {

            ESP_LOGI(
                TAG,
                "Board full - clearing!"
            );


            clear_board();


            vTaskDelay(
                pdMS_TO_TICKS(CLEAR_DELAY_MS)
            );
        }
    }
}
