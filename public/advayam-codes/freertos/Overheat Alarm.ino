/*
 * =============================================================================
 * LMT87 TEMPERATURE MONITOR WITH OVERHEAT ALARM - FREERTOS VERSION
 * =============================================================================
 *
 * The LMT87 temperature sensor is connected to GPIO 1.
 *
 * A FreeRTOS task periodically reads the temperature and prints it to the
 * Serial Monitor.
 *
 * If the temperature reaches or exceeds 40 °C:
 *
 *   - Buzzer on GPIO 16 turns ON and OFF repeatedly.
 *   - User LED on GPIO 45 turns ON and OFF repeatedly.
 *
 * When the temperature is below the threshold, the alarm remains OFF.
 *
 * The original delay()-based timing has been replaced with FreeRTOS
 * vTaskDelay() calls so the task yields CPU time properly.
 * =============================================================================
 */

#include <Arduino.h>
#include <LMT87.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define TEMP_SENSOR_PIN  1
#define BUZZER_PIN       16
#define USER_LED         45

//=============================================================================
// Temperature Configuration
//=============================================================================

#define TEMP_THRESHOLD   40.0

// LMT87 ADC configuration.
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//=============================================================================
// Read Temperature
//=============================================================================

double readTemperatureC()
{
    // The minus sign is intentional.
    // It produces the correct positive temperature with the LMT87
    // library configuration used on this board.
    double temperatureC =
        -lmt87.getCDegree(
            TEMP_SENSOR_PIN,
            ADCres,
            voltageRef,
            100
        );

    return temperatureC;
}

//=============================================================================
// Temperature Monitoring Task
//=============================================================================

void temperatureTask(void *parameter)
{
    while (true)
    {
        // Read the current temperature.
        double temperatureC = readTemperatureC();

        // Display the measured temperature.
        Serial.print("Temperature: ");
        Serial.print(temperatureC, 1);
        Serial.println(" °C");

        //=====================================================================
        // Overheat condition
        //=====================================================================

        if (temperatureC >= TEMP_THRESHOLD)
        {
            Serial.println("WARNING: OVER TEMPERATURE!");

            // Turn both alarm indicators ON.
            digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(USER_LED, HIGH);

            // Keep the alarm active for 200 ms.
            vTaskDelay(pdMS_TO_TICKS(200));

            // Turn both alarm indicators OFF.
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(USER_LED, LOW);

            // Pause before the next alarm cycle.
            vTaskDelay(pdMS_TO_TICKS(200));
        }
        else
        {
            // Normal temperature: ensure the alarm is OFF.
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(USER_LED, LOW);

            // Temperature is checked once per second.
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(USER_LED, OUTPUT);

    // Make sure the alarm devices start OFF.
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(USER_LED, LOW);

    Serial.println("--------------------------------");
    Serial.println("Active Overheat Alarm");
    Serial.println("FreeRTOS Version");
    Serial.println("--------------------------------");

    // Create the temperature monitoring task.
    xTaskCreate(
        temperatureTask,
        "TemperatureTask",
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
    // Application work is handled by the FreeRTOS task.
    vTaskDelay(portMAX_DELAY);
}
