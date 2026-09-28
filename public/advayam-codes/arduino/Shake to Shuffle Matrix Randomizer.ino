/*
 * This program detects a strong shake using the accelerometer.
 *
 * A rapid acceleration change triggers a randomized burst of
 * colors across the 5x5 RGB matrix.
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

float previousMagnitude = 1.0;

void setup() {
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    Serial.println("IMU initialization failed!");
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  randomSeed(analogRead(1));
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float magnitude = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  float change = fabs(magnitude - previousMagnitude);

  if (change > 10.0) {
    Serial.println("SHAKE!");

    for (int i = 0; i < 25; i++) {
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
    delay(150);

    matrix.clear();
    matrix.show();
  }

  previousMagnitude = magnitude;

  delay(20);
}
