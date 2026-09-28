/*
 * This program turns board tilt into sound.
 *
 * Pitch is controlled by board pitch and buzzer duty cycle is
 * controlled by board roll.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

#define BUZZER_CHANNEL 0

Adafruit_LSM6DSOX imu;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  // ESP32 LEDC PWM setup.
  ledcSetup(BUZZER_CHANNEL, 1000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float pitch = atan2(
    accel.acceleration.x,
    sqrt(
      accel.acceleration.y * accel.acceleration.y +
      accel.acceleration.z * accel.acceleration.z
    )
  ) * 180.0 / PI;

  float roll = atan2(
    -accel.acceleration.y,
    accel.acceleration.z
  ) * 180.0 / PI;

  int frequency = map(
    constrain((int)pitch, -45, 45),
    -45, 45,
    200, 2000
  );

  int duty = map(
    constrain((int)fabs(roll), 0, 45),
    0, 45,
    255, 40
  );

  // ESP32 LEDC generates PWM audio.
  ledcWriteTone(BUZZER_CHANNEL, frequency);
  ledcWrite(BUZZER_CHANNEL, duty);

  delay(20);
}
