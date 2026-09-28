/*
 * This program monitors temperature using an LMT87 temperature sensor
 * connected to GPIO 1.
 *
 * The measured temperature is displayed on the Serial Monitor.
 * If the temperature reaches or exceeds 40 °C, an overheat alarm
 * is activated. The buzzer and LED connected to GPIO 16 and GPIO 45
 * will turn ON and OFF repeatedly to provide both audible and visual
 * warnings.
 *
 * When the temperature is below the threshold, the buzzer and LED
 * remain OFF and the temperature is checked once every second.
 */

#include <Arduino.h>
#include <LMT87.h>

//====================================================
// GPIO Configuration
//====================================================
#define TEMP_SENSOR_PIN  1
#define BUZZER_PIN       16
#define USER_LED         45

//====================================================
// Temperature Configuration
//====================================================
#define TEMP_THRESHOLD   40.0

// LMT87 ADC configuration
// ADCres defines the ADC resolution used to read the sensor.
// voltageRef is the reference voltage used for the ADC reading.
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//====================================================
// Read Temperature
//====================================================
double readTemperatureC()
{
    // Minus sign is intentional.
    // It produces the correct positive temperature
    // with the LMT87 library used on this board.
    double temperatureC =
        -lmt87.getCDegree(
            TEMP_SENSOR_PIN,
            ADCres,
            voltageRef,
            100
        );

    // Return the calculated temperature in degrees Celsius
    // so the main program can check it against the threshold.
    return temperatureC;
}

//====================================================
// Setup
//====================================================
void setup()
{
    // Start serial communication so the temperature and
    // alarm messages can be viewed in the Serial Monitor.
    Serial.begin(115200);

    pinMode(BUZZER_PIN, OUTPUT);
    pinMode(USER_LED, OUTPUT);

    // Make sure the alarm devices are OFF when the board starts.
    digitalWrite(BUZZER_PIN, LOW);
    digitalWrite(USER_LED, LOW);

    Serial.println("--------------------------------");
    Serial.println("Active Overheat Alarm");
    Serial.println("--------------------------------");
}

//====================================================
// Loop
//====================================================
void loop()
{
    // Read the current temperature from the LMT87 sensor.
    double temperatureC = readTemperatureC();

    // Display the measured temperature with one decimal place.
    Serial.print("Temperature: ");
    Serial.print(temperatureC, 1);
    Serial.println(" °C");

    //================================================
    // Overheating Alarm
    //================================================
    if (temperatureC >= TEMP_THRESHOLD)
    {
        // The temperature has reached or exceeded the
        // configured safety threshold, so activate the alarm.
        Serial.println("WARNING: OVER TEMPERATURE!");

        // Buzzer ON + LED ON
        // Both indicators are turned on to warn the user.
        digitalWrite(BUZZER_PIN, HIGH);
        digitalWrite(USER_LED, HIGH);

        // Keep the alarm ON for 200 ms.
        delay(200);

        // Buzzer OFF + LED OFF
        // Turning both devices off creates the repeating
        // ON/OFF alarm pattern.
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(USER_LED, LOW);

        // Keep the alarm OFF for another 200 ms before
        // checking the temperature again.
        delay(200);
    }
    else
    {
        // Normal temperature
        // No alarm is required, so make sure both indicators
        // remain turned off.
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(USER_LED, LOW);

        // Wait one second before taking the next temperature reading.
        delay(1000);
    }
}
