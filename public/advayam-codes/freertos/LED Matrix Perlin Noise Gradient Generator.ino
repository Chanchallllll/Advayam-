/*
 * =============================================================================
 * PROCEDURAL COLOUR GRADIENT - FreeRTOS VERSION
 * =============================================================================
 *
 * Creates smoothly changing procedural colour gradients across the
 * 5x5 RGB matrix using interpolated mathematical noise.
 *
 * FreeRTOS:
 *   Animation task -> Generates and displays each gradient frame.
 *
 * Matrix = GPIO 7
 * =============================================================================
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
// Smooth Interpolation
//=============================================================================

float smoothStep(float t)
{
  return t * t * (3.0 - 2.0 * t);
}


//=============================================================================
// Procedural Noise
//=============================================================================

float noise2D(
  float x,
  float y
)
{
  float value =
    sin(x * 1.7 + sin(y * 1.3)) *
    cos(y * 1.9 + sin(x * 0.8));

  return (value + 1.0) / 2.0;
}


//=============================================================================
// Gradient Animation Task
//=============================================================================

void gradientTask(void *parameter)
{
  float time = 0;

  while (true)
  {
    if (xSemaphoreTake(
          matrixMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
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

      xSemaphoreGive(matrixMutex);
    }

    time += 0.05;

    // Approximately 20 FPS.
    vTaskDelay(
      pdMS_TO_TICKS(50)
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

  matrixMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    gradientTask,
    "GradientAnimation",
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
