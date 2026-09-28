/*
 * This program uses the accelerometer to create a digital bubble level.
 *
 * The green center LED represents the bubble. The bubble moves
 * according to board tilt and stays centered when the board is level.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;
Adafruit_NeoPixel matrix(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

// Convert x/y coordinates to the serpentine LED index.
int pixelXY(int x, int y) {
  if (y % 2 == 0) return y * 5 + x;
  return y * 5 + (4 - x);
}

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

  Serial.println("Digital Bubble Level");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  // Convert acceleration into approximate tilt position.
  int x = constrain(
    2 - round(accel.acceleration.y / 2.5),
    0, 4
  );

  int y = constrain(
    2 + round(accel.acceleration.x / 2.5),
    0, 4
  );

  matrix.clear();

  matrix.setPixelColor(
    pixelXY(x, y),
    matrix.Color(0, 255, 0)
  );

  matrix.show();

  delay(50);
}
