/*
 * This program measures rotation around the Z axis.
 *
 * A bright pixel travels around the matrix edge. Its speed
 * increases as the measured angular velocity increases.
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

const int edgePixels[] = {
  0, 1, 2, 3, 4,
  9, 14, 19, 24,
  23, 22, 21, 20,
  15, 10, 5
};

int position = 0;
unsigned long lastMove = 0;

void setup() {
  matrix.begin();
  matrix.setBrightness(25);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setGyroRange(LSM6DS_GYRO_RANGE_500_DPS);
  imu.setGyroDataRate(LSM6DS_RATE_104_HZ);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float speed = fabs(gyro.gyro.z);

  int interval = map(
    constrain((int)speed, 0, 500),
    0, 500,
    500, 20
  );

  if (millis() - lastMove > interval) {
    position++;

    if (position >= 16)
      position = 0;

    lastMove = millis();
  }

  matrix.clear();

  matrix.setPixelColor(
    edgePixels[position],
    matrix.Color(0, 255, 50)
  );

  matrix.show();

  delay(10);
}
