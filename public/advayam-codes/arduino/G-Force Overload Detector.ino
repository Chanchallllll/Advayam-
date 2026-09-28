/*
 * This program monitors total acceleration.
 *
 * If acceleration exceeds 2.5g, the matrix flashes red and
 * the buzzer produces a high-frequency warning chirp.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25
#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;
Adafruit_NeoPixel matrix(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  pinMode(BUZZER_PIN, OUTPUT);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float g = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  ) / 9.80665;

  if (g > 2.5) {
    matrix.fill(matrix.Color(255, 0, 0));
    matrix.show();

    tone(BUZZER_PIN, 3500);
    delay(120);
    noTone(BUZZER_PIN);

    matrix.clear();
    matrix.show();
  }

  delay(20);
}
