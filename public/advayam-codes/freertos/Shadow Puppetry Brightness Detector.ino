/*
 * =============================================================================
 * SHADOW PUPPETRY BRIGHTNESS DETECTOR - FREERTOS VERSION
 * =============================================================================
 *
 * Detects rapid changes in ambient brightness using the RPR-0521RS sensor.
 *
 * A sufficiently large reduction in light is interpreted as a shadow:
 *
 *     Light decreases -> low sound
 *
 * A sufficiently large increase in light means the shadow has disappeared:
 *
 *     Light increases -> high sound
 *
 * Buzzer:
 *   GPIO 16
 *
 * The brightness sensor is sampled by a FreeRTOS task.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define BUZZER_PIN 16

RPR0521RS sensor;

float previousLux = 0;

//=============================================================================
// Shadow Detection Task
//=============================================================================

void shadowTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            float change =
                lux - previousLux;

            // A sufficiently large negative change indicates a shadow.
            if (change < -20)
            {
                tone(
                    BUZZER_PIN,
                    300,
                    150
                );

                Serial.println(
                    "Shadow detected!"
                );
            }

            // A sufficiently large positive change indicates that
            // the shadow has disappeared.
            else if (change > 20)
            {
                tone(
                    BUZZER_PIN,
                    1200,
                    150
                );

                Serial.println(
                    "Light restored!"
                );
            }

            // Store this reading for comparison with the next reading.
            previousLux = lux;
        }

        // Sample ambient light approximately every 80 ms.
        vTaskDelay(pdMS_TO_TICKS(80));
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);

    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    if (sensor.init() != 0)
    {
        Serial.println(
            "Sensor initialization failed!"
        );

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println(
        "Shadow Puppetry detector started."
    );

    Serial.println(
        "FreeRTOS Version"
    );

    // Start the brightness monitoring task.
    xTaskCreate(
        shadowTask,
        "ShadowTask",
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
