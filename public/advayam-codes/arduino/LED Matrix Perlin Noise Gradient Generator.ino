/*
 * This program generates smoothly changing procedural color
 * gradients across the 5x5 RGB matrix using interpolated noise.
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

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

// Smooth interpolation
float smoothStep(float t)
{
  return t * t * (3.0 - 2.0 * t);
}

// Simple procedural noise
float noise2D(float x, float y)
{
  float value =
    sin(x * 1.7 + sin(y * 1.3)) *
    cos(y * 1.9 + sin(x * 0.8));

  return (value + 1.0) / 2.0;
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
}

void loop()
{
  static float time = 0;

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      float n =
        noise2D(
          x * 0.6 + time,
          y * 0.6 + time * 0.7
        );

      uint16_t hue =
        n * 65535;

      matrix.setPixelColor(
        pixelXY(x, y),
        matrix.ColorHSV(
          hue,
          255,
          255
        )
      );
    }
  }

  matrix.show();

  time += 0.05;

  delay(50);
}
