/*
 * This program detects a quick left-to-right flick of the board.
 *
 * Each detected gesture changes the RGB matrix to the next
 * color in a predefined palette.
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

uint8_t palette = 0;
unsigned long lastGesture = 0;

uint32_t colors[] = {
  0xFF0000,
  0x00FF00,
  0x0000FF,
  0xFFFF00,
  0xFF00FF,
  0x00FFFF
};

void showColor() {
  matrix.fill(colors[palette]);
  matrix.show();
}

void setup() {
  matrix.begin();
  matrix.setBrightness(25);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  showColor();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  if (accel.acceleration.y < -14 &&
      millis() - lastGesture > 500) {

    palette++;
    palette %= 6;

    showColor();

    lastGesture = millis();
  }

  delay(20);
}
