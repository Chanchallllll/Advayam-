/*
 * =============================================================================
 * ANTI-THEFT ALARM - FreeRTOS VERSION
 * =============================================================================
 *
 * This program arms an anti-theft alarm using the push button.
 *
 * Once armed, movement causes an escalating multi-tone siren.
 *
 * FreeRTOS:
 *   The alarm monitoring and siren generation run inside a dedicated task.
 *
 * Button:
 *   GPIO 17
 *
 * Buzzer:
 *   GPIO 16
 *
 * IMU:
 *   LSM6DSOX over SPI
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

//=============================================================================
// Buzzer Configuration
//=============================================================================

#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define ALARM_TASK_STACK_SIZE 4096
#define ALARM_TASK_PRIORITY   2

#define ALARM_CHECK_INTERVAL 30

//=============================================================================
// Hardware
//=============================================================================

Adafruit_LSM6DSOX imu;

//=============================================================================
// Alarm State
//=============================================================================

bool armed = false;

// Expected acceleration magnitude when stationary.
float baseline = 9.81;

//=============================================================================
// Alarm Task
//=============================================================================

void alarmTask(void *parameter)
{
    while (true)
    {
        //-------------------------------------------------------------------------
        // Check Arm Button
        //-------------------------------------------------------------------------

        if (digitalRead(BUTTON_PIN) == LOW)
        {
            armed = true;

            Serial.println("ARMED");

            // Prevent repeated arming messages from a held button.
            vTaskDelay(pdMS_TO_TICKS(500));
        }

        //-------------------------------------------------------------------------
        // Read IMU
        //-------------------------------------------------------------------------

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
            accel.acceleration.x *
            accel.acceleration.x +

            accel.acceleration.y *
            accel.acceleration.y +

            accel.acceleration.z *
            accel.acceleration.z
        );

        //-------------------------------------------------------------------------
        // Detect Movement
        //-------------------------------------------------------------------------

        if (armed &&
            fabs(magnitude - baseline) > 1.5)
        {
            Serial.println(
                "MOVEMENT DETECTED - ALARM!"
            );

            // Generate the escalating multi-tone siren.
            for (
                int frequency = 600;
                frequency <= 2400;
                frequency += 300
            )
            {
                ledcSetup(
                    BUZZER_CHANNEL,
                    frequency,
                    BUZZER_RESOLUTION
                );

                ledcWrite(
                    BUZZER_CHANNEL,
                    128
                );

                // Allow FreeRTOS to schedule other tasks
                // during each siren step.
                vTaskDelay(
                    pdMS_TO_TICKS(100)
                );
            }

            // Silence the buzzer after the siren sequence.
            ledcWrite(
                BUZZER_CHANNEL,
                0
            );
        }

        // Check the system again after 30 ms.
        vTaskDelay(
            pdMS_TO_TICKS(ALARM_CHECK_INTERVAL)
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    //-------------------------------------------------------------------------
    // GPIO
    //-------------------------------------------------------------------------

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    //-------------------------------------------------------------------------
    // Buzzer PWM
    //-------------------------------------------------------------------------

    ledcSetup(
        BUZZER_CHANNEL,
        1000,
        BUZZER_RESOLUTION
    );

    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    ledcWrite(
        BUZZER_CHANNEL,
        0
    );

    //-------------------------------------------------------------------------
    // SPI
    //-------------------------------------------------------------------------

    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    //-------------------------------------------------------------------------
    // IMU
    //-------------------------------------------------------------------------

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println(
            "IMU initialization failed!"
        );

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

    Serial.println(
        "Anti-Theft Alarm Ready"
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Alarm Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        alarmTask,
        "Anti Theft Task",
        ALARM_TASK_STACK_SIZE,
        NULL,
        ALARM_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    // The anti-theft system runs entirely inside alarmTask().
    vTaskDelay(pdMS_TO_TICKS(1000));
}
