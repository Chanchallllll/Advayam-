/*
 * =============================================================================
 * TOUCHLESS TRASHCAN LID MIMIC - FreeRTOS
 * =============================================================================
 *
 * The RPR-0521RS proximity sensor detects a hand hovering above the sensor.
 *
 * A continuous hover of approximately one second triggers the lid animation.
 *
 * Matrix:
 *   GPIO 7
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * FreeRTOS structure:
 *
 *   SensorTask
 *       |
 *       | 1-second hover detected
 *       v
 *   AnimationTask
 *
 * A FreeRTOS task notification is used to signal the animation task.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

#define PROXIMITY_THRESHOLD 500

#define SENSOR_INTERVAL_MS 50
#define HOVER_TIME_MS      1000

RPR0521RS sensor;

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

TaskHandle_t animationTaskHandle = NULL;

//------------------------------------------------------------------------------
// Convert matrix row/column coordinates to the NeoPixel index.
//------------------------------------------------------------------------------
int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//------------------------------------------------------------------------------
// Lid opening/closing animation.
//------------------------------------------------------------------------------
void animateLid()
{
  // Open the lid by lighting rows from bottom to top.
  for (int row = 0; row < 5; row++)
  {
    for (int col = 0; col < 5; col++)
    {
      matrix.setPixelColor(
        pixelXY(col, row),
        matrix.Color(255, 120, 20)
      );
    }

    matrix.show();

    vTaskDelay(
      pdMS_TO_TICKS(120)
    );
  }

  // Keep the lid open for a short time.
  vTaskDelay(
    pdMS_TO_TICKS(500)
  );

  // Close the lid by clearing rows from top to bottom.
  for (int row = 4; row >= 0; row--)
  {
    for (int col = 0; col < 5; col++)
    {
      matrix.setPixelColor(
        pixelXY(col, row),
        0
      );
    }

    matrix.show();

    vTaskDelay(
      pdMS_TO_TICKS(120)
    );
  }
}

//------------------------------------------------------------------------------
// Monitors the proximity sensor.
//
// The task detects a continuous one-second hover and notifies the animation
// task. The animation itself does not execute inside this task.
//------------------------------------------------------------------------------
void sensorTask(void *parameter)
{
  unsigned long hoverStart = 0;
  bool animationPlayed = false;

  while (true)
  {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(&proximity, &lux) == 0)
    {
      if (proximity > PROXIMITY_THRESHOLD)
      {
        if (hoverStart == 0)
        {
          hoverStart = millis();
        }

        if (!animationPlayed &&
            millis() - hoverStart >= HOVER_TIME_MS)
        {
          animationPlayed = true;

          // Notify the animation task that a complete hover occurred.
          xTaskNotifyGive(animationTaskHandle);
        }
      }
      else
      {
        // Hand has left the sensor.
        hoverStart = 0;
        animationPlayed = false;
      }
    }

    vTaskDelay(
      pdMS_TO_TICKS(SENSOR_INTERVAL_MS)
    );
  }
}

//------------------------------------------------------------------------------
// Animation task.
//
// It waits for a notification from SensorTask before playing the animation.
//------------------------------------------------------------------------------
void animationTask(void *parameter)
{
  while (true)
  {
    // Sleep until the sensor task reports a valid hover.
    ulTaskNotifyTake(
      pdTRUE,
      portMAX_DELAY
    );

    Serial.println("Opening lid...");

    animateLid();

    Serial.println("Lid closed.");
  }
}

void setup()
{
  Serial.begin(115200);

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  if (sensor.init() != 0)
  {
    Serial.println("Sensor initialization failed!");

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  matrix.begin();
  matrix.setBrightness(20);
  matrix.clear();
  matrix.show();

  // Create the animation task first so its handle is available.
  xTaskCreate(
    animationTask,
    "AnimationTask",
    4096,
    NULL,
    1,
    &animationTaskHandle
  );

  xTaskCreate(
    sensorTask,
    "SensorTask",
    4096,
    NULL,
    2,
    NULL
  );
}

void loop()
{
  // Sensor and animation tasks perform the application work.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
