/*
 * This program detects free-fall using the LSM6DS
 * accelerometer.
 *
 * When total acceleration drops close to zero,
 * the buzzer on GPIO 16 sounds.
 *
 * GPIO 38 is retained as the IMU interrupt input.
 *
 * The free-fall calculation uses the total acceleration
 * magnitude rather than only the Z axis.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define BUZZER_PIN 16
#define IMU_INT    38

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// Buzzer / LEDC Configuration
//====================================================
#define BUZZER_CHANNEL    0
#define BUZZER_FREQUENCY  2500
#define BUZZER_RESOLUTION 8

Adafruit_LSM6DSOX imu;

volatile bool imuInterrupt = false;

//====================================================
// IMU Interrupt Service Routine
//====================================================
void IRAM_ATTR imuISR()
{
    imuInterrupt = true;
}

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

    // Configure buzzer using ESP32 LEDC.
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

    // Configure the IMU interrupt input.
    pinMode(
        IMU_INT,
        INPUT
    );

    attachInterrupt(
        digitalPinToInterrupt(IMU_INT),
        imuISR,
        RISING
    );

    // Start SPI communication.
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

    Serial.println("Free-Fall Alarm Ready");
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
    float magnitude = sqrt(
        accel.acceleration.x * accel.acceleration.x +
        accel.acceleration.y * accel.acceleration.y +
        accel.acceleration.z * accel.acceleration.z
    );

    // Free fall means the measured acceleration
    // approaches zero on all three axes.
    if (magnitude < 2.0)
    {
        Serial.println(
            "FREE FALL DETECTED!"
        );

        buzzerOn();

        // Keep the alarm active for 5 seconds.
        delay(5000);

        buzzerOff();

        Serial.println(
            "Free-Fall Alarm Ready"
        );

        delay(100);
    }

    // Clear the software interrupt flag.
    imuInterrupt = false;

    delay(10);
}
