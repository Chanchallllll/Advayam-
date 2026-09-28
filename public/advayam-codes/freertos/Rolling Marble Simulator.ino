/*
 * =============================================================================
 * ROLLING MARBLE SIMULATION - FREERTOS VERSION
 * =============================================================================
 *
 * Simulates a marble moving across the 5x5 RGB matrix.
 *
 * Board tilt provides acceleration.
 * Simulated friction gradually slows the marble.
 * The marble bounces when it reaches an edge.
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
 * The complete simulation runs inside a FreeRTOS task.
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
// Marble State
//=============================================================================

float x = 2;
float y = 2;

float vx = 0;
float vy = 0;

//=============================================================================
// Matrix Coordinate Conversion
//=============================================================================

int pixelXY(int px, int py)
{
    if (py % 2 == 0)
        return py * 5 + px;

    return py * 5 + (4 - px);
}

//=============================================================================
// Marble Simulation Task
//=============================================================================

void marbleTask(void *parameter)
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

        //=====================================================================
        // Tilt supplies acceleration.
        //=====================================================================

        vx +=
            accel.acceleration.y * 0.002;

        vy -=
            accel.acceleration.x * 0.002;

        //=====================================================================
        // Simulated surface friction.
        //=====================================================================

        vx *= 0.97;
        vy *= 0.97;

        //=====================================================================
        // Update marble position.
        //=====================================================================

        x += vx;
        y += vy;

        //=====================================================================
        // Bounce from horizontal edges.
        //=====================================================================

        if (x < 0)
        {
            x = 0;
            vx = -vx;
        }

        if (x > 4)
        {
            x = 4;
            vx = -vx;
        }

        //=====================================================================
        // Bounce from vertical edges.
        //=====================================================================

        if (y < 0)
        {
            y = 0;
            vy = -vy;
        }

        if (y > 4)
        {
            y = 4;
            vy = -vy;
        }

        //=====================================================================
        // Draw marble.
        //=====================================================================

        matrix.clear();

        matrix.setPixelColor(
            pixelXY(
                round(x),
                round(y)
            ),
            matrix.Color(255, 180, 0)
        );

        matrix.show();

        // Run the simulation approximately every 30 ms.
        vTaskDelay(pdMS_TO_TICKS(30));
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

    // Create the marble simulation task.
    xTaskCreate(
        marbleTask,
        "MarbleTask",
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
