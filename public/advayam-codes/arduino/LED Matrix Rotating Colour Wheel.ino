/*
 * This program generates a rotating rainbow color wheel across
 * all 25 pixels of the RGB matrix using HSV color space.
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

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();
}

void loop()
{
  static uint16_t offset = 0;

  // Give every pixel a different hue
  for (int i = 0; i < NUM_LEDS; i++)
  {
    uint16_t hue =
      (offset + i * 1024) % 65536;

    matrix.setPixelColor(
      i,
      matrix.ColorHSV(
        hue,
        255,
        255
      )
    );
  }

  matrix.show();

  // Rotate the color pattern
  offset += 500;

  delay(50);
}
