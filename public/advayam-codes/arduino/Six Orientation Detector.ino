/*
 * This program determines which of six directions the board is facing.
 *
 * The matrix displays a colored pixel indicating Up, Down, Left,
 * Right, Forward, or Backward.
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

void setup() {
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  matrix.begin();
  matrix.setBrightness(25);

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

  float ax = accel.acceleration.x;
  float ay = accel.acceleration.y;
  float az = accel.acceleration.z;

  float absX = fabs(ax);
  float absY = fabs(ay);
  float absZ = fabs(az);

  matrix.clear();

  String orientation;

  if (absZ >= absX && absZ >= absY) {
    orientation = az > 0 ? "UP" : "DOWN";
    matrix.setPixelColor(
      12,
      matrix.Color(255, 255, 0)
    );
  }
  else if (absY >= absX) {
    orientation = ay > 0 ? "RIGHT" : "LEFT";
    matrix.setPixelColor(
      12,
      matrix.Color(0, 255, 0)
    );
  }
  else {
    orientation = ax > 0 ? "FORWARD" : "BACKWARD";
    matrix.setPixelColor(
      12,
      matrix.Color(0, 0, 255)
    );
  }

  matrix.show();

  Serial.println(orientation);

  delay(250);
}
