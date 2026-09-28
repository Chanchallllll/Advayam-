/*
 * =============================================================================
 * BOARD LEVEL CHALLENGE - FREERTOS VERSION
 * =============================================================================
 *
 * The player must keep the board level for 15 seconds.
 *
 * LED behavior:
 *
 *   Green center pixel -> board is stable
 *   Red center pixel   -> board is tilted
 *   Full green matrix  -> challenge completed
 *
 * FreeRTOS structure:
 *
 *   LevelTask
 *       - Reads the accelerometer
 *       - Calculates roll and pitch
 *       - Determines whether the board is stable
 *       - Updates the NeoPixel matrix
 *       - Tracks the 15-second challenge timer
 *
 * The task checks the board approximately every 30 ms.
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
#define NUM_LEDS   25

//====================================================
// IMU Configuration
//====================================================
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

//====================================================
// Level Task
//====================================================
void LevelTask(void *parameter)
{
    unsigned long stableStart = 0;
    bool timing = false;

    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read accelerometer data.
        imu.getEvent(&accel, &gyro, &temp);

        // Calculate roll angle.
        float roll =
            atan2(
                -accel.acceleration.y,
                accel.acceleration.z
            ) * 180.0 / PI;

        // Calculate pitch angle.
        float pitch =
            atan2(
                accel.acceleration.x,
                sqrt(
                    accel.acceleration.y *
                    accel.acceleration.y +
                    accel.acceleration.z *
                    accel.acceleration.z
                )
            ) * 180.0 / PI;

        // Board is considered level when both angles
        // remain within +/-3 degrees.
        bool stable =
            fabs(roll) < 3.0 &&
            fabs(pitch) < 3.0;

        //================================================
        // Stable State
        //================================================
        if (stable)
        {
            if (!timing)
            {
                // Start the 15-second stability timer.
                stableStart = millis();
                timing = true;
            }

            // Green center pixel indicates stable position.
            matrix.setPixelColor(
                12,
                matrix.Color(0, 255, 0)
            );

            // Check whether the board has remained stable
            // for 15 seconds.
            if (millis() - stableStart >= 15000)
            {
                // Challenge completed.
                matrix.fill(
                    matrix.Color(0, 255, 0)
                );

                matrix.show();

                Serial.println("SUCCESS!");

                // Keep the success indication visible for
                // one second without blocking the Arduino
                // loop or another task.
                vTaskDelay(pdMS_TO_TICKS(1000));

                timing = false;
            }
        }

        //================================================
        // Unstable State
        //================================================
        else
        {
            // Any tilt resets the stability timer.
            timing = false;

            matrix.clear();

            // Red center pixel indicates instability.
            matrix.setPixelColor(
                12,
                matrix.Color(255, 0, 0)
            );
        }

        matrix.show();

        // Run the level check approximately every 30 ms.
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    matrix.begin();
    matrix.setBrightness(25);

    pinMode(16, OUTPUT);
    digitalWrite(16, LOW);

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

    imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

    xTaskCreate(
        LevelTask,
        "Level Challenge",
        4096,
        NULL,
        2,
        NULL
    );
}

//====================================================
// Loop
//====================================================
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
