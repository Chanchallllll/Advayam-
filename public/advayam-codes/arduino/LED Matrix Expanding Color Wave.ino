/*
 * This program animates expanding square frames on the 5x5
 * RGB LED matrix, starting from the center and expanding
 * toward the outer boundary.
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

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

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

// Draw one square frame
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

    // Top and bottom edges
    for (int x = minPos; x <= maxPos; x++)
    {
        setPixelXY(x, minPos, color);
        setPixelXY(x, maxPos, color);
    }

    // Left and right edges
    for (int y = minPos; y <= maxPos; y++)
    {
        setPixelXY(minPos, y, color);
        setPixelXY(maxPos, y, color);
    }

    matrix.show();
}

void setup()
{
    matrix.begin();
    matrix.setBrightness(50);

    matrix.clear();
    matrix.show();
}

void loop()
{
    // Center pixel
    drawSquare(
        0,
        matrix.Color(255, 0, 0)
    );

    delay(300);

    // 3x3 square
    drawSquare(
        1,
        matrix.Color(0, 255, 0)
    );

    delay(300);

    // 5x5 square
    drawSquare(
        2,
        matrix.Color(0, 0, 255)
    );

    delay(500);
}
