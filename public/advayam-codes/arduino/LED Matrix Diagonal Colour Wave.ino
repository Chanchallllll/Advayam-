/*
 * This program creates a moving diagonal rainbow wave across
 * the 5x5 RGB LED matrix connected to GPIO 7.
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

// Convert coordinates to the serpentine wiring
int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

// Set a pixel using coordinates
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

void setup()
{
    matrix.begin();
    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();
}

void loop()
{
    static uint8_t offset = 0;

    // Create diagonal hue movement
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

    delay(50);
}
