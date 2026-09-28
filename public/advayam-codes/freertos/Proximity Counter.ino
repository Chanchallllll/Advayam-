/*
 * =============================================================================
 * PROXIMITY COUNTER - FREERTOS VERSION
 * =============================================================================
 *
 * Counts how many times an object enters the configured proximity range.
 *
 * A new object is counted only after the previous object leaves the detection
 * area.
 *
 * The RPR-0521RS sensor is sampled periodically by a FreeRTOS task.
 *
 * UART:
 *   USB Serial at 115200 baud
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;

unsigned long objectCount = 0;
bool objectPresent = false;

//=============================================================================
// Proximity Counter Task
//=============================================================================

void proximityCounterTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            bool detected =
                proximity > PROXIMITY_THRESHOLD;

            // Count only the transition from NOT detected to detected.
            if (detected && !objectPresent)
            {
                objectCount++;

                Serial.print(
                    "Object detected! Total count: "
                );

                Serial.println(objectCount);
            }

            // Remember the current detection state.
            objectPresent = detected;
        }

        // Sample the sensor approximately every 50 ms.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    Wire.begin(I2C_SDA, I2C_SCL);

    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println("Proximity Counter started.");
    Serial.println("FreeRTOS Version");

    // Start the counter task.
    xTaskCreate(
        proximityCounterTask,
        "ProximityCounter",
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
    // The counter runs entirely inside the FreeRTOS task.
    vTaskDelay(portMAX_DELAY);
}
