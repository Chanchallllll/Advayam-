/*
 * =============================================================================
 * LONG PRESS FACTORY RESET DEMONSTRATION - FreeRTOS VERSION
 * =============================================================================
 *
 * This program detects how long a button connected to GPIO 17 is held.
 *
 * A normal short press does nothing.
 *
 * While the button is being held:
 *   - The LED connected to GPIO 45 blinks progressively faster.
 *   - This shows that the reset timer is counting down.
 *
 * If the button is held for 5 seconds:
 *   - A reset sound is played through the buzzer on GPIO 16.
 *   - The LED flashes to indicate that the factory-reset condition
 *     has been reached.
 *
 * This is a starter-kit demonstration, so no actual flash, NVS, or EEPROM
 * data is erased.
 *
 * The performFactoryReset() function is where real application reset
 * logic can be added.
 *
 * FreeRTOS:
 *   The complete button/reset demonstration runs inside a dedicated task.
 *
 * =============================================================================
 */

#include <Arduino.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define BUTTON_PIN 17
#define USER_LED   45
#define BUZZER_PIN 16

//=============================================================================
// Long Press Configuration
//=============================================================================

#define RESET_TIME 5000

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define RESET_TASK_STACK_SIZE 4096
#define RESET_TASK_PRIORITY   1

#define BUTTON_CHECK_INTERVAL 5

//=============================================================================
// Factory Reset Sound
//=============================================================================

void resetSound()
{
    // Three short confirmation beeps.
    //
    // These beeps provide an audible indication that
    // the 5-second reset condition has been reached.

    for (int i = 0; i < 3; i++)
    {
        digitalWrite(
            BUZZER_PIN,
            HIGH
        );

        vTaskDelay(
            pdMS_TO_TICKS(150)
        );

        digitalWrite(
            BUZZER_PIN,
            LOW
        );

        vTaskDelay(
            pdMS_TO_TICKS(150)
        );
    }
}

//=============================================================================
// Factory Reset Function
//=============================================================================

void performFactoryReset()
{
    Serial.println(
        "FACTORY RESET TRIGGERED"
    );

    // Play the reset confirmation sound.
    resetSound();

    //-------------------------------------------------------------------------
    // Reset Indication
    //-------------------------------------------------------------------------

    // Flash the LED five times to visually confirm
    // that the reset condition has been reached.

    for (int i = 0; i < 5; i++)
    {
        digitalWrite(
            USER_LED,
            HIGH
        );

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );

        digitalWrite(
            USER_LED,
            LOW
        );

        vTaskDelay(
            pdMS_TO_TICKS(100)
        );
    }

    //-------------------------------------------------------------------------
    // Actual Application Reset Logic
    //-------------------------------------------------------------------------
    //
    // Add actual reset logic here.
    //
    // For example:
    //   - Clear configuration
    //   - Reset user preferences
    //   - Clear NVS/EEPROM data
    //
}

//=============================================================================
// Factory Reset Task
//=============================================================================

void resetTask(void *parameter)
{
    bool wasPressed = false;

    // Prevents the reset function from being triggered repeatedly
    // while the button remains held after 5 seconds.
    bool resetTriggered = false;

    // Stores when the current button hold started.
    unsigned long pressStartTime = 0;

    // Stores when the LED was last changed.
    unsigned long lastBlinkTime = 0;

    while (true)
    {
        // Because INPUT_PULLUP is being used,
        // LOW means the button is pressed.
        bool buttonPressed =
            digitalRead(BUTTON_PIN) == LOW;

        //=====================================================================
        // Button Press Started
        //=====================================================================

        if (
            buttonPressed &&
            !wasPressed
        )
        {
            // Record when the button was pressed.
            pressStartTime = millis();

            // Start LED timing from the beginning of the hold.
            lastBlinkTime = millis();

            // Allow the reset condition to be triggered
            // again for this new button press.
            resetTriggered = false;

            Serial.println(
                "Button hold started"
            );
        }

        //=====================================================================
        // Button Being Held
        //=====================================================================

        if (buttonPressed)
        {
            unsigned long holdTime =
                millis() - pressStartTime;

            unsigned long blinkInterval;

            // The longer the button is held,
            // the faster the LED blinks.

            if (holdTime < 1000)
            {
                blinkInterval = 500;
            }
            else if (holdTime < 2000)
            {
                blinkInterval = 350;
            }
            else if (holdTime < 3000)
            {
                blinkInterval = 250;
            }
            else if (holdTime < 4000)
            {
                blinkInterval = 150;
            }
            else
            {
                blinkInterval = 75;
            }

            // Change LED state whenever the selected
            // blink interval has elapsed.
            if (
                millis() - lastBlinkTime >=
                blinkInterval
            )
            {
                digitalWrite(
                    USER_LED,
                    !digitalRead(USER_LED)
                );

                lastBlinkTime = millis();
            }

            //=================================================================
            // 5 Second Reset Threshold
            //=================================================================

            if (
                holdTime >= RESET_TIME &&
                !resetTriggered
            )
            {
                // Make sure the reset occurs only once
                // during this button hold.
                resetTriggered = true;

                // Stop the progressive blinking.
                digitalWrite(
                    USER_LED,
                    LOW
                );

                // Perform the reset demonstration.
                performFactoryReset();
            }
        }

        //=====================================================================
        // Button Released
        //=====================================================================

        if (
            !buttonPressed &&
            wasPressed
        )
        {
            // Always turn the LED off after release.
            digitalWrite(
                USER_LED,
                LOW
            );

            if (!resetTriggered)
            {
                Serial.println(
                    "Button released before 5 seconds"
                );
            }
        }

        // Save the current state for the next iteration.
        wasPressed = buttonPressed;

        // Give FreeRTOS time to run other tasks.
        vTaskDelay(
            pdMS_TO_TICKS(
                BUTTON_CHECK_INTERVAL
            )
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

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

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    // Start with both output devices turned off.
    digitalWrite(
        USER_LED,
        LOW
    );

    digitalWrite(
        BUZZER_PIN,
        LOW
    );

    Serial.println(
        "Long Press Factory Reset"
    );

    Serial.println(
        "Hold button for 5 seconds."
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Reset Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        resetTask,
        "Factory Reset Task",
        RESET_TASK_STACK_SIZE,
        NULL,
        RESET_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
