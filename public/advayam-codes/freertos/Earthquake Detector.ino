/*
 * This program monitors acceleration from the LSM6DSOX
 * and detects strong mechanical vibration.
 *
 * Strong vibrations trigger the buzzer on GPIO 16.
 *
 * The buzzer uses ESP32 LEDC directly instead of tone()
 * so that it works reliably with the ESP32 Arduino core.
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
// Start Buzzer
//====================================================
void buzzerOn()
{
    ledcWriteTone(
        BUZZER_CHANNEL,
        BUZZER_FREQUENCY
    );
}

//====================================================
// Stop Buzzer
//====================================================
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

    // Configure ESP32 LEDC for the buzzer.
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

    Serial.println("Vibration detector ready.");
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

    // Calculate total acceleration magnitude.
    float accelerationMagnitude = sqrt(
        accel.acceleration.x * accel.acceleration.x +
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
    );

    // Remove the approximate 1g gravity component.
    float vibration =
        fabs(accelerationMagnitude - 9.81);

    // Trigger the buzzer when vibration exceeds
    // the configured threshold.
    if (vibration > 2.0)
    {
        buzzerOn();

        Serial.println("VIBRATION!");
    }
    else
    {
        buzzerOff();
    }

    delay(20);
}
