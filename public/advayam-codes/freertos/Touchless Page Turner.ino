/*
 * =============================================================================
 * TOUCHLESS PAGE TURNER - FREERTOS VERSION
 * =============================================================================
 *
 * Generates UART commands for controlling presentation slides.
 *
 * IMPORTANT:
 * The single RPR-0521RS sensor cannot determine actual left/right movement.
 * This implementation interprets rapid proximity changes as gestures:
 *
 *   Rapid approach -> NEXT
 *   Rapid retreat  -> PREV
 *
 * Example UART commands:
 *   NEXT
 *   PREV
 *
 * A PC-side program can translate these commands into keyboard actions.
 *
 * FreeRTOS structure:
 *
 *   GestureTask
 *       - Reads proximity data from the RPR-0521RS
 *       - Detects rapid changes
 *       - Generates NEXT/PREV commands
 *
 * The sensor is checked approximately every 40 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>

//====================================================
// I2C Configuration
//====================================================
#define I2C_SDA 5
#define I2C_SCL 6

//====================================================
// Gesture Configuration
//====================================================
#define GESTURE_CHANGE  300
#define GESTURE_TIMEOUT 500

RPR0521RS sensor;

//====================================================
// Gesture State
//====================================================
unsigned short previousProximity = 0;
unsigned long lastGesture = 0;

//====================================================
// Gesture Task
//====================================================
void GestureTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        // Read proximity and ambient-light data.
        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            int change =
                (int)proximity -
                (int)previousProximity;

            // Prevent multiple gestures from being
            // generated too quickly.
            if (
                millis() - lastGesture >
                GESTURE_TIMEOUT
            )
            {
                // Large positive change:
                // hand is rapidly approaching.
                if (change > GESTURE_CHANGE)
                {
                    Serial.println("NEXT");

                    lastGesture = millis();
                }

                // Large negative change:
                // hand is rapidly moving away.
                else if (change < -GESTURE_CHANGE)
                {
                    Serial.println("PREV");

                    lastGesture = millis();
                }
            }

            previousProximity = proximity;
        }

        // Check the sensor every 40 ms.
        vTaskDelay(pdMS_TO_TICKS(40));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    // Initialize I2C.
    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    // Initialize the RPR-0521RS.
    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println("Touchless Page Turner ready.");
    Serial.println("FreeRTOS Version");

    xTaskCreate(
        GestureTask,
        "Gesture Task",
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
