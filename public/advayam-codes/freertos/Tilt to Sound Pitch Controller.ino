/*
 * =============================================================================
 * TILT-TO-SOUND CONTROLLER - FREERTOS VERSION
 * =============================================================================
 *
 * Board pitch controls buzzer frequency.
 *
 * Board roll controls buzzer duty cycle.
 *
 * Pitch:
 *   -45 degrees -> approximately 200 Hz
 *    +45 degrees -> approximately 2000 Hz
 *
 * Roll:
 *   0 degrees  -> maximum duty cycle
 *   45 degrees -> reduced duty cycle
 *
 * FreeRTOS structure:
 *
 *   TiltAudioTask
 *       - Reads accelerometer data
 *       - Calculates pitch and roll
 *       - Converts them to PWM parameters
 *       - Updates the buzzer
 *
 * The task updates the audio approximately every 20 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

//====================================================
// GPIO Configuration
//====================================================
#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// PWM Configuration
//====================================================
#define BUZZER_CHANNEL 0

Adafruit_LSM6DSOX imu;

//====================================================
// Tilt Audio Task
//====================================================
void TiltAudioTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read accelerometer data.
        imu.getEvent(&accel, &gyro, &temp);

        //================================================
        // Calculate Pitch
        //================================================
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

        //================================================
        // Calculate Roll
        //================================================
        float roll =
            atan2(
                -accel.acceleration.y,
                accel.acceleration.z
            ) * 180.0 / PI;

        //================================================
        // Convert Pitch to Frequency
        //================================================
        int frequency =
            map(
                constrain((int)pitch, -45, 45),
                -45,
                45,
                200,
                2000
            );

        //================================================
        // Convert Roll to Duty Cycle
        //================================================
        int duty =
            map(
                constrain((int)fabs(roll), 0, 45),
                0,
                45,
                255,
                40
            );

        //================================================
        // Generate PWM Audio
        //================================================
        ledcWriteTone(
            BUZZER_CHANNEL,
            frequency
        );

        ledcWrite(
            BUZZER_CHANNEL,
            duty
        );

        // Update approximately every 20 ms.
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

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

    // ESP32 LEDC PWM configuration.
    ledcSetup(
        BUZZER_CHANNEL,
        1000,
        8
    );

    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    xTaskCreate(
        TiltAudioTask,
        "Tilt Audio",
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
