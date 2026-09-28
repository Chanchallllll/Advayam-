/*
 * =============================================================================
 * TILT-CONTROLLED SAND SIMULATION - FREERTOS VERSION
 * =============================================================================
 *
 * Simulates individual sand grains on the 5x5 RGB matrix.
 *
 * Board tilt determines the direction in which the sand moves.
 *
 * IMU:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * Matrix:
 *   GPIO 7
 *
 * The sand simulation and IMU processing run inside a FreeRTOS task.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

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
// Sand State
//=============================================================================

bool sand[5][5];

//=============================================================================
// Matrix Coordinate Conversion
//=============================================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

//=============================================================================
// Sand Simulation Task
//=============================================================================

void sandTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read accelerometer data.
        imu.getEvent(
            &accel,
            &gyro,
            &temp
        );

        int dx = 0;
        int dy = 0;

        // Determine vertical sand movement from X-axis tilt.
        if (fabs(accel.acceleration.x) > 2)
        {
            dy =
                accel.acceleration.x > 0
                    ? -1
                    : 1;
        }

        // Determine horizontal sand movement from Y-axis tilt.
        if (fabs(accel.acceleration.y) > 2)
        {
            dx =
                accel.acceleration.y > 0
                    ? 1
                    : -1;
        }

        // Temporary array stores the next sand configuration.
        bool next[5][5] = {};

        //=====================================================================
        // Move each grain in the direction of the tilt.
        //=====================================================================

        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                if (!sand[y][x])
                    continue;

                int nx =
                    constrain(
                        x + dx,
                        0,
                        4
                    );

                int ny =
                    constrain(
                        y + dy,
                        0,
                        4
                    );

                // If the destination is free, move the grain.
                if (!next[ny][nx])
                {
                    next[ny][nx] = true;
                }
                else
                {
                    // Otherwise leave the grain at its current position.
                    next[y][x] = true;
                }
            }
        }

        // Replace the current sand state.
        memcpy(
            sand,
            next,
            sizeof(sand)
        );

        //=====================================================================
        // Draw the sand.
        //=====================================================================

        matrix.clear();

        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                if (sand[y][x])
                {
                    matrix.setPixelColor(
                        pixelXY(x, y),
                        matrix.Color(
                            255,
                            180,
                            30
                        )
                    );
                }
            }
        }

        matrix.show();

        // Update the simulation approximately every 100 ms.
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
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

    // Seed random number generation.
    randomSeed(
        (uint32_t)esp_random()
    );

    //=====================================================================
    // Generate the initial random pile.
    //=====================================================================

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            sand[y][x] =
                random(100) < 45;
        }
    }

    Serial.begin(115200);
    Serial.println("Tilt-controlled sand simulation started.");
    Serial.println("FreeRTOS Version");

    // Start the simulation task.
    xTaskCreate(
        sandTask,
        "SandTask",
        4096,
        NULL,
        1,
        NULL
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    vTaskDelay(portMAX_DELAY);
}
