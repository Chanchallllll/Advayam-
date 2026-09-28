/*
 * This program learns a quiet acceleration baseline and monitors
 * mechanical vibration around that baseline.
 *
 * Strong vibrations trigger the buzzer on GPIO 16.
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

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  Serial.println("Vibration detector ready.");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float vibration = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  vibration = fabs(vibration - 9.81);

  if (vibration > 2.0) {
    tone(BUZZER_PIN, 1800);
    Serial.println("VIBRATION!");
  }
  else {
    noTone(BUZZER_PIN);
  }

  delay(20);
}
