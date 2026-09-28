/*
 * =============================================================================
 * SIX-DIRECTION ORIENTATION DETECTOR - FREERTOS VERSION
 * =============================================================================
 *
 * Determines which major direction the board is facing using the accelerometer.
 *
 * Possible orientations:
 *   UP
 *   DOWN
 *   LEFT
 *   RIGHT
 *   FORWARD
 *   BACKWARD
 *
 * The center LED indicates the detected orientation.
 *
 * LED colors:
 *   Yellow = UP / DOWN
 *   Green  = LEFT / RIGHT
 *   Blue   = FORWARD / BACKWARD
 *
 * The orientation detection runs inside a FreeRTOS task.
 * vTaskDelay() controls the sensor update interval.
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

#define ORIENTATION_TASK_STACK_SIZE 4096
#define ORIENTATION_TASK_PRIORITY   1

TaskHandle_t orientationTaskHandle = NULL;

//=============================================================================
// Hardware
//=============================================================================

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Orientation Task
//=============================================================================

void orientationTask(void *parameter)
{
  while (true)
  {
    sensors_event_t accel, gyro, temp;

    imu.getEvent(
      &accel,
      &gyro,
      &temp
    );

    float ax = accel.acceleration.x;
    float ay = accel.acceleration.y;
    float az = accel.acceleration.z;

    float absX = fabs(ax);
    float absY = fabs(ay);
    float absZ = fabs(az);

    matrix.clear();

    String orientation;

    // Z axis has the strongest acceleration.
    if (absZ >= absX && absZ >= absY)
    {
      orientation =
        az > 0 ? "UP" : "DOWN";

      matrix.setPixelColor(
        12,
        matrix.Color(255, 255, 0)
      );
    }

    // Y axis has the strongest acceleration.
    else if (absY >= absX)
    {
      orientation =
        ay > 0 ? "RIGHT" : "LEFT";

      matrix.setPixelColor(
        12,
        matrix.Color(0, 255, 0)
      );
    }

    // X axis has the strongest acceleration.
    else
    {
      orientation =
        ax > 0 ? "FORWARD" : "BACKWARD";

      matrix.setPixelColor(
        12,
        matrix.Color(0, 0, 255)
      );
    }

    matrix.show();

    Serial.println(orientation);

    // Update orientation approximately every 250 ms.
    vTaskDelay(
      pdMS_TO_TICKS(250)
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

  // Start the orientation FreeRTOS task.
  xTaskCreate(
    orientationTask,
    "OrientationTask",
    ORIENTATION_TASK_STACK_SIZE,
    NULL,
    ORIENTATION_TASK_PRIORITY,
    &orientationTaskHandle
  );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  // All application work is performed by the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
