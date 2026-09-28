/*
 * This program alternates two complementary colors across the
 * 5x5 RGB matrix in a checkerboard pattern, creating a strobe
 * effect.
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
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void drawCheckerboard(
  bool inverted
)
{
  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      bool firstColor =
        ((x + y) % 2 == 0);

      if (inverted)
        firstColor = !firstColor;

      uint32_t color;

      if (firstColor)
      {
        color =
          matrix.Color(
            255,
            0,
            100
          );
      }
      else
      {
        color =
          matrix.Color(
            0,
            255,
            255
          );
      }

      matrix.setPixelColor(
        pixelXY(x, y),
        color
      );
    }
  }

  matrix.show();
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(15);
}

void loop()
{
  // First checkerboard arrangement
  drawCheckerboard(false);

  delay(300);

  // Swap the colors
  drawCheckerboard(true);

  delay(300);
}
