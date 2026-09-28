/*
 * =============================================================================
 * GREEN DIGITAL RAIN - FreeRTOS
 * =============================================================================
 *
 * Creates a green digital-rain effect on the 5x5 RGB LED matrix.
 *
 * The rain visually moves upward.
 *
 * Matrix:
 *   GPIO 7
 *
 * Each column has an independently moving rain drop.
 *
 * The animation runs inside a dedicated FreeRTOS task.
 * delay() has been replaced by vTaskDelay().
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define RAIN_UPDATE_MS 150

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Convert X/Y coordinates to LED index.
//=============================================================================

int pixelXY(int x, int y)
{
  // Flip the Y axis so the visual movement is upward.
  y = 4 - y;

  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//=============================================================================
// Rain Drop Positions
//=============================================================================

int drops[5];

//=============================================================================
// FreeRTOS Rain Animation Task
//=============================================================================

void rainTask(void *parameter)
{
  while (true)
  {
    matrix.clear();

    // Draw the moving rain drops.
    for (int x = 0; x < 5; x++)
    {
      int y = drops[x];

      // Bright head.
      if (y >= 0 && y < 5)
      {
        matrix.setPixelColor(
          pixelXY(x, y),
          matrix.Color(0, 255, 0)
        );
      }

      // Dim trail behind the head.
      if (y - 1 >= 0)
      {
        matrix.setPixelColor(
          pixelXY(x, y - 1),
          matrix.Color(0, 80, 0)
        );
      }

      // Move the drop upward.
      drops[x]++;

      // Restart the drop after it leaves the display.
      if (drops[x] > 6)
      {
        drops[x] =
          random(-4, 1);
      }
    }

    matrix.show();

    // Allow other FreeRTOS tasks to execute.
    vTaskDelay(
      pdMS_TO_TICKS(RAIN_UPDATE_MS)
    );
  }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  randomSeed(
    analogRead(1)
  );

  // Start each column at a random position.
  for (int x = 0; x < 5; x++)
  {
    drops[x] =
      random(0, 5);
  }

  xTaskCreate(
    rainTask,
    "RainTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // Animation is handled by RainTask.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
