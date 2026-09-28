/*
 * =============================================================================
 * SINGLE / DOUBLE / TRIPLE BUTTON CLICK - FreeRTOS VERSION
 * =============================================================================
 *
 * This program detects single, double, and triple button clicks
 * using a button connected to GPIO 17.
 *
 * Depending on the number of clicks, the LED connected to GPIO 45
 * blinks the same number of times:
 *
 *   Single click  -> 1 blink
 *   Double click  -> 2 blinks
 *   Triple click  -> 3 blinks
 *
 * The program uses debounce timing to prevent accidental multiple
 * detections caused by mechanical button bouncing.
 *
 * It also waits for a short period after a click to determine
 * whether another click follows.
 *
 * FreeRTOS:
 *   A dedicated task continuously monitors the button and processes
 *   the click sequence.
 *
 * =============================================================================
 */

#include <Arduino.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define BUTTON_PIN 17
#define USER_LED   45

//=============================================================================
// Click Configuration
//=============================================================================

#define DEBOUNCE_TIME 30
#define CLICK_TIMEOUT 400

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define CLICK_TASK_STACK_SIZE 4096
#define CLICK_TASK_PRIORITY   1

#define BUTTON_SCAN_INTERVAL 5

//=============================================================================
// Blink LED
//=============================================================================

void blinkLED(int count)
{
    // Repeat the blink sequence for the requested number of times.
    for (int i = 0; i < count; i++)
    {
        digitalWrite(
            USER_LED,
            HIGH
        );

        vTaskDelay(
            pdMS_TO_TICKS(150)
        );

        digitalWrite(
            USER_LED,
            LOW
        );

        vTaskDelay(
            pdMS_TO_TICKS(150)
        );
    }
}

//=============================================================================
// Button Task
//=============================================================================

void clickTask(void *parameter)
{
    // Previous stable button state.
    bool lastButtonState = HIGH;

    // Current debounced button state.
    bool currentButtonState = HIGH;

    // Number of clicks detected in the current sequence.
    int clickCount = 0;

    // Time of the most recent valid click.
    unsigned long lastClickTime = 0;

    // Time at which the physical button reading changed.
    unsigned long lastDebounceTime = 0;

    while (true)
    {
        // Read the current physical button state.
        bool reading =
            digitalRead(BUTTON_PIN);

        //=====================================================================
        // Physical State Changed
        //=====================================================================

        if (
            reading !=
            lastButtonState
        )
        {
            lastDebounceTime =
                millis();

            lastButtonState =
                reading;
        }

        //=====================================================================
        // Debounce
        //=====================================================================

        if (
            millis() -
            lastDebounceTime >
            DEBOUNCE_TIME
        )
        {
            // Check whether the stable button state
            // has actually changed.
            if (
                reading !=
                currentButtonState
            )
            {
                currentButtonState =
                    reading;

                // INPUT_PULLUP means LOW is pressed.
                if (
                    currentButtonState ==
                    LOW
                )
                {
                    // A valid click has been detected.
                    clickCount++;

                    lastClickTime =
                        millis();

                    Serial.print(
                        "Click detected: "
                    );

                    Serial.println(
                        clickCount
                    );
                }
            }
        }

        //=====================================================================
        // Determine Click Sequence
        //=====================================================================

        if (
            clickCount > 0 &&
            millis() - lastClickTime >
            CLICK_TIMEOUT
        )
        {
            // No additional click arrived before the timeout.
            // The click sequence is now complete.

            if (clickCount == 1)
            {
                // Single click.
                blinkLED(1);
            }
            else if (clickCount == 2)
            {
                // Double click.
                blinkLED(2);
            }
            else
            {
                // Triple click or more.
                // Any count greater than two produces three blinks.
                blinkLED(3);
            }

            // Reset the sequence.
            clickCount = 0;
        }

        // Give the CPU to other FreeRTOS tasks.
        vTaskDelay(
            pdMS_TO_TICKS(
                BUTTON_SCAN_INTERVAL
            )
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    //-------------------------------------------------------------------------
    // GPIO
    //-------------------------------------------------------------------------

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        USER_LED,
        OUTPUT
    );

    // Your original program explicitly held GPIO 16 LOW.
    // This is preserved here so the hardware starts in the same state.
    pinMode(
        16,
        OUTPUT
    );

    digitalWrite(
        16,
        LOW
    );

    // Keep the LED turned off at startup.
    digitalWrite(
        USER_LED,
        LOW
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Button Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        clickTask,
        "Click Detection Task",
        CLICK_TASK_STACK_SIZE,
        NULL,
        CLICK_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    // Button detection is handled by clickTask().
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
