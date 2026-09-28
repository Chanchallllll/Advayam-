/*
 * =============================================================================
 * 45-DEGREE PITCH TARGET
 * =============================================================================
 *
 * This program measures the board's pitch using the LSM6DSOX accelerometer.
 *
 * The buzzer beeps increasingly quickly as the board approaches
 * the 45-degree target angle.
 *
 * The application runs inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *   Buzzer -> GPIO 16
 *   IMU CS   -> GPIO 10
 *   IMU SCK  -> GPIO 12
 *   IMU MOSI -> GPIO 11
 *   IMU MISO -> GPIO 13
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

// =============================================================================
// IMU
// =============================================================================

Adafruit_LSM6DSOX imu;

// =============================================================================
// FreeRTOS task
// =============================================================================

void pitchTask(void *parameter)
{
  unsigned long lastBeep = 0;

  while (true)
  {
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    // Read the IMU.
    imu.getEvent(
      &accel,
      &gyro,
      &temp
    );

    // Calculate board pitch from the accelerometer.
    float pitch = atan2(
      -accel.acceleration.x,
      sqrt(
        accel.acceleration.y *
        accel.acceleration.y +
        accel.acceleration.z *
        accel.acceleration.z
      )
    ) * 180.0 / PI;

    // Calculate the distance from the 45-degree target.
    float error = fabs(45.0 - pitch);

    // Only beep when reasonably close to the target.
    if (error < 30)
    {
      // Smaller error = faster beeping.
      int interval = map(
        constrain((int)error, 0, 30),
        0, 30,
        80, 700
      );

      if (millis() - lastBeep > interval)
      {
        tone(
          BUZZER_PIN,
          1800,
          50
        );

        lastBeep = millis();
      }
    }

    // Run the pitch measurement approximately every 20 ms.
    vTaskDelay(pdMS_TO_TICKS(20));
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup()
{
  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );

  if (!imu.begin_SPI(IMU_CS, &SPI))
  {
    while (true)
    {
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }

  imu.setAccelRange(
    LSM6DS_ACCEL_RANGE_4_G
  );

  imu.setAccelDataRate(
    LSM6DS_RATE_104_HZ
  );

  // Create the pitch measurement task.
  xTaskCreatePinnedToCore(
    pitchTask,
    "Pitch Task",
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
  // All application processing occurs inside pitchTask().
  vTaskDelay(pdMS_TO_TICKS(1000));
}
