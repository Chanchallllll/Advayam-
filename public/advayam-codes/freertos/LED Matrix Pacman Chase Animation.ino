/*
 * =============================================================================
 * PACMAN-STYLE CHASE - FreeRTOS VERSION
 * =============================================================================
 *
 * A yellow Pacman pixel is chased by a red ghost pixel across the
 * 5x5 RGB matrix.
 *
 * FreeRTOS:
 *   Animation task -> Controls the Pacman/ghost animation.
 *
 * Matrix = GPIO 7
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

SemaphoreHandle_t matrixMutex;


//=============================================================================
// Matrix Mapping
//=============================================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


//=============================================================================
// Draw Scene
//=============================================================================

void drawScene(
  int pacman,
  int ghost
)
{
  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) != pdTRUE)
  {
    return;
  }

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

  xSemaphoreGive(matrixMutex);
}


//=============================================================================
// Animation Task
//=============================================================================

void animationTask(void *parameter)
{
  while (true)
  {
    // Move Pacman from left to right.
    for (int x = 0; x < 5; x++)
    {
      int ghost =
        max(0, x - 2);

      drawScene(
        x,
        ghost
      );

      vTaskDelay(
        pdMS_TO_TICKS(250)
      );
    }

    // Return in the opposite direction.
    for (int x = 4; x >= 0; x--)
    {
      int ghost =
        min(4, x + 2);

      drawScene(
        x,
        ghost
      );

      vTaskDelay(
        pdMS_TO_TICKS(250)
      );
    }
  }
}


//=============================================================================
// Setup
//=============================================================================

void setup()
{
  matrix.begin();

  matrix.setBrightness(25);

  matrixMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    animationTask,
    "PacmanAnimation",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}


//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
