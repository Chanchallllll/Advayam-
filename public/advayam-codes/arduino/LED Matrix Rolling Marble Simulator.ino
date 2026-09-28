/*
 * This program simulates a rolling marble on the 5x5 matrix.
 *
 * Board tilt accelerates the marble while simulated friction
 * gradually slows it down.
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

float x = 2;
float y = 2;
float vx = 0;
float vy = 0;

int pixelXY(int px, int py) {
  if (py % 2 == 0) return py * 5 + px;
  return py * 5 + (4 - px);
}

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

  // Tilt supplies acceleration.
  vx += accel.acceleration.y * 0.002;
  vy -= accel.acceleration.x * 0.002;

  // Surface friction.
  vx *= 0.97;
  vy *= 0.97;

  x += vx;
  y += vy;

  // Bounce off the edges.
  if (x < 0) {
    x = 0;
    vx = -vx;
  }

  if (x > 4) {
    x = 4;
    vx = -vx;
  }

  if (y < 0) {
    y = 0;
    vy = -vy;
  }

  if (y > 4) {
    y = 4;
    vy = -vy;
  }

  matrix.clear();

  matrix.setPixelColor(
    pixelXY(round(x), round(y)),
    matrix.Color(255, 180, 0)
  );

  matrix.show();

  delay(30);
}
