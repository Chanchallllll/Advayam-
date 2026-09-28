/*
 * This program simulates a five-bar audio VU meter on the 5x5
 * RGB matrix.
 *
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

int barss[5];

//====================================================
// Convert X/Y coordinates to LED index
//====================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//====================================================
// Draw Upside-Down VU Meter
//====================================================

void drawMeter()
{
  matrix.clear();

  for (int x = 0; x < 5; x++)
  {
    for (int level = 0; level < barss[x]; level++)
    {
      // Grow from TOP to BOTTOM
      int y = level;

      uint32_t color;

      if (level < 3)
        color = matrix.Color(0, 255, 0);
      else
        color = matrix.Color(255, 80, 0);

      matrix.setPixelColor(
        pixelXY(x, y),
        color
      );
    }
  }

  matrix.show();
}

//====================================================
// Setup
//====================================================

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);

  randomSeed(analogRead(1));

  for (int i = 0; i < 5; i++)
    barss[i] = 0;
}

//====================================================
// Main Loop
//====================================================

void loop()
{
  for (int i = 0; i < 5; i++)
  {
    // Random peaks
    if (random(100) < 40)
      barss[i] = random(1, 6);

    // Decay
    else if (barss[i] > 0)
      barss[i]--;
  }

  drawMeter();

  delay(120);
}
