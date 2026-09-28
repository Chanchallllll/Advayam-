/*
 * =============================================================================
 * IMU ROLL / PITCH MONITOR - FREERTOS VERSION
 * =============================================================================
 *
 * Calculates board roll and pitch from the accelerometer.
 *
 * Output format:
 *
 *     time_ms,roll,pitch
 *
 * The IMU is read periodically by a dedicated FreeRTOS task.
 *
 * IMU:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//=============================================================================
// IMU Pins
//=============================================================================

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

//=============================================================================
// IMU Monitoring Task
//=============================================================================

void imuTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read the latest IMU data.
        imu.getEvent(
            &accel,
            &gyro,
            &temp
        );

        // Calculate roll from Y and Z acceleration.
        float roll =
            atan2(
                accel.acceleration.y,
                accel.acceleration.z
            )
            * 180.0 / PI;

        // Calculate pitch from X, Y and Z acceleration.
        float pitch =
            atan2(
                -accel.acceleration.x,
                sqrt(
                    accel.acceleration.y *
                    accel.acceleration.y +
                    accel.acceleration.z *
                    accel.acceleration.z
                )
            )
            * 180.0 / PI;

        // Output CSV data for real-time plotting.
        Serial.print(millis());
        Serial.print(",");
        Serial.print(roll, 2);
        Serial.print(",");
        Serial.println(pitch, 2);

        // Sample approximately every 50 ms.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

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

    // Configure accelerometer.
    imu.setAccelRange(
        LSM6DS_ACCEL_RANGE_4_G
    );

    imu.setAccelDataRate(
        LSM6DS_RATE_104_HZ
    );

    Serial.println("time_ms,roll,pitch");

    // Create the IMU monitoring task.
    xTaskCreate(
        imuTask,
        "IMUTask",
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
