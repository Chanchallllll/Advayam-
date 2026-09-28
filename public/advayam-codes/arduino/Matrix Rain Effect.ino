/*
 * Green digital rain effect on a 5x5 RGB LED matrix.
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

//====================================================
// Convert X/Y coordinates to LED index
//====================================================
int pixelXY(int x, int y)
{
  // Flip Y axis so the rain moves upward
  y = 4 - y;

  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//====================================================
// Rain drop position for each column
//====================================================
int drops[5];

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  randomSeed(analogRead(1));

  // Start each column at a random height
  for (int x = 0; x < 5; x++)
    drops[x] = random(0, 5);
}

//====================================================
// Main Loop
//====================================================
void loop()
{
  matrix.clear();

  // Draw falling/upward-moving drops
  for (int x = 0; x < 5; x++)
  {
    int y = drops[x];

    // Bright head
    if (y >= 0 && y < 5)
    {
      matrix.setPixelColor(
        pixelXY(x, y),
        matrix.Color(0, 255, 0)
      );
    }

    // Dim trail behind the head
    if (y - 1 >= 0)
    {
      matrix.setPixelColor(
        pixelXY(x, y - 1),
        matrix.Color(0, 80, 0)
      );
    }

    // Move upward
    drops[x]++;

    // Restart from the bottom
    if (drops[x] > 6)
      drops[x] = random(-4, 1);
  }

  matrix.show();

  delay(150);
}
