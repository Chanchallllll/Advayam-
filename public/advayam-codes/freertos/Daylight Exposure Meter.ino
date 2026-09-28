/*
 * =============================================================================
 * CODE 4 - DAYLIGHT EXPOSURE METER
 * =============================================================================
 *
 * Tracks the cumulative amount of time during which ambient light is at or
 * above BRIGHT_LUX_THRESHOLD.
 *
 * Sensor:
 *   RPR-0521RS
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * IMPORTANT:
 * This is an electronics demonstration.
 * It is NOT a medical or UV exposure measurement device.
 *
 * The RPR-0521RS measures ambient/visible light and does not measure UV.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>

//====================================================
// I2C Configuration
//====================================================

#define I2C_SDA 5
#define I2C_SCL 6

//====================================================
// Light Threshold
//====================================================

#define BRIGHT_LUX_THRESHOLD 300

//====================================================
// Sensor
//====================================================

RPR0521RS sensor;

//====================================================
// Exposure Timing
//====================================================

unsigned long lastUpdate = 0;
unsigned long brightMilliseconds = 0;

//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(115200);

    // Start I2C using the specified ESP32 pins.
    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    Serial.println("Initializing RPR-0521RS...");

    // Initialize sensor.
    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        // Stop here if initialization failed.
        while (1)
        {
            delay(1000);
        }
    }

    lastUpdate = millis();

    Serial.println("Daylight Exposure Meter started.");
    Serial.print("Bright-light threshold: ");
    Serial.print(BRIGHT_LUX_THRESHOLD);
    Serial.println(" lux");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    unsigned short proximity;
    float lux;

    // Current time.
    unsigned long now = millis();

    // Calculate time since previous measurement.
    unsigned long elapsed =
        now - lastUpdate;

    lastUpdate = now;

    // Read proximity and ambient light.
    if (sensor.get_psalsval(
            &proximity,
            &lux
        ) == 0)
    {
        // Only accumulate exposure when the measured
        // ambient light is above the configured threshold.
        if (lux >= BRIGHT_LUX_THRESHOLD)
        {
            brightMilliseconds += elapsed;
        }

        // Convert milliseconds to seconds.
        unsigned long totalSeconds =
            brightMilliseconds / 1000;

        // Convert seconds to minutes.
        unsigned long minutes =
            totalSeconds / 60;

        // Remaining seconds.
        unsigned long seconds =
            totalSeconds % 60;

        Serial.print("Lux: ");
        Serial.print(lux, 1);

        Serial.print(" | Bright exposure: ");
        Serial.print(minutes);

        Serial.print(" min ");

        Serial.print(seconds);

        Serial.println(" sec");
    }
    else
    {
        Serial.println("Sensor reading failed.");
    }

    // Take another measurement after 1 second.
    delay(1000);
}
