/*
 * This program creates a virtual six-sided die.
 *
 * Shaking or tilting the board makes the die roll. When the
 * board becomes flat again, the result settles between 1 and 6.
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

bool rolling = false;
unsigned long rollStart;

void showNumber(int n) {
  matrix.clear();

  // Simple binary representation.
  for (int i = 0; i < n; i++) {
    matrix.setPixelColor(
      i,
      matrix.Color(255, 255, 255)
    );
  }

  matrix.show();
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
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float tilt = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y
  );

  if (tilt > 5 && !rolling) {
    rolling = true;
    rollStart = millis();
  }

  if (rolling) {
    showNumber(random(1, 7));

    if (tilt < 2 &&
        millis() - rollStart > 500) {

      showNumber(random(1, 7));

      rolling = false;
    }

    delay(100);
  }
}
