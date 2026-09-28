/*
 * This program detects a quick flick of the board.
 *
 * Each detected gesture changes the RGB matrix
 * to the next color in a predefined palette.
 *
 * The original gesture condition is retained:
 * a strong negative Y-axis acceleration is treated
 * as the flick gesture.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

// Current color in the palette.
uint8_t palette = 0;

// Time of the last detected gesture.
unsigned long lastGesture = 0;

//====================================================
// Color Palette
//====================================================
uint32_t colors[] = {
    0xFF0000,  // Red
    0x00FF00,  // Green
    0x0000FF,  // Blue
    0xFFFF00,  // Yellow
    0xFF00FF,  // Magenta
    0x00FFFF   // Cyan
};

//====================================================
// Display Current Color
//====================================================
void showColor()
{
    matrix.fill(
        colors[palette]
    );

    matrix.show();
}

void setup()
{
    // Initialize RGB matrix.
    matrix.begin();

    matrix.setBrightness(25);

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

    // Show the first palette color.
    showColor();
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

    // Detect a strong negative Y-axis acceleration.
    //
    // A 500 ms lockout prevents one physical flick
    // from being detected multiple times.
    if (
        accel.acceleration.y < -14 &&
        millis() - lastGesture > 500
    )
    {
        // Move to the next color.
        palette++;

        // Return to the beginning after the sixth color.
        palette %= 6;

        // Display the new color.
        showColor();

        // Remember when the gesture occurred.
        lastGesture = millis();
    }

    delay(20);
}
