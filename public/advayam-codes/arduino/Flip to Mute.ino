/*
 * This program demonstrates flip-to-mute behavior.
 *
 * A buzzer alarm runs until the board is turned face down.
 * Face-down orientation immediately silences the buzzer.
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

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  tone(BUZZER_PIN, 1800);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  // Negative Z indicates face-down orientation.
  if (accel.acceleration.z < -7.0) {
    noTone(BUZZER_PIN);
    Serial.println("Alarm muted.");
  }
  else {
    tone(BUZZER_PIN, 1800);
  }

  delay(50);
}
