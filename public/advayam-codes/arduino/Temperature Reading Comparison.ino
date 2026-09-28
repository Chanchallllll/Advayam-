/*
 * This program compares the LSM6DS internal temperature sensor
 * with the LMT87 analog temperature sensor on GPIO 1.
 *
 * Both temperatures are printed over UART for comparison.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <LMT87.h>

#define LMT_PIN 1

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();
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

  Serial.println("IMU vs LMT87 Temperature");
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  // The minus sign is intentional for this board's LMT87 setup.
  double analogTemp =
    -lmt87.getCDegree(
      LMT_PIN,
      ADCres,
      voltageRef,
      100
    );

  Serial.print("IMU: ");
  Serial.print(temp.temperature, 1);
  Serial.print(" C");

  Serial.print(" | LMT87: ");
  Serial.print(analogTemp, 1);
  Serial.println(" C");

  delay(1000);
}
