/*
 * This program illuminates every pixel of the 5x5 RGB matrix
 * one-by-one in a clockwise spiral from the outside toward
 * the center using a FreeRTOS task.
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

// FreeRTOS Task for handling the matrix spiral animation
void matrixSpiralTask(void *pvParameters)
{
  while (true)
  {
    matrix.clear();

    int left = 0;
    int right = 4;
    int top = 0;
    int bottom = 4;

    // Walk inward in clockwise order
    while (left <= right && top <= bottom)
    {
      for (int x = left; x <= right; x++)
      {
        matrix.setPixelColor(
          pixelXY(x, top),
          matrix.Color(0, 150, 255)
        );
        matrix.show();
        vTaskDelay(pdMS_TO_TICKS(100));
      }

      top++;

      for (int y = top; y <= bottom; y++)
      {
        matrix.setPixelColor(
          pixelXY(right, y),
          matrix.Color(0, 150, 255)
        );
        matrix.show();
        vTaskDelay(pdMS_TO_TICKS(100));
      }

      right--;

      if (top <= bottom)
      {
        for (int x = right; x >= left; x--)
        {
          matrix.setPixelColor(
            pixelXY(x, bottom),
            matrix.Color(0, 150, 255)
          );
          matrix.show();
          vTaskDelay(pdMS_TO_TICKS(100));
        }

        bottom--;
      }

      if (left <= right)
      {
        for (int y = bottom; y >= top; y--)
        {
          matrix.setPixelColor(
            pixelXY(left, y),
            matrix.Color(0, 150, 255)
          );
          matrix.show();
          vTaskDelay(pdMS_TO_TICKS(100));
        }

        left++;
      }
    }

    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  // Create the FreeRTOS task
  xTaskCreate(
    matrixSpiralTask,
    "SpiralTask",
    2048,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // FreeRTOS handles execution via tasks; leave loop empty or delete task
  vTaskDelete(NULL);
}