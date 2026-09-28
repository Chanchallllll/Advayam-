/*
 * This program estimates walking steps from acceleration peaks.
 *
 * Each detected step increments the counter and displays the
 * current count in binary on the 5x5 matrix.
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

unsigned long steps = 0;
unsigned long lastStep = 0;

void displaySteps() {
  matrix.clear();

  // Display the lower 25 bits of the count.
  for (int i = 0; i < 25; i++) {
    if (steps & (1UL << i)) {
      matrix.setPixelColor(i, matrix.Color(0, 255, 0));
    }
  }

  matrix.show();
}

void setup() {
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  displaySteps();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float magnitude = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  // Walking creates repeated acceleration peaks.
  if (magnitude > 13.0 &&
      millis() - lastStep > 300) {

    steps++;
    lastStep = millis();

    Serial.print("Steps: ");
    Serial.println(steps);

    displaySteps();
  }

  delay(20);
}
