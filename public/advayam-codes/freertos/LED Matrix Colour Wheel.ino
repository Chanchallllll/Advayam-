/*
 * =============================================================================
 * 5x5 ROTATING RAINBOW COLOR WHEEL
 * =============================================================================
 *
 * Every pixel receives a different HSV hue.
 *
 * The hue pattern continuously rotates around the matrix.
 *
 * Hardware:
 *   Matrix DATA = GPIO 7
 * =============================================================================
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

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  matrix.begin();

  matrix.setBrightness(25);

  matrix.clear();

  matrix.show();
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  static uint16_t offset = 0;

  //------------------------------------------------------------------------
  // Give every pixel a different hue.
  //------------------------------------------------------------------------

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

  //------------------------------------------------------------------------
  // Rotate color pattern.
  //------------------------------------------------------------------------

  offset += 500;

  delay(50);
}
