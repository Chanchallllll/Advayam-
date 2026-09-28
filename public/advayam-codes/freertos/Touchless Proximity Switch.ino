/*
 * =============================================================================
 * TOUCHLESS PROXIMITY SWITCH - FREERTOS VERSION
 * =============================================================================
 *
 * Wave a hand near the RPR-0521RS to toggle the onboard User LED.
 *
 * The RPR-0521RS does not directly report distance in centimeters, so
 * PROXIMITY_THRESHOLD must be calibrated experimentally.
 *
 * User LED:
 *   GPIO 45
 *
 * FreeRTOS structure:
 *
 *   ProximityTask
 *       - Reads proximity data
 *       - Detects when a hand enters the detection area
 *       - Toggles the User LED
 *
 * The task checks the sensor approximately every 50 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>

//====================================================
// GPIO Configuration
//====================================================
#define I2C_SDA 5
#define I2C_SCL 6

#define USER_LED 45

//====================================================
// Proximity Configuration
//====================================================
//
// Increase or decrease this value after testing the
// actual sensor response.
//
#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;

//====================================================
// Switch State
//====================================================
bool ledState = false;
bool handWasNear = false;

//====================================================
// Proximity Task
//====================================================
void ProximityTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            // Determine whether a hand is inside the
            // configured detection area.
            bool handIsNear =
                proximity > PROXIMITY_THRESHOLD;

            // Toggle only when the hand ENTERS the
            // detection area.
            //
            // This prevents repeated toggling while
            // the hand remains near the sensor.
            if (
                handIsNear &&
                !handWasNear
            )
            {
                ledState = !ledState;

                digitalWrite(
                    USER_LED,
                    ledState ? HIGH : LOW
                );

                Serial.print("LED: ");
                Serial.println(
                    ledState ? "ON" : "OFF"
                );
            }

            handWasNear = handIsNear;
        }

        // Check the proximity sensor every 50 ms.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    pinMode(USER_LED, OUTPUT);
    digitalWrite(USER_LED, LOW);

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

    Serial.println("Touchless Proximity Switch started.");
    Serial.println("FreeRTOS Version");

    xTaskCreate(
        ProximityTask,
        "Proximity Task",
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
