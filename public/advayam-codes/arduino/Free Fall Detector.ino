/*
 * This program detects free-fall using the LSM6DS accelerometer.
 *
 * When total acceleration drops close to zero, the buzzer on
 * GPIO 16 sounds immediately. GPIO 38 is used as the IMU interrupt input.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUZZER_PIN 16
#define IMU_INT 38

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

volatile bool imuInterrupt = false;

void IRAM_ATTR imuISR() {
  imuInterrupt = true;
}

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(IMU_INT, INPUT);

  attachInterrupt(
    digitalPinToInterrupt(IMU_INT),
    imuISR,
    RISING
  );

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    Serial.println("IMU initialization failed!");
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  Serial.println("Free-Fall Alarm Ready");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  float x = accel.acceleration.x;
  float y = accel.acceleration.y;
  float z = accel.acceleration.z;

  float magnitude = abs(z);

  // Free fall means acceleration approaches zero.
  if (magnitude < 2.0) {
    Serial.println("FREE FALL DETECTED!");

    tone(BUZZER_PIN, 2500);

    delay(5000);

    noTone(BUZZER_PIN);

    Serial.println("Free-Fall Alarm Ready");

    delay(100);
  }

  imuInterrupt = false;

  delay(10);
}
