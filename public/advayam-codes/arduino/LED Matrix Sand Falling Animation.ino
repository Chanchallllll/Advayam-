/*
 * This program simulates sand particles on the 5x5 matrix.
 *
 * Board tilt determines which direction the sand falls.
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

bool sand[5][5];

int pixelXY(int x, int y) {
  if (y % 2 == 0) return y * 5 + x;
  return y * 5 + (4 - x);
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

  randomSeed(analogRead(1));

  // Start with a random pile.
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      sand[y][x] = random(100) < 45;
    }
  }
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  int dx = 0;
  int dy = 0;

  if (fabs(accel.acceleration.x) > 2)
    dy = accel.acceleration.x > 0 ? -1 : 1;

  if (fabs(accel.acceleration.y) > 2)
    dx = accel.acceleration.y > 0 ? 1 : -1;

  bool next[5][5] = {};

  // Move each grain in the tilt direction.
  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {

      if (!sand[y][x]) continue;

      int nx = constrain(x + dx, 0, 4);
      int ny = constrain(y + dy, 0, 4);

      if (!next[ny][nx])
        next[ny][nx] = true;
      else
        next[y][x] = true;
    }
  }

  memcpy(sand, next, sizeof(sand));

  matrix.clear();

  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      if (sand[y][x]) {
        matrix.setPixelColor(
          pixelXY(x, y),
          matrix.Color(255, 180, 30)
        );
      }
    }
  }

  matrix.show();

  delay(100);
}
