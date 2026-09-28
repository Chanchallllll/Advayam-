/*
 * =============================================================================
 * STRONG SHAKE RGB BURST - FREERTOS VERSION
 * =============================================================================
 *
 * A FreeRTOS task continuously monitors the accelerometer.
 *
 * A rapid change in acceleration magnitude is interpreted as a strong shake.
 * When a shake is detected, all 25 RGB LEDs display randomized colors briefly.
 *
 * Hardware:
 *   RGB Matrix : GPIO 7
 *   IMU CS     : GPIO 10
 *   IMU SCK    : GPIO 12
 *   IMU MOSI   : GPIO 11
 *   IMU MISO   : GPIO 13
 *
 * The application logic runs inside a FreeRTOS task instead of the Arduino
 * loop(). vTaskDelay() is used so the task gives CPU time back to FreeRTOS.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// Hardware Configuration
//=============================================================================

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define IMU_CS     10
#define IMU_SCK    12
#define IMU_MOSI   11
#define IMU_MISO   13

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define SHAKE_TASK_STACK_SIZE 4096
#define SHAKE_TASK_PRIORITY   1

TaskHandle_t shakeTaskHandle = NULL;

//=============================================================================
// Hardware Objects
//=============================================================================

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// Previous acceleration magnitude used for shake detection.
float previousMagnitude = 1.0;

//=============================================================================
// Shake Task
//=============================================================================

void shakeTask(void *parameter)
{
  while (true)
  {
    sensors_event_t accel, gyro, temp;

    imu.getEvent(
      &accel,
      &gyro,
      &temp
    );

    // Calculate the total acceleration magnitude.
    float magnitude = sqrt(
      accel.acceleration.x * accel.acceleration.x +
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    );

    // Calculate how quickly the acceleration changed.
    float change =
      fabs(magnitude - previousMagnitude);

    // A large acceleration change indicates a strong shake.
    if (change > 10.0)
    {
      Serial.println("SHAKE!");

      // Fill the matrix with random colors.
      for (int i = 0; i < NUM_LEDS; i++)
      {
        matrix.setPixelColor(
          i,
          matrix.Color(
            random(256),
            random(256),
            random(256)
          )
        );
      }

      matrix.show();

      // Keep the color burst visible for 150 ms.
      vTaskDelay(
        pdMS_TO_TICKS(150)
      );

      matrix.clear();
      matrix.show();
    }

    previousMagnitude = magnitude;

    // Sensor is sampled approximately every 20 ms.
    vTaskDelay(
      pdMS_TO_TICKS(20)
    );
  }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  // Initialize SPI for the LSM6DSOX.
  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );

  if (!imu.begin_SPI(IMU_CS, &SPI))
  {
    Serial.println("IMU initialization failed!");

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  imu.setAccelRange(
    LSM6DS_ACCEL_RANGE_4_G
  );

  imu.setAccelDataRate(
    LSM6DS_RATE_104_HZ
  );

  randomSeed(
    analogRead(1)
  );

  // Create the FreeRTOS application task.
  xTaskCreate(
    shakeTask,
    "ShakeTask",
    SHAKE_TASK_STACK_SIZE,
    NULL,
    SHAKE_TASK_PRIORITY,
    &shakeTaskHandle
  );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  // Application logic is handled by the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
