/*
 * ============================================================================
 * DIGITAL BUBBLE LEVEL - FREERTOS VERSION
 * ============================================================================
 *
 * Uses the accelerometer to create a digital bubble level.
 *
 * The green centre LED represents the bubble. The bubble moves according
 * to board tilt and remains near the centre when the board is level.
 *
 * The sensor reading and LED display run inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *
 *   Matrix:
 *     GPIO 7
 *
 *   LSM6DSOX SPI:
 *     CS   -> GPIO 10
 *     SCK  -> GPIO 12
 *     MOSI -> GPIO 11
 *     MISO -> GPIO 13
 * ============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

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

//=============================================================================
// Convert x/y coordinates to the serpentine LED index.
//=============================================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

//=============================================================================
// Bubble level FreeRTOS task.
//=============================================================================

void bubbleLevelTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        imu.getEvent(
            &accel,
            &gyro,
            &temp
        );

        // Convert acceleration into an approximate tilt position.
        int x = constrain(
            2 - round(accel.acceleration.y / 2.5),
            0,
            4
        );

        int y = constrain(
            2 + round(accel.acceleration.x / 2.5),
            0,
            4
        );

        matrix.clear();

        matrix.setPixelColor(
            pixelXY(x, y),
            matrix.Color(0, 255, 0)
        );

        matrix.show();

        // Update approximately 20 times per second.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//=============================================================================
// Setup.
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(16, OUTPUT);
    digitalWrite(16, LOW);

    matrix.begin();
    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();

    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println("IMU initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    imu.setAccelRange(
        LSM6DS_ACCEL_RANGE_4_G
    );

    imu.setAccelDataRate(
        LSM6DS_RATE_104_HZ
    );

    Serial.println("Digital Bubble Level");

    // Create the bubble-level task.
    xTaskCreate(
        bubbleLevelTask,
        "Bubble Level Task",
        4096,
        NULL,
        1,
        NULL
    );
}

//=============================================================================
// Arduino loop.
//
// The application runs inside the FreeRTOS task.
//=============================================================================

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
