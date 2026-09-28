/*
 * This program provides a precision digital spirit level
 * using an IMU task running on FreeRTOS.
 *
 * Green means the board is level. Yellow indicates a small error
 * and red indicates a larger tilt.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;
Adafruit_NeoPixel matrix(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

// FreeRTOS task for reading IMU and updating the spirit level display
void spiritLevelTask(void *pvParameters)
{
  while (true)
  {
    sensors_event_t accel, gyro, temp;
    imu.getEvent(&accel, &gyro, &temp);

    float roll = atan2(
      -accel.acceleration.y,
      accel.acceleration.z
    ) * 180.0 / PI;

    float pitch = atan2(
      accel.acceleration.x,
      sqrt(
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
      )
    ) * 180.0 / PI;

    float error = max(
      fabs(roll),
      fabs(pitch)
    );

    uint32_t color;

    if (error < 2)
      color = matrix.Color(0, 255, 0);
    else if (error < 8)
      color = matrix.Color(255, 150, 0);
    else
      color = matrix.Color(255, 0, 0);

    matrix.clear();
    matrix.setPixelColor(12, color);
    matrix.show();

    vTaskDelay(pdMS_TO_TICKS(30));
  }
}

void setup() {
  matrix.begin();
  matrix.setBrightness(25);
  pinMode(16, OUTPUT);
  digitalWrite(16, LOW);
  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  // Create FreeRTOS task for sensor processing
  xTaskCreate(
    spiritLevelTask,
    "LevelTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop() {
  vTaskDelete(NULL);
}