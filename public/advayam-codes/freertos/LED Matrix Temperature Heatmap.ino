/*
 * LMT87 Temperature Heatmap - FreeRTOS Version
 *
 * This program displays the LMT87 temperature as a vertical
 * heatmap on the 5x5 RGB matrix.
 *
 * Lower temperatures produce blue colors while higher
 * temperatures progress through green and yellow to red.
 *
 * FreeRTOS tasks:
 *   1. TemperatureTask - Reads the LMT87 sensor.
 *   2. DisplayTask     - Updates the LED matrix.
 *
 * A mutex protects the shared temperature/heat value.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LMT87.h>

#define MATRIX_PIN 7
#define TEMP_PIN   1
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

LMT87 lmt87 = LMT87();

int ADCres = 10;
double voltageRef = 3.3;


// Shared heat level.
int currentHeat = 0;


// Mutex protecting currentHeat.
SemaphoreHandle_t temperatureMutex;


// ============================================================================
// Convert X/Y coordinates to the serpentine LED index
// ============================================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


// ============================================================================
// Temperature sensor task
// ============================================================================

void TemperatureTask(void *parameter)
{
  while (true)
  {
    double temperatureC =
      -lmt87.getCDegree(
        TEMP_PIN,
        ADCres,
        voltageRef,
        100
      );

    Serial.print("Temperature: ");
    Serial.print(
      temperatureC,
      1
    );
    Serial.println(" C");


    // Convert temperature to heat level.
    int heat =
      constrain(
        (int)((temperatureC - 15.0) * 10),
        0,
        255
      );


    // Update the shared heat value.
    if (xSemaphoreTake(
          temperatureMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      currentHeat = heat;

      xSemaphoreGive(
        temperatureMutex
      );
    }


    // Temperature does not need to be sampled
    // continuously at a very high rate.
    vTaskDelay(
      pdMS_TO_TICKS(500)
    );
  }
}


// ============================================================================
// Matrix display task
// ============================================================================

void DisplayTask(void *parameter)
{
  while (true)
  {
    int heat;

    // Get the latest heat value.
    if (xSemaphoreTake(
          temperatureMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      heat = currentHeat;

      xSemaphoreGive(
        temperatureMutex
      );
    }


    matrix.clear();


    for (int y = 0; y < 5; y++)
    {
      // Bottom is hottest.
      int rowHeat =
        heat - y * 35;

      rowHeat =
        constrain(
          rowHeat,
          0,
          255
        );


      uint32_t color;


      // ---------------------------------------------------------------
      // Blue -> cyan
      // ---------------------------------------------------------------

      if (rowHeat < 85)
      {
        color = matrix.Color(
          0,
          rowHeat * 3,
          255
        );
      }


      // ---------------------------------------------------------------
      // Cyan -> yellow
      // ---------------------------------------------------------------

      else if (rowHeat < 170)
      {
        color = matrix.Color(
          (rowHeat - 85) * 3,
          255,
          255 - (rowHeat - 85) * 3
        );
      }


      // ---------------------------------------------------------------
      // Yellow -> red
      // ---------------------------------------------------------------

      else
      {
        color = matrix.Color(
          255,
          255 - (rowHeat - 170) * 3,
          0
        );
      }


      for (int x = 0; x < 5; x++)
      {
        matrix.setPixelColor(
          pixelXY(x, y),
          color
        );
      }
    }


    matrix.show();


    // Refresh the display every 50 ms.
    vTaskDelay(
      pdMS_TO_TICKS(50)
    );
  }
}


// ============================================================================
// Setup
// ============================================================================

void setup()
{
  Serial.begin(115200);

  matrix.begin();

  matrix.setBrightness(15);

  matrix.clear();
  matrix.show();


  // Create mutex.
  temperatureMutex =
    xSemaphoreCreateMutex();

  if (temperatureMutex == NULL)
  {
    Serial.println(
      "ERROR: Failed to create temperature mutex."
    );

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }


  // Create temperature task.
  xTaskCreatePinnedToCore(
    TemperatureTask,
    "TemperatureTask",
    4096,
    NULL,
    1,
    NULL,
    0
  );


  // Create matrix display task.
  xTaskCreatePinnedToCore(
    DisplayTask,
    "DisplayTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}


// ============================================================================
// Arduino loop
// ============================================================================

void loop()
{
  // Application execution is handled by FreeRTOS.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
