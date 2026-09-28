/*
 * =============================================================================
 * BLIND SPOT PROXIMITY WARNING - FreeRTOS VERSION
 * =============================================================================
 *
 * Provides an audible warning when an object approaches the RPR-0521RS.
 *
 * Farther obstacle -> slow intermittent beep
 * Closer obstacle  -> faster beep
 *
 * Buzzer:
 *   GPIO 16
 *
 * FreeRTOS:
 *   A dedicated task reads the proximity sensor and controls the buzzer.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define I2C_SDA 5
#define I2C_SCL 6

#define BUZZER_PIN 16

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define PROXIMITY_TASK_STACK_SIZE 4096
#define PROXIMITY_TASK_PRIORITY   1

#define SENSOR_INTERVAL 20

//=============================================================================
// Hardware
//=============================================================================

RPR0521RS sensor;

//=============================================================================
// Buzzer State
//=============================================================================

unsigned long lastBeep = 0;
bool buzzerOn = false;

//=============================================================================
// Proximity Task
//=============================================================================

void proximityTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        // Read the proximity and ambient-light sensor.
        if (sensor.get_psalsval(
                &proximity,
                &lux
            ) == 0)
        {
            if (proximity > 100)
            {
                // Map proximity to beep interval.
                //
                // High proximity:
                //     Short interval
                //     Faster beeping
                //
                // Lower proximity:
                //     Longer interval
                //     Slower beeping

                int interval = map(
                    constrain(
                        proximity,
                        100,
                        2000
                    ),
                    100,
                    2000,
                    1000,
                    100
                );

                if (
                    millis() - lastBeep >=
                    (unsigned long)interval
                )
                {
                    lastBeep = millis();

                    if (!buzzerOn)
                    {
                        tone(
                            BUZZER_PIN,
                            1800
                        );

                        buzzerOn = true;
                    }
                    else
                    {
                        noTone(
                            BUZZER_PIN
                        );

                        buzzerOn = false;
                    }
                }
            }
            else
            {
                // No nearby object.
                noTone(BUZZER_PIN);
                buzzerOn = false;
            }
        }

        // Check the sensor again after 20 ms.
        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_INTERVAL
            )
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
    // Initialize I2C
    //-------------------------------------------------------------------------

    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    //-------------------------------------------------------------------------
    // Initialize Sensor
    //-------------------------------------------------------------------------

    if (sensor.init() != 0)
    {
        Serial.println(
            "Sensor initialization failed!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    Serial.println(
        "Blind Spot Proximity Warning Ready."
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Proximity Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        proximityTask,
        "Proximity Task",
        PROXIMITY_TASK_STACK_SIZE,
        NULL,
        PROXIMITY_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
