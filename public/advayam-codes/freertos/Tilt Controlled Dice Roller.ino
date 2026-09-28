/*
 * =============================================================================
 * VIRTUAL SIX-SIDED DIE - FREERTOS VERSION
 * =============================================================================
 *
 * Shaking or tilting the board starts a virtual die roll.
 *
 * While rolling:
 *   The NeoPixel matrix rapidly displays random values from 1 to 6.
 *
 * When the board becomes flat again:
 *   The final value is displayed.
 *
 * FreeRTOS structure:
 *
 *   DiceTask
 *       - Reads accelerometer data
 *       - Detects the start/end of a roll
 *       - Generates random die values
 *       - Updates the NeoPixel matrix
 *
 * The task waits 100 ms between rolling updates.
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
// Display Number
//====================================================
void showNumber(int n)
{
    matrix.clear();

    // Simple representation:
    // illuminate n pixels to represent the die value.
    for (int i = 0; i < n; i++)
    {
        matrix.setPixelColor(
            i,
            matrix.Color(255, 255, 255)
        );
    }

    matrix.show();
}

//====================================================
// Dice Task
//====================================================
void DiceTask(void *parameter)
{
    bool rolling = false;
    unsigned long rollStart = 0;

    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        imu.getEvent(&accel, &gyro, &temp);

        // Calculate horizontal acceleration magnitude.
        float tilt =
            sqrt(
                accel.acceleration.x *
                accel.acceleration.x +
                accel.acceleration.y *
                accel.acceleration.y
            );

        //================================================
        // Start Rolling
        //================================================
        if (tilt > 5 && !rolling)
        {
            rolling = true;
            rollStart = millis();

            Serial.println("Rolling...");
        }

        //================================================
        // Rolling State
        //================================================
        if (rolling)
        {
            // Display rapidly changing random values.
            showNumber(random(1, 7));

            // When the board becomes relatively flat and
            // at least 500 ms have elapsed, settle the die.
            if (
                tilt < 2 &&
                millis() - rollStart > 500
            )
            {
                int finalValue = random(1, 7);

                showNumber(finalValue);

                Serial.print("Result: ");
                Serial.println(finalValue);

                rolling = false;
            }

            // Update the rolling animation every 100 ms.
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        else
        {
            // When not rolling, check for movement more
            // frequently.
            vTaskDelay(pdMS_TO_TICKS(30));
        }
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

    // Seed the random-number generator using the
    // analog input originally used by the sketch.
    randomSeed(analogRead(1));

    xTaskCreate(
        DiceTask,
        "Dice Task",
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
