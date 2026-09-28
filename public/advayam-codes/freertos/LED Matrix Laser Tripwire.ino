/*
 * =============================================================================
 * LASER TRIPWIRE / BEAM BREAK ALARM - FreeRTOS
 * =============================================================================
 *
 * Uses the RPR-0521RS ambient-light sensor as a beam-break detector.
 *
 * A steady light source illuminates the sensor.
 *
 * When an object blocks the beam:
 *
 *   measured lux < baseline lux - configured percentage
 *
 * The buzzer sounds continuously until the beam is restored.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * Buzzer:
 *   GPIO 16
 *
 * FreeRTOS tasks:
 *
 *   BaselineTask
 *       Establishes the initial light level.
 *
 *   MonitorTask
 *       Continuously monitors the beam.
 *
 * A task notification is used to tell MonitorTask when the baseline has
 * been successfully established.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define BUZZER_PIN 16

#define BEAM_BREAK_PERCENT 30

#define BASELINE_SAMPLES 20
#define BASELINE_INTERVAL_MS 100

#define MONITOR_INTERVAL_MS 50

RPR0521RS sensor;

float baselineLux = 0;

SemaphoreHandle_t sensorMutex;
TaskHandle_t monitorTaskHandle = NULL;

//=============================================================================
// Establish Light Baseline
//=============================================================================

void baselineTask(void *parameter)
{
  float sum = 0;
  int validSamples = 0;

  Serial.println(
    "Establishing light baseline..."
  );

  for (int i = 0; i < BASELINE_SAMPLES; i++)
  {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(
          &proximity,
          &lux) == 0)
    {
      sum += lux;
      validSamples++;
    }

    vTaskDelay(
      pdMS_TO_TICKS(BASELINE_INTERVAL_MS)
    );
  }

  if (validSamples > 0)
  {
    if (xSemaphoreTake(
          sensorMutex,
          portMAX_DELAY) == pdTRUE)
    {
      baselineLux =
        sum / validSamples;

      xSemaphoreGive(sensorMutex);
    }

    Serial.print("Baseline lux: ");
    Serial.println(baselineLux);

    Serial.println(
      "Tripwire armed."
    );

    // Tell MonitorTask that baseline setup is complete.
    xTaskNotifyGive(
      monitorTaskHandle
    );
  }
  else
  {
    Serial.println(
      "Unable to establish baseline."
    );
  }

  // Baseline only needs to be calculated once.
  vTaskDelete(NULL);
}

//=============================================================================
// Beam Monitoring Task
//=============================================================================

void monitorTask(void *parameter)
{
  // Wait until the baseline task has completed.
  ulTaskNotifyTake(
    pdTRUE,
    portMAX_DELAY
  );

  while (true)
  {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(
          &proximity,
          &lux) == 0)
    {
      float baseline;

      if (xSemaphoreTake(
            sensorMutex,
            portMAX_DELAY) == pdTRUE)
      {
        baseline = baselineLux;

        xSemaphoreGive(sensorMutex);
      }

      float triggerLevel =
        baseline *
        (1.0 -
         BEAM_BREAK_PERCENT / 100.0);

      if (lux < triggerLevel)
      {
        // Beam has been interrupted.
        tone(
          BUZZER_PIN,
          2000
        );

        Serial.print(
          "BEAM BROKEN! Lux = "
        );

        Serial.println(lux);
      }
      else
      {
        // Beam restored.
        noTone(BUZZER_PIN);
      }
    }

    vTaskDelay(
      pdMS_TO_TICKS(MONITOR_INTERVAL_MS)
    );
  }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  noTone(BUZZER_PIN);

  Wire.begin(
    I2C_SDA,
    I2C_SCL
  );

  if (sensor.init() != 0)
  {
    Serial.println(
      "Sensor initialization failed!"
    );

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  sensorMutex =
    xSemaphoreCreateMutex();

  // Create monitor task first so its handle is available.
  xTaskCreate(
    monitorTask,
    "MonitorTask",
    4096,
    NULL,
    2,
    &monitorTaskHandle
  );

  xTaskCreate(
    baselineTask,
    "BaselineTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // The FreeRTOS tasks perform the tripwire operation.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
