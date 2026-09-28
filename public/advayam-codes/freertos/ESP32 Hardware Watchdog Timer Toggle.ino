/*
 * This program uses the ESP32 Task Watchdog Timer to monitor
 * the main loop.
 *
 * The User LED on GPIO 45 toggles whenever the loop executes
 * successfully and feeds the watchdog.
 */

#include <Arduino.h>
#include "esp_task_wdt.h"

#define USER_LED 45

// Watchdog timeout in seconds
#define WDT_TIMEOUT 3

void setup()
{
    Serial.begin(115200);

    pinMode(USER_LED, OUTPUT);

    digitalWrite(USER_LED, LOW);

    // Start the Task Watchdog Timer.
    // The second argument enables panic/reset behavior
    // if the watchdog is not serviced in time.
    esp_task_wdt_init(WDT_TIMEOUT, true);

    // Register the current Arduino task with the watchdog.
    esp_task_wdt_add(NULL);

    Serial.println("Task Watchdog Started");
}

void loop()
{
    // Toggle the LED to show that the main loop is running.
    digitalWrite(
        USER_LED,
        !digitalRead(USER_LED)
    );

    Serial.println("Watchdog heartbeat");

    // Tell the watchdog that the task is still running.
    esp_task_wdt_reset();

    delay(500);
}
