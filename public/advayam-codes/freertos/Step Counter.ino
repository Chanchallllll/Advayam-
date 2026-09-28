/*
 * =============================================================================
 * WALKING STEP COUNTER - FREERTOS VERSION
 * =============================================================================
 *
 * Estimates walking steps by detecting peaks in the accelerometer magnitude.
 *
 * Every detected step:
 *   1. Increments the step counter.
 *   2. Prints the count over Serial.
 *   3. Displays the lower 25 bits of the count on the RGB matrix.
 *
 * The step detector runs as a FreeRTOS task.
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

#define STEP_TASK_STACK_SIZE 4096
#define STEP_TASK_PRIORITY   1

TaskHandle_t stepTaskHandle = NULL;

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
// Step Counter
//=============================================================================

unsigned long steps = 0;
unsigned long lastStep = 0;

//=============================================================================
// Display Step Count
//=============================================================================

void displaySteps()
{
  matrix.clear();

  // Display the lower 25 bits of the step count.
  for (int i = 0; i < 25; i++)
  {
    if (steps & (1UL << i))
    {
      matrix.setPixelColor(
        i,
        matrix.Color(0, 255, 0)
      );
    }
  }

  matrix.show();
}

//=============================================================================
// Step Detection Task
//=============================================================================

void stepTask(void *parameter)
{
  while (true)
  {
    sensors_event_t accel, gyro, temp;

    imu.getEvent(
      &accel,
      &gyro,
      &temp
    );

    // Calculate total acceleration magnitude.
    float magnitude = sqrt(
      accel.acceleration.x * accel.acceleration.x +
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    );

    // Detect an acceleration peak.
    //
    // The 300 ms minimum interval prevents one physical step
    // from being counted multiple times.
    if (magnitude > 13.0 &&
        millis() - lastStep > 300)
    {
      steps++;

      lastStep = millis();

      Serial.print("Steps: ");
      Serial.println(steps);

      displaySteps();
    }

    // Sample the accelerometer approximately every 20 ms.
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

  displaySteps();

  // Create the step detection task.
  xTaskCreate(
    stepTask,
    "StepTask",
    STEP_TASK_STACK_SIZE,
    NULL,
    STEP_TASK_PRIORITY,
    &stepTaskHandle
  );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  // Step detection is handled by FreeRTOS.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
