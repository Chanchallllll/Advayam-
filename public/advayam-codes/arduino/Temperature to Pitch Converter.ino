/*
 * This program reads the LMT87 temperature sensor on GPIO 1
 * and maps the measured temperature to a buzzer frequency
 * on GPIO 16.
 *
 * Higher temperatures produce higher pitch.
 */

#include <Arduino.h>
#include <LMT87.h>

#define TEMP_PIN    1
#define BUZZER_PIN  16

// LMT87 ADC settings
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

// Temperature-to-frequency range
const float MIN_TEMP = 20.0;
const float MAX_TEMP = 50.0;

const int MIN_FREQ = 200;
const int MAX_FREQ = 2000;

void setup()
{
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);

    Serial.println("Temperature-to-Pitch Converter");
}

void loop()
{
    // Read temperature using the working LMT87 conversion
    double temperatureC =
        -lmt87.getCDegree(
            TEMP_PIN,
            ADCres,
            voltageRef,
            100
        );

    // Limit temperature to the mapping range
    float limitedTemp =
        constrain(
            temperatureC,
            MIN_TEMP,
            MAX_TEMP
        );

    // Convert temperature to frequency
    int frequency =
        map(
            (int)(limitedTemp * 10),
            MIN_TEMP * 10,
            MAX_TEMP * 10,
            MIN_FREQ,
            MAX_FREQ
        );

    // Generate the tone
    tone(
        BUZZER_PIN,
        frequency
    );

    // Show values
    Serial.print("Temperature: ");
    Serial.print(temperatureC, 1);
    Serial.print(" C | Frequency: ");
    Serial.print(frequency);
    Serial.println(" Hz");

    delay(250);
}
