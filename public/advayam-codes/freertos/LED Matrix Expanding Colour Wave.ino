/*
 * Expanding Square Frames - FreeRTOS Version
 *
 * This program animates expanding square frames on the 5x5
 * RGB LED matrix, starting from the center and expanding
 * toward the outer boundary.
 *
 * FreeRTOS task:
 *   SquareAnimationTask - Controls the complete animation.
 *
 * Delays are implemented using FreeRTOS vTaskDelay().
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);


// ============================================================================
// Convert X/Y coordinates to the serpentine LED index
// ============================================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


// ============================================================================
// Set a pixel using X/Y coordinates
// ============================================================================

void setPixelXY(
    int x,
    int y,
    uint32_t color
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5)
    {
        return;
    }

    matrix.setPixelColor(
        pixelXY(x, y),
        color
    );
}


// ============================================================================
// Draw one square frame
// ============================================================================

void drawSquare(
    int size,
    uint32_t color
)
{
    matrix.clear();

    int minPos =
        2 - size;

    int maxPos =
        2 + size;

    // Top and bottom edges.
    for (int x = minPos; x <= maxPos; x++)
    {
        setPixelXY(
            x,
            minPos,
            color
        );

        setPixelXY(
            x,
            maxPos,
            color
        );
    }

    // Left and right edges.
    for (int y = minPos; y <= maxPos; y++)
    {
        setPixelXY(
            minPos,
            y,
            color
        );

        setPixelXY(
            maxPos,
            y,
            color
        );
    }

    matrix.show();
}


// ============================================================================
// FreeRTOS animation task
// ============================================================================

void SquareAnimationTask(void *parameter)
{
    while (true)
    {
        // ---------------------------------------------------------------
        // Center pixel
        // ---------------------------------------------------------------

        drawSquare(
            0,
            matrix.Color(255, 0, 0)
        );

        vTaskDelay(
            pdMS_TO_TICKS(300)
        );


        // ---------------------------------------------------------------
        // 3x3 square
        // ---------------------------------------------------------------

        drawSquare(
            1,
            matrix.Color(0, 255, 0)
        );

        vTaskDelay(
            pdMS_TO_TICKS(300)
        );


        // ---------------------------------------------------------------
        // 5x5 square
        // ---------------------------------------------------------------

        drawSquare(
            2,
            matrix.Color(0, 0, 255)
        );

        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}


// ============================================================================
// Setup
// ============================================================================

void setup()
{
    Serial.begin(115200);

    matrix.begin();

    matrix.setBrightness(50);

    matrix.clear();
    matrix.show();

    // Create the animation task.
    xTaskCreatePinnedToCore(
        SquareAnimationTask,
        "SquareAnimationTask",
        2048,
        NULL,
        1,
        NULL,
        1
    );
}


// ============================================================================
// Arduino loop
// ============================================================================

void loop()
{
    // Animation is controlled entirely by FreeRTOS.
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
