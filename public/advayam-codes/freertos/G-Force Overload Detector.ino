/*
 * This program monitors total acceleration.
 *
 * If acceleration exceeds 2.5g, the matrix flashes red
 * and the buzzer produces a high-frequency warning chirp.
 *
 * The buzzer uses ESP32 LEDC directly instead of tone().
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// Buzzer / LEDC Configuration
//====================================================
#define BUZZER_CHANNEL    0
#define BUZZER_FREQUENCY  3500
#define BUZZER_RESOLUTION 8

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

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
    // Initialize RGB matrix.
    matrix.begin();

    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();

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
        Serial.println(
            "IMU initialization failed!"
        );

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

    // Calculate total acceleration magnitude
    // and convert it from m/s² to g.
    float g = sqrt(
        accel.acceleration.x * accel.acceleration.x +
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
    ) / 9.80665;

    // High acceleration detected.
    if (g > 2.5)
    {
        // Flash the entire matrix red.
        matrix.fill(
            matrix.Color(255, 0, 0)
        );

        matrix.show();

        // Produce a short high-frequency chirp.
        buzzerOn();

        delay(120);

        buzzerOff();

        // Turn the matrix back off.
        matrix.clear();
        matrix.show();
    }

    delay(20);
}
