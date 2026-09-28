/*
 * This program detects how long a button connected to GPIO 17 is held.
 *
 * A normal short press does nothing. While the button is being held,
 * the LED connected to GPIO 45 blinks progressively faster to show
 * that the reset timer is counting down.
 *
 * If the button is held for 5 seconds, a reset sound is played through
 * the buzzer on GPIO 16 and the LED flashes to indicate that the
 * factory-reset condition has been reached.
 *
 * This is a starter-kit demonstration, so no actual flash, NVS, or
 * EEPROM data is erased. The performFactoryReset() function is where
 * real application reset logic can be added.
 */

#include <Arduino.h>

//====================================================
// GPIO Configuration
//====================================================
#define BUTTON_PIN 17
#define USER_LED   45
#define BUZZER_PIN 16

//====================================================
// Long Press Configuration
//====================================================
#define RESET_TIME 5000

//====================================================
// Factory Reset Sound
//====================================================
void resetSound()
{
    // Three short confirmation beeps
    // These beeps provide an audible indication that
    // the 5-second reset condition has been reached.
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(150);

        digitalWrite(BUZZER_PIN, LOW);
        delay(150);
    }
}

//====================================================
// Factory Reset Function
//====================================================
void performFactoryReset()
{
    // Print a message to the Serial Monitor so the reset
    // event can be easily seen during testing.
    Serial.println("FACTORY RESET TRIGGERED");

    // Play the reset confirmation sound.
    resetSound();

    // Reset indication
    // Flash the LED five times to visually confirm that
    // the reset condition has been reached.
    for (int i = 0; i < 5; i++)
    {
        digitalWrite(USER_LED, HIGH);
        delay(100);

        digitalWrite(USER_LED, LOW);
        delay(100);
    }

    // -------------------------------------------------
    // Add actual application reset logic here.
    // For example:
    // - Clear configuration
    // - Reset user preferences
    // - Clear NVS/EEPROM data
    // -------------------------------------------------
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    // Configure the button with the internal pull-up resistor.
    // The button normally reads HIGH and reads LOW when pressed.
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(USER_LED, OUTPUT);
    pinMode(BUZZER_PIN, OUTPUT);

    // Start with both output devices turned off.
    digitalWrite(USER_LED, LOW);
    digitalWrite(BUZZER_PIN, LOW);

    // Display startup instructions in the Serial Monitor.
    Serial.println("Long Press Factory Reset");
    Serial.println("Hold button for 5 seconds.");
}

//====================================================
// Loop
//====================================================
void loop()
{
    // wasPressed remembers the button state from the previous
    // loop so the program can detect when a press starts or ends.
    static bool wasPressed = false;

    // Prevents the reset function from being triggered repeatedly
    // while the button remains held after 5 seconds.
    static bool resetTriggered = false;

    // Stores when the current button hold started.
    static unsigned long pressStartTime = 0;

    // Stores when the LED was last changed.
    static unsigned long lastBlinkTime = 0;

    // Because INPUT_PULLUP is being used, LOW means the
    // button is currently being pressed.
    bool buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

    //================================================
    // Button Press Started
    //================================================
    if (buttonPressed && !wasPressed)
    {
        // Record the exact time the button was pressed.
        // This timestamp is used to calculate the total
        // amount of time the button has been held.
        pressStartTime = millis();

        // Start the LED timing from the beginning of the hold.
        lastBlinkTime = millis();

        // Allow the reset condition to be triggered again
        // for this new button press.
        resetTriggered = false;

        Serial.println("Button hold started");
    }

    //================================================
    // Button Being Held
    //================================================
    if (buttonPressed)
    {
        // Calculate how long the button has been held
        // by comparing the current time with the time
        // when the press started.
        unsigned long holdTime = millis() - pressStartTime;

        // Blink interval gets shorter as hold time increases
        unsigned long blinkInterval;

        // The longer the button is held, the faster the LED blinks.
        // This gives the user visual feedback that the reset
        // threshold is getting closer.
        if (holdTime < 1000)
            blinkInterval = 500;
        else if (holdTime < 2000)
            blinkInterval = 350;
        else if (holdTime < 3000)
            blinkInterval = 250;
        else if (holdTime < 4000)
            blinkInterval = 150;
        else
            blinkInterval = 75;

        // Change the LED state whenever the selected blink
        // interval has elapsed.
        if (millis() - lastBlinkTime >= blinkInterval)
        {
            // Toggle the LED instead of simply turning it on.
            // This creates the blinking effect.
            digitalWrite(USER_LED, !digitalRead(USER_LED));
            lastBlinkTime = millis();
        }

        //================================================
        // 5 Second Reset Threshold
        //================================================
        if (holdTime >= RESET_TIME && !resetTriggered)
        {
            // Mark the reset as triggered so it only happens
            // once during this button hold.
            resetTriggered = true;

            // Stop the progressive blinking before showing
            // the reset confirmation pattern.
            digitalWrite(USER_LED, LOW);

            // Perform the reset demonstration.
            performFactoryReset();
        }
    }

    //================================================
    // Button Released
    //================================================
    if (!buttonPressed && wasPressed)
    {
        // Always turn the LED off when the button is released.
        digitalWrite(USER_LED, LOW);

        // If the reset was not triggered, the user released
        // the button before reaching the 5-second threshold.
        if (!resetTriggered)
        {
            Serial.println("Button released before 5 seconds");
        }
    }

    // Save the current button state for the next loop.
    // This allows the program to detect the next press or release.
    wasPressed = buttonPressed;

    // Small delay to keep the loop responsive while reducing
    // unnecessary processing and button-state checks.
    delay(5);
}
