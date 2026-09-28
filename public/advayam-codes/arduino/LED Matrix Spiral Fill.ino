/*
 * This program illuminates every pixel of the 5x5 RGB matrix
 * one-by-one in a clockwise spiral from the outside toward
 * the center.
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

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();
}

void loop()
{
  matrix.clear();

  int left = 0;
  int right = 4;
  int top = 0;
  int bottom = 4;

  // Walk inward in clockwise order
  while (left <= right && top <= bottom)
  {
    for (int x = left; x <= right; x++)
    {
      matrix.setPixelColor(
        pixelXY(x, top),
        matrix.Color(0, 150, 255)
      );
      matrix.show();
      delay(100);
    }

    top++;

    for (int y = top; y <= bottom; y++)
    {
      matrix.setPixelColor(
        pixelXY(right, y),
        matrix.Color(0, 150, 255)
      );
      matrix.show();
      delay(100);
    }

    right--;

    if (top <= bottom)
    {
      for (int x = right; x >= left; x--)
      {
        matrix.setPixelColor(
          pixelXY(x, bottom),
          matrix.Color(0, 150, 255)
        );
        matrix.show();
        delay(100);
      }

      bottom--;
    }

    if (left <= right)
    {
      for (int y = bottom; y >= top; y--)
      {
        matrix.setPixelColor(
          pixelXY(left, y),
          matrix.Color(0, 150, 255)
        );
        matrix.show();
        delay(100);
      }

      left++;
    }
  }

  delay(500);
}
