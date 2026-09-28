/*
 * This program monitors the temperature using an LMT87 sensor
 * connected to GPIO 1 and keeps track of the temperature range
 * measured since the board was started.
 *
 * It continuously records:
 *   - The current temperature
 *   - The minimum temperature since startup
 *   - The maximum temperature since startup
 *
 * Whenever a new highest temperature is detected, the LED on
 * GPIO 45 gives one long blink.
 *
 * Whenever a new lowest temperature is detected, the LED gives
 * two short blinks.
 *
 * The minimum and maximum values are stored in memory and remain
 * available for as long as the board is running. They are reset
 * when the board is restarted.
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
// ADCres defines the ADC resolution used to read the sensor.
// voltageRef is the ADC reference voltage.
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//====================================================
// Read Temperature
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

    // Return the converted temperature so it can be
    // compared with the stored minimum and maximum values.
    return temperatureC;
}

//====================================================
// New Maximum Indication
//====================================================
void indicateMaximum()
{
    // One long blink
    // A longer blink tells the user that a new
    // highest temperature has been recorded.
    digitalWrite(USER_LED, HIGH);
    delay(300);

    digitalWrite(USER_LED, LOW);
}

//====================================================
// New Minimum Indication
//====================================================
void indicateMinimum()
{
    // Two short blinks
    // Two quick flashes tell the user that a new
    // lowest temperature has been recorded.
    for (int i = 0; i < 2; i++)
    {
        digitalWrite(USER_LED, HIGH);
        delay(100);

        digitalWrite(USER_LED, LOW);
        delay(100);
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    // Start serial communication so the current,
    // minimum, and maximum temperatures can be monitored.
    Serial.begin(115200);

    pinMode(USER_LED, OUTPUT);

    // Make sure the LED starts in the OFF state.
    digitalWrite(USER_LED, LOW);

    Serial.println("--------------------------------");
    Serial.println("LMT87 Temperature High/Low Latch");
    Serial.println("--------------------------------");
}

//====================================================
// Loop
//====================================================
void loop()
{
    // firstReading is used to identify the very first
    // temperature measurement after startup.
    static bool firstReading = true;

    // These variables remember the lowest and highest
    // temperatures measured during the current power cycle.
    static double minimumTemperature = 0.0;
    static double maximumTemperature = 0.0;

    //================================================
    // Read Temperature
    //================================================
    // Take a new temperature measurement from the LMT87.
    double temperatureC = readTemperatureC();

    //================================================
    // Initialize Min/Max
    //================================================
    if (firstReading)
    {
        // The first temperature reading becomes both
        // the initial minimum and maximum value.
        minimumTemperature = temperatureC;
        maximumTemperature = temperatureC;

        // From this point onward, new readings can be
        // compared against the stored limits.
        firstReading = false;

        Serial.println("Initial temperature recorded.");
    }

    //================================================
    // New Maximum
    //================================================
    else if (temperatureC > maximumTemperature)
    {
        // The current temperature is higher than every
        // previous reading, so update the maximum value.
        maximumTemperature = temperatureC;

        Serial.println("NEW HIGH TEMPERATURE!");

        // Give one long LED blink to indicate the new maximum.
        indicateMaximum();
    }

    //================================================
    // New Minimum
    //================================================
    else if (temperatureC < minimumTemperature)
    {
        // The current temperature is lower than every
        // previous reading, so update the minimum value.
        minimumTemperature = temperatureC;

        Serial.println("NEW LOW TEMPERATURE!");

        // Give two short LED blinks to indicate the new minimum.
        indicateMinimum();
    }

    //================================================
    // Display Temperature Data
    //================================================
    // Show all three temperature values in the Serial Monitor.
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

    // Read every 2 seconds
    // The next temperature measurement will be taken
    // after this delay.
    delay(2000);
}
