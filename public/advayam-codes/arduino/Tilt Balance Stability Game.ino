/*
 * This program challenges the user to keep the board level.
 *
 * The center LED remains green while stable. The player must
 * keep the bubble inside the center area for 15 seconds.
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

unsigned long stableStart = 0;
bool timing = false;

void setup() {
  Serial.begin(115200);

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

  float roll = atan2(
    -accel.acceleration.y,
    accel.acceleration.z
  ) * 180.0 / PI;

  float pitch = atan2(
    accel.acceleration.x,
    sqrt(
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    )
  ) * 180.0 / PI;

  bool stable =
    fabs(roll) < 3.0 &&
    fabs(pitch) < 3.0;

  if (stable) {
    if (!timing) {
      stableStart = millis();
      timing = true;
    }

    matrix.setPixelColor(
      12,
      matrix.Color(0, 255, 0)
    );

    if (millis() - stableStart >= 15000) {
      matrix.fill(matrix.Color(0, 255, 0));
      matrix.show();

      Serial.println("SUCCESS!");
      delay(1000);

      timing = false;
    }
  }
  else {
    timing = false;

    matrix.clear();
    matrix.setPixelColor(
      12,
      matrix.Color(255, 0, 0)
    );
  }

  matrix.show();

  delay(30);
}
