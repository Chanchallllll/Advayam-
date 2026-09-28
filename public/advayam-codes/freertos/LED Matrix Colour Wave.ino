/*
 * ============================================================================
 * MOVING DIAGONAL RAINBOW WAVE - FREERTOS VERSION
 * ============================================================================
 *
 * Creates a moving diagonal rainbow wave across the 5x5 RGB LED matrix.
 *
 * The animation runs inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *   Matrix data -> GPIO 7
 * ============================================================================
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

//=============================================================================
// Convert coordinates to the serpentine wiring.
//=============================================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

//=============================================================================
// Set a pixel using x/y coordinates.
//=============================================================================

void setPixelXY(
    int x,
    int y,
    uint32_t colour
)
{
    if (x < 0 || x >= 5 ||
        y < 0 || y >= 5)
    {
        return;
    }

    matrix.setPixelColor(
        pixelXY(x, y),
        colour
    );
}

//=============================================================================
// Rainbow animation FreeRTOS task.
//=============================================================================

void rainbowTask(void *parameter)
{
    uint8_t offset = 0;

    while (true)
    {
        // Create the diagonal rainbow pattern.
        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                uint8_t hue =
                    offset +
                    ((x + y) * 25);

                setPixelXY(
                    x,
                    y,
                    matrix.ColorHSV(
                        (uint16_t)hue * 256
                    )
                );
            }
        }

        matrix.show();

        offset += 4;

        // Give the CPU to other FreeRTOS tasks.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//=============================================================================
// Setup.
//=============================================================================

void setup()
{
    matrix.begin();
    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();

    // Create the rainbow animation task.
    xTaskCreate(
        rainbowTask,
        "Rainbow Task",
        4096,
        NULL,
        1,
        NULL
    );
}

//=============================================================================
// Arduino loop.
//
// The application runs in the FreeRTOS task above.
//=============================================================================

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
