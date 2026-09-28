/*
 * This program creates a simple Pacman-style chase animation.
 * A yellow Pacman pixel is chased by a red ghost pixel across
 * the 5x5 RGB matrix.
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

void drawScene(int pacman, int ghost)
{
  matrix.clear();

  matrix.setPixelColor(
    pixelXY(pacman, 2),
    matrix.Color(255, 200, 0)
  );

  matrix.setPixelColor(
    pixelXY(ghost, 2),
    matrix.Color(255, 0, 0)
  );

  matrix.show();
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
}

void loop()
{
  // Move across the row
  for (int x = 0; x < 5; x++)
  {
    int ghost =
      max(0, x - 2);

    drawScene(
      x,
      ghost
    );

    delay(250);
  }

  // Return in the opposite direction
  for (int x = 4; x >= 0; x--)
  {
    int ghost =
      min(4, x + 2);

    drawScene(
      x,
      ghost
    );

    delay(250);
  }
}
