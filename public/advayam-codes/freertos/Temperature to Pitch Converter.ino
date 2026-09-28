/*
 * =============================================================================
 * TEMPERATURE-TO-PITCH CONVERTER - FREERTOS VERSION
 * =============================================================================
 *
 * The LMT87 temperature sensor controls the frequency of a buzzer.
 *
 * Higher temperature -> higher pitch.
 *
 * FreeRTOS structure:
 *
 *   TemperatureAudioTask
 *       - Reads the LMT87
 *       - Converts temperature to frequency
 *       - Generates the buzzer tone
 *       - Prints the current values
 *
 * The task updates the buzzer approximately every 250 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <LMT87.h>

//====================================================
// GPIO Configuration
//====================================================
#define TEMP_PIN    1
#define BUZZER_PIN 16

//====================================================
// LMT87 ADC Settings
//====================================================
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//====================================================
// Temperature-to-Frequency Range
//====================================================
const float MIN_TEMP = 20.0;
const float MAX_TEMP = 50.0;

const int MIN_FREQ = 200;
const int MAX_FREQ = 2000;

//====================================================
// Temperature / Audio Task
//====================================================
void TemperatureAudioTask(void *parameter)
{
    while (true)
    {
        // Read temperature using the working LMT87 conversion.
        double temperatureC =
            -lmt87.getCDegree(
                TEMP_PIN,
                ADCres,
                voltageRef,
                100
            );

        // Limit temperature to the defined mapping range.
        float limitedTemp =
            constrain(
                temperatureC,
                MIN_TEMP,
                MAX_TEMP
            );

        // Convert temperature into frequency.
        int frequency =
            map(
                (int)(limitedTemp * 10),
                MIN_TEMP * 10,
                MAX_TEMP * 10,
                MIN_FREQ,
                MAX_FREQ
            );

        // Generate the tone.
        tone(
            BUZZER_PIN,
            frequency
        );

        // Display the current values.
        Serial.print("Temperature: ");
        Serial.print(temperatureC, 1);
        Serial.print(" C | Frequency: ");
        Serial.print(frequency);
        Serial.println(" Hz");

        // Update the temperature-to-pitch conversion every 250 ms.
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

    Serial.println("Temperature-to-Pitch Converter");
    Serial.println("FreeRTOS Version");

    xTaskCreate(
        TemperatureAudioTask,
        "Temperature Audio",
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
