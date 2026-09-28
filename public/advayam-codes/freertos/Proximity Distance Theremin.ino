/*
 * =============================================================================
 * PROXIMITY DISTANCE THEREMIN - FREERTOS VERSION
 * =============================================================================
 *
 * Uses the RPR-0521RS proximity reading to control the pitch of a buzzer.
 *
 * Hand closer  -> higher pitch
 * Hand farther  -> lower pitch
 *
 * Buzzer:
 *   GPIO 16
 *
 * A FreeRTOS task continuously reads the proximity sensor and updates the
 * buzzer frequency.
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

//=============================================================================
// Theremin Configuration
//=============================================================================

#define MIN_PROXIMITY 50
#define MAX_PROXIMITY 2000

#define MIN_FREQUENCY 150
#define MAX_FREQUENCY 2500

//=============================================================================
// Proximity Theremin Task
//=============================================================================

void thereminTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            // Limit the sensor reading to the configured range.
            int p =
                constrain(
                    proximity,
                    MIN_PROXIMITY,
                    MAX_PROXIMITY
                );

            // Convert proximity to an audible frequency.
            int frequency =
                map(
                    p,
                    MIN_PROXIMITY,
                    MAX_PROXIMITY,
                    MIN_FREQUENCY,
                    MAX_FREQUENCY
                );

            // Only produce sound once the proximity value is
            // above the minimum usable range.
            if (proximity > MIN_PROXIMITY)
            {
                tone(BUZZER_PIN, frequency);
            }
            else
            {
                noTone(BUZZER_PIN);
            }

            Serial.print("Proximity: ");
            Serial.print(proximity);
            Serial.print(" | Frequency: ");
            Serial.println(frequency);
        }

        // Update the pitch approximately every 30 ms.
        vTaskDelay(pdMS_TO_TICKS(30));
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

    Wire.begin(I2C_SDA, I2C_SCL);

    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    Serial.println("Proximity Theremin started.");
    Serial.println("FreeRTOS Version");

    xTaskCreate(
        thereminTask,
        "ThereminTask",
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
