/*
 * This program arms an anti-theft alarm using the push button.
 *
 * Once armed, movement causes an escalating multi-tone siren.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

#define BUZZER_CHANNEL 0

Adafruit_LSM6DSOX imu;

bool armed = false;
float baseline = 9.81;

void setup() {
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  // Initialize buzzer PWM.
  ledcSetup(BUZZER_CHANNEL, 1000, 8);
  ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);
  ledcWrite(BUZZER_CHANNEL, 0);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) while (1);

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
}

void loop() {
  if (digitalRead(BUTTON_PIN) == LOW) {
    armed = true;
    Serial.println("ARMED");

    delay(500);
  }

  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float magnitude = sqrt(
    accel.acceleration.x * accel.acceleration.x +
    accel.acceleration.y * accel.acceleration.y +
    accel.acceleration.z * accel.acceleration.z
  );

  if (armed && fabs(magnitude - baseline) > 1.5) {

    for (int frequency = 600;
         frequency <= 2400;
         frequency += 300) {

      ledcSetup(BUZZER_CHANNEL, frequency, 8);
      ledcWrite(BUZZER_CHANNEL, 128);

      delay(100);
    }

    ledcWrite(BUZZER_CHANNEL, 0);
  }

  delay(30);
}
