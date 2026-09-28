/*
 * =============================================================================
 * COLOUR TEMPERATURE SIMULATOR FROM AMBIENT LUX
 * =============================================================================
 *
 * Adjusts the RGB matrix colour according to ambient light:
 *
 *   Bright room -> cool white
 *   Dim room    -> warm amber
 *
 * Ambient light is measured using the RPR-0521RS sensor.
 *
 * Hardware:
 *   I2C SDA -> GPIO 5
 *   I2C SCL -> GPIO 6
 *   Matrix  -> GPIO 7
 *
 * The sensor measurement and matrix update run inside
 * a dedicated FreeRTOS task.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

// =============================================================================
// Sensor and matrix
// =============================================================================

RPR0521RS sensor;

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Ambient light FreeRTOS task
// =============================================================================

void ambientLightTask(void *parameter)
{
  while (true)
  {
    unsigned short proximity;
    float lux;

    // Read proximity and ambient-light data.
    if (
      sensor.get_psalsval(
        &proximity,
        &lux
      ) == 0
    )
    {
      // Convert 0-500 lux into a warm-to-cool colour transition.
      float ratio =
        constrain(
          lux,
          0.0,
          500.0
        ) / 500.0;

      int red = 255;

      int green =
        80 + (int)(175 * ratio);

      int blue =
        (int)(255 * ratio);

      uint32_t colour =
        matrix.Color(
          red,
          green,
          blue
        );

      // Apply the calculated colour to every pixel.
      for (
        int i = 0;
        i < NUM_PIXELS;
        i++
      )
      {
        matrix.setPixelColor(
          i,
          colour
        );
      }

      matrix.show();

      Serial.print(
        "Lux: "
      );

      Serial.print(lux);

      Serial.print(
        " | RGB: "
      );

      Serial.print(red);

      Serial.print(",");

      Serial.print(green);

      Serial.print(",");

      Serial.println(blue);
    }

    // Update the display every 300 ms.
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

    // Keep the Arduino task alive while
    // preventing a busy loop.
    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  matrix.begin();

  matrix.setBrightness(100);

  matrix.clear();
  matrix.show();

  // Create the ambient-light task.
  xTaskCreatePinnedToCore(
    ambientLightTask,
    "Ambient Light Task",
    4096,
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
  // Ambient-light processing is handled by ambientLightTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
