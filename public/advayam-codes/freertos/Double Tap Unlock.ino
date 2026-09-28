/*
 * =============================================================================
 * CODE 5 - DOUBLE-TAP UNLOCK
 * =============================================================================
 *
 * Detects two separate acceleration spikes within 500 ms.
 *
 * Successful double tap:
 *   - Prints "DOUBLE TAP - UNLOCKED"
 *   - Lights center matrix pixel green
 *
 * IMU:
 *   LSM6DSOX
 *
 * Matrix:
 *   GPIO 7
 *
 * SPI:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

//====================================================
// Matrix Configuration
//====================================================

#define MATRIX_PIN 7
#define NUM_LEDS 25

//====================================================
// IMU SPI Configuration
//====================================================

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// Double-Tap Configuration
//====================================================

// Acceleration threshold in m/s².
//
// 16 m/s² is approximately 1.63 g.
#define TAP_THRESHOLD 16.0

// Maximum time allowed between first and second tap.
#define DOUBLE_TAP_WINDOW 500

//====================================================
// Hardware Objects
//====================================================

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//====================================================
// Double-Tap State
//====================================================

unsigned long firstTapTime = 0;

bool waitingSecondTap = false;

// Prevents one continuous acceleration spike from
// being counted as multiple taps.
bool waitingForRelease = false;

//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(115200);

    // Initialize NeoPixel matrix.
    matrix.begin();

    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();

    // Initialize SPI.
    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    // Initialize LSM6DSOX over SPI.
    if (!imu.begin_SPI(
            IMU_CS,
            &SPI
        ))
    {
        Serial.println("LSM6DSOX initialization failed!");

        while (1)
        {
            delay(1000);
        }
    }

    // Configure accelerometer range.
    imu.setAccelRange(
        LSM6DS_ACCEL_RANGE_4_G
    );

    // Configure accelerometer sample rate.
    imu.setAccelDataRate(
        LSM6DS_RATE_104_HZ
    );

    Serial.println("Double-Tap Unlock");
    Serial.println("Tap the device twice quickly.");
}

//====================================================
// Show Unlock
//====================================================

void unlockDisplay()
{
    // Clear matrix.
    matrix.clear();

    // Light center pixel.
    matrix.setPixelColor(
        12,
        matrix.Color(
            0,
            255,
            0
        )
    );

    matrix.show();

    Serial.println("DOUBLE TAP - UNLOCKED");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    // Read IMU.
    imu.getEvent(
        &accel,
        &gyro,
        &temp
    );

    // Calculate total acceleration magnitude.
    float magnitude = sqrt(
        accel.acceleration.x *
        accel.acceleration.x +

        accel.acceleration.y *
        accel.acceleration.y +

        accel.acceleration.z *
        accel.acceleration.z
    );

    // Determine whether acceleration is currently
    // above the tap threshold.
    bool tapSignal =
        magnitude > TAP_THRESHOLD;

    //================================================
    // Wait for acceleration to fall again
    //================================================

    if (waitingForRelease)
    {
        // Do not accept another tap until the
        // acceleration falls below the threshold.
        if (!tapSignal)
        {
            waitingForRelease = false;
        }

        delay(10);
        return;
    }

    //================================================
    // Detect a new acceleration spike
    //================================================

    if (tapSignal)
    {
        unsigned long now = millis();

        // First tap.
        if (!waitingSecondTap)
        {
            firstTapTime = now;

            waitingSecondTap = true;

            Serial.println("First tap detected.");
        }

        // Second tap.
        else
        {
            if (now - firstTapTime <= DOUBLE_TAP_WINDOW)
            {
                unlockDisplay();

                waitingSecondTap = false;
            }
            else
            {
                // The first tap was too old.
                // Treat this as a new first tap.
                firstTapTime = now;

                waitingSecondTap = true;

                Serial.println("First tap detected.");
            }
        }

        // Wait for acceleration to return to normal.
        waitingForRelease = true;
    }

    //================================================
    // Expire double-tap window
    //================================================

    if (waitingSecondTap)
    {
        if (millis() - firstTapTime >
            DOUBLE_TAP_WINDOW)
        {
            waitingSecondTap = false;

            Serial.println("Double-tap window expired.");
        }
    }

    delay(10);
}
    