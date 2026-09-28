/*
 * This program simulates a radar sweep rotating around the
 * center pixel of the 5x5 matrix. Green trails fade behind
 * the rotating radar line.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

uint8_t brightness[5][5];

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
  static float angle = 0;

  // Fade previous trail
  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      brightness[y][x] =
        brightness[y][x] > 25
        ? brightness[y][x] - 25
        : 0;
    }
  }

  // Calculate radar endpoint
  int x =
    2 + round(cos(angle) * 2);

  int y =
    2 + round(sin(angle) * 2);

  // Light pixels from center to endpoint
  for (int step = 0; step <= 10; step++)
  {
    int px =
      2 + round(
        (x - 2) * step / 10.0
      );

    int py =
      2 + round(
        (y - 2) * step / 10.0
      );

    if (px >= 0 && px < 5 &&
        py >= 0 && py < 5)
    {
      brightness[py][px] = 255;
    }
  }

  matrix.clear();

  for (int yy = 0; yy < 5; yy++)
  {
    for (int xx = 0; xx < 5; xx++)
    {
      matrix.setPixelColor(
        pixelXY(xx, yy),
        matrix.Color(
          0,
          brightness[yy][xx],
          0
        )
      );
    }
  }

  matrix.show();

  angle += 0.2;

  delay(60);
}
