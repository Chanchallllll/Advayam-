/*
 * =============================================================================
 * NIGHT LIGHT CONTROLLER - FreeRTOS VERSION
 * =============================================================================
 *
 * Turns the RGB matrix into a soft warm-white night light when ambient
 * brightness falls below DARK_LUX_THRESHOLD.
 *
 * FreeRTOS:
 *   Sensor task -> Reads the RPR-0521RS and updates the night light.
 *
 * The task periodically samples the ambient light sensor without blocking
 * the rest of the ESP32-S3 application.
 *
 * Matrix:
 *   GPIO 7
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
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

#define DARK_LUX_THRESHOLD 20

RPR0521RS sensor;

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

SemaphoreHandle_t matrixMutex;


//=============================================================================
// Night Light Task
//=============================================================================

void nightLightTask(void *parameter)
{
  while (true)
  {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(
          &proximity,
          &lux
        ) == 0)
    {
      if (xSemaphoreTake(
            matrixMutex,
            portMAX_DELAY
          ) == pdTRUE)
      {
        matrix.clear();

        if (lux < DARK_LUX_THRESHOLD)
        {
          // Warm-white:
          // high red, medium green and low blue.
          uint32_t warmWhite =
            matrix.Color(
              255,
              120,
              30
            );

          for (int i = 0;
               i < NUM_PIXELS;
               i++)
          {
            matrix.setPixelColor(
              i,
              warmWhite
            );
          }
        }

        matrix.show();

        xSemaphoreGive(matrixMutex);
      }

      Serial.print(
        "Ambient lux: "
      );

      Serial.println(lux);
    }
    else
    {
      Serial.println(
        "Light sensor read failed."
      );
    }

    // Sensor is updated every 500 ms.
    vTaskDelay(
      pdMS_TO_TICKS(500)
    );
  }
}


//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  if (sensor.init() != 0)
  {
    Serial.println(
      "Sensor initialization failed!"
    );

    // Stop this task if the sensor cannot initialize.
    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  matrix.begin();

  // Low overall brightness makes the night light comfortable.
  matrix.setBrightness(30);

  matrix.clear();
  matrix.show();

  matrixMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    nightLightTask,
    "NightLight",
    4096,
    NULL,
    1,
    NULL,
    1
  );

  Serial.println(
    "Night Light FreeRTOS started."
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
