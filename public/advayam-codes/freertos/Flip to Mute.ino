/*
 * This program demonstrates flip-to-mute behavior.
 *
 * A buzzer alarm runs until the board is turned face down.
 * Face-down orientation immediately silences the buzzer.
 *
 * The buzzer uses ESP32 LEDC directly instead of tone().
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// Buzzer / LEDC Configuration
//====================================================
#define BUZZER_CHANNEL    0
#define BUZZER_FREQUENCY  1800
#define BUZZER_RESOLUTION 8

Adafruit_LSM6DSOX imu;

//====================================================
// Buzzer Control
//====================================================
void buzzerOn()
{
    ledcWriteTone(
        BUZZER_CHANNEL,
        BUZZER_FREQUENCY
    );
}

void buzzerOff()
{
    ledcWriteTone(
        BUZZER_CHANNEL,
        0
    );
}

void setup()
{
    Serial.begin(115200);

    // Configure the buzzer using ESP32 LEDC.
    ledcSetup(
        BUZZER_CHANNEL,
        BUZZER_FREQUENCY,
        BUZZER_RESOLUTION
    );

    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    buzzerOff();

    // Start SPI communication with the IMU.
    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println("IMU initialization failed!");

        while (1)
        {
            delay(100);
        }
    }

    imu.setAccelRange(
        LSM6DS_ACCEL_RANGE_4_G
    );

    imu.setAccelDataRate(
        LSM6DS_RATE_104_HZ
    );

    // Start the alarm immediately.
    buzzerOn();
}

void loop()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    imu.getEvent(
        &accel,
        &gyro,
        &temp
    );

    // Negative Z acceleration indicates that the board
    // is approximately face-down.
    if (accel.acceleration.z < -7.0)
    {
        buzzerOff();

        Serial.println("Alarm muted.");
    }
    else
    {
        buzzerOn();
    }

    delay(50);
}
