/*
 * This program measures the board's pitch relative to a 45-degree target.
 *
 * The buzzer beeps increasingly quickly as the board approaches
 * the target angle.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

unsigned long lastBeep = 0;

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float pitch = atan2(
    -accel.acceleration.x,
    sqrt(
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    )
  ) * 180.0 / PI;

  float error = fabs(45.0 - pitch);

  if (error < 30) {
    int interval = map(
      constrain((int)error, 0, 30),
      0, 30,
      80, 700
    );

    if (millis() - lastBeep > interval) {
      tone(BUZZER_PIN, 1800, 50);
      lastBeep = millis();
    }
  }

  delay(20);
}
