/*
 * This program detects two rapid acceleration spikes as a double tap.
 *
 * A verified double tap unlocks the system and lights the center
 * matrix pixel green.
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

unsigned long firstTap = 0;
bool waitingSecond = false;

void setup() {
  matrix.begin();
  matrix.setBrightness(25);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float magnitude = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  bool tap = magnitude > 16.0;

  if (tap) {
    if (!waitingSecond) {
      firstTap = millis();
      waitingSecond = true;
    }
    else if (millis() - firstTap < 500) {
      matrix.clear();

      matrix.setPixelColor(
        12,
        matrix.Color(0, 255, 0)
      );

      matrix.show();

      Serial.println("DOUBLE TAP - UNLOCKED");

      waitingSecond = false;
      delay(300);
    }
  }

  if (waitingSecond &&
      millis() - firstTap > 500) {
    waitingSecond = false;
  }

  delay(20);
}
