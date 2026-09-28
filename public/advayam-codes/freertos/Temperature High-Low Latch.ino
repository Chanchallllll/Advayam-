/*
 * =============================================================================
 * LMT87 TEMPERATURE HIGH/LOW LATCH - FREERTOS VERSION
 * =============================================================================
 *
 * This program monitors the temperature using an LMT87 sensor connected to
 * GPIO 1 and keeps track of the minimum and maximum temperatures measured
 * since startup.
 *
 * FreeRTOS structure:
 *
 *   TemperatureTask
 *       - Reads the LMT87
 *       - Updates minimum/maximum values
 *       - Prints temperature information
 *       - Requests LED indication for new highs/lows
 *
 *   LedTask
 *       - Handles LED indication without blocking the temperature task
 *
 * New maximum:
 *   One long LED blink.
 *
 * New minimum:
 *   Two short LED blinks.
 *
 * The minimum and maximum values are stored only while the board is running.
 * They are reset after a restart.
 * =============================================================================
 */

#include <Arduino.h>
#include <LMT87.h>

//====================================================
// GPIO Configuration
//====================================================
#define TEMP_SENSOR_PIN  1
#define USER_LED         45

//====================================================
// LMT87 Configuration
//====================================================
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//====================================================
// FreeRTOS Objects
//====================================================

// Used to notify the LED task that a new temperature
// maximum or minimum has been detected.
TaskHandle_t ledTaskHandle = NULL;

//====================================================
// Temperature Reading
//====================================================
double readTemperatureC()
{
    // Minus sign is intentional.
    // This is the working conversion for this board.
    double temperatureC =
        -lmt87.getCDegree(
            TEMP_SENSOR_PIN,
            ADCres,
            voltageRef,
            100
        );

    return temperatureC;
}

//====================================================
// LED Task
//====================================================
//
// The LED indication is handled by a separate FreeRTOS
// task so that LED blinking does not block temperature
// measurements.
//
void LedTask(void *parameter)
{
    while (true)
    {
        // Wait until TemperatureTask sends an event.
        uint32_t notification = ulTaskNotifyTake(
            pdTRUE,
            portMAX_DELAY
        );

        if (notification == 1)
        {
            // New maximum:
            // one long blink.
            digitalWrite(USER_LED, HIGH);
            vTaskDelay(pdMS_TO_TICKS(300));
            digitalWrite(USER_LED, LOW);
        }
        else if (notification == 2)
        {
            // New minimum:
            // two short blinks.
            for (int i = 0; i < 2; i++)
            {
                digitalWrite(USER_LED, HIGH);
                vTaskDelay(pdMS_TO_TICKS(100));

                digitalWrite(USER_LED, LOW);
                vTaskDelay(pdMS_TO_TICKS(100));
            }
        }
    }
}

//====================================================
// Temperature Task
//====================================================
//
// Runs once every 2 seconds.
//
void TemperatureTask(void *parameter)
{
    bool firstReading = true;

    double minimumTemperature = 0.0;
    double maximumTemperature = 0.0;

    while (true)
    {
        //================================================
        // Read Temperature
        //================================================
        double temperatureC = readTemperatureC();

        //================================================
        // Initialize Min/Max
        //================================================
        if (firstReading)
        {
            minimumTemperature = temperatureC;
            maximumTemperature = temperatureC;

            firstReading = false;

            Serial.println("Initial temperature recorded.");
        }

        //================================================
        // New Maximum
        //================================================
        else if (temperatureC > maximumTemperature)
        {
            maximumTemperature = temperatureC;

            Serial.println("NEW HIGH TEMPERATURE!");

            // Tell the LED task to perform one long blink.
            xTaskNotifyGive(ledTaskHandle);
        }

        //================================================
        // New Minimum
        //================================================
        else if (temperatureC < minimumTemperature)
        {
            minimumTemperature = temperatureC;

            Serial.println("NEW LOW TEMPERATURE!");

            // Send notification value 2 to the LED task.
            xTaskNotify(
                ledTaskHandle,
                2,
                eSetValueWithOverwrite
            );
        }

        //================================================
        // Display Temperature Data
        //================================================
        Serial.print("Current : ");
        Serial.print(temperatureC, 1);
        Serial.println(" °C");

        Serial.print("Minimum : ");
        Serial.print(minimumTemperature, 1);
        Serial.println(" °C");

        Serial.print("Maximum : ");
        Serial.print(maximumTemperature, 1);
        Serial.println(" °C");

        Serial.println();

        // Wait approximately 2 seconds before the next
        // temperature measurement.
        vTaskDelay(pdMS_TO_TICKS(2000));
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

    Serial.println("--------------------------------");
    Serial.println("LMT87 Temperature High/Low Latch");
    Serial.println("FreeRTOS Version");
    Serial.println("--------------------------------");

    // Create the LED task first so its handle is available
    // when the temperature task starts sending notifications.
    xTaskCreate(
        LedTask,
        "LED Task",
        2048,
        NULL,
        1,
        &ledTaskHandle
    );

    // Create the temperature monitoring task.
    xTaskCreate(
        TemperatureTask,
        "Temperature Task",
        4096,
        NULL,
        2,
        NULL
    );
}

//====================================================
// Arduino Loop
//====================================================
//
// Application work is performed by FreeRTOS tasks.
//
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
