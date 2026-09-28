/*
 * =============================================================================
 * AMBIENT LUX DATA LOGGER - FreeRTOS VERSION
 * =============================================================================
 *
 * Samples ambient light every minute and stores the readings in the ESP32-S3
 * internal flash filesystem using LittleFS.
 *
 * FreeRTOS:
 *   A dedicated task performs the periodic sensor reading and logging.
 *
 * Log file:
 *   /lux_log.csv
 *
 * NOTE:
 * The ESP32 does not have a battery-backed real-time clock in this setup.
 * Therefore the timestamp below is elapsed time since the logger task started.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <LittleFS.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define I2C_SDA 5
#define I2C_SCL 6

//=============================================================================
// Logging Configuration
//=============================================================================

#define LOG_INTERVAL 60000UL

// FreeRTOS task configuration
#define LOGGER_TASK_STACK_SIZE 4096
#define LOGGER_TASK_PRIORITY   1

//=============================================================================
// Sensor
//=============================================================================

RPR0521RS sensor;

//=============================================================================
// Logger Task
//=============================================================================

void loggerTask(void *parameter)
{
    unsigned long elapsedMinutes = 0;

    // Keep the task running continuously.
    while (true)
    {
        unsigned short proximity;
        float lux;

        // Wait one minute before taking the next reading.
        vTaskDelay(pdMS_TO_TICKS(LOG_INTERVAL));

        elapsedMinutes++;

        // Read ambient light and proximity.
        if (sensor.get_psalsval(&proximity, &lux) == 0)
        {
            // Open the CSV file in append mode.
            File file = LittleFS.open(
                "/lux_log.csv",
                FILE_APPEND
            );

            if (file)
            {
                file.print(elapsedMinutes);
                file.print(",");
                file.println(lux, 2);
                file.close();

                Serial.print("Logged: ");
                Serial.print(elapsedMinutes);
                Serial.print(" min, ");
                Serial.print(lux, 2);
                Serial.println(" lux");
            }
            else
            {
                Serial.println("Failed to open log file.");
            }
        }
        else
        {
            Serial.println("Failed to read RPR-0521RS.");
        }
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

    Wire.begin(I2C_SDA, I2C_SCL);

    //-------------------------------------------------------------------------
    // Initialize RPR-0521RS
    //-------------------------------------------------------------------------

    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        // Keep the Arduino task alive while reporting the error.
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    //-------------------------------------------------------------------------
    // Initialize LittleFS
    //-------------------------------------------------------------------------

    // Mount the internal flash filesystem.
    if (!LittleFS.begin(true))
    {
        Serial.println("LittleFS initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    //-------------------------------------------------------------------------
    // Create CSV Header
    //-------------------------------------------------------------------------

    // Create the CSV header if the file doesn't already exist.
    if (!LittleFS.exists("/lux_log.csv"))
    {
        File file = LittleFS.open(
            "/lux_log.csv",
            FILE_WRITE
        );

        if (file)
        {
            file.println("Elapsed_Minutes,Lux");
            file.close();
        }
    }

    Serial.println("Ambient Lux Logger started.");

    //-------------------------------------------------------------------------
    // Create FreeRTOS Logger Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        loggerTask,
        "Lux Logger Task",
        LOGGER_TASK_STACK_SIZE,
        NULL,
        LOGGER_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================
//
// The actual application runs inside the FreeRTOS task above.
// The Arduino loop is therefore kept idle.
//

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
