/*
 * =============================================================================
 * RGB CHECKERBOARD STROBE
 * =============================================================================
 *
 * This program alternates two complementary colours across the
 * 5x5 RGB matrix in a checkerboard pattern, creating a strobe
 * effect.
 *
 * The animation runs inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *   NeoPixel matrix -> GPIO 7
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define MATRIX_PIN 7
#define NUM_LEDS 25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Matrix coordinate mapping
// =============================================================================

int pixelXY(
  int x,
  int y
)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

// =============================================================================
// Draw checkerboard
// =============================================================================

void drawCheckerboard(
  bool inverted
)
{
  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      bool firstColour =
        ((x + y) % 2 == 0);

      if (inverted)
        firstColour = !firstColour;

      uint32_t colour;

      if (firstColour)
      {
        colour =
          matrix.Color(
            255,
            0,
            100
          );
      }
      else
      {
        colour =
          matrix.Color(
            0,
            255,
            255
          );
      }

      matrix.setPixelColor(
        pixelXY(x, y),
        colour
      );
    }
  }

  matrix.show();
}

// =============================================================================
// Strobe FreeRTOS task
// =============================================================================

void checkerboardTask(void *parameter)
{
  while (true)
  {
    // First checkerboard arrangement.
    drawCheckerboard(false);

    vTaskDelay(
      pdMS_TO_TICKS(300)
    );

    // Swap the colours.
    drawCheckerboard(true);

    vTaskDelay(
      pdMS_TO_TICKS(300)
    );
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup()
{
  matrix.begin();

  matrix.setBrightness(10);

  // Create the checkerboard task.
  xTaskCreatePinnedToCore(
    checkerboardTask,
    "Checkerboard Task",
    2048,
    NULL,
    1,
    NULL,
    1
  );
}

// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // Animation is handled by checkerboardTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
