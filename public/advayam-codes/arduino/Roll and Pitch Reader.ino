/*
 * This program calculates board roll and pitch from the accelerometer.
 *
 * Values are streamed as CSV over UART for real-time plotting.
 * Output format: time_ms,roll,pitch
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

void setup() {
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    Serial.println("IMU initialization failed!");
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  Serial.println("time_ms,roll,pitch");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float roll = atan2(
    accel.acceleration.y,
    accel.acceleration.z
  ) * 180.0 / PI;

  float pitch = atan2(
    -accel.acceleration.x,
    sqrt(
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    )
  ) * 180.0 / PI;

  Serial.print(millis());
  Serial.print(",");
  Serial.print(roll, 2);
  Serial.print(",");
  Serial.println(pitch, 2);

  delay(50);
}
