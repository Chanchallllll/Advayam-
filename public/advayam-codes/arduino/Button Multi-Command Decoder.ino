/*
 * This program detects single, double, and triple button clicks
 * using a button connected to GPIO 17.
 *
 * Depending on the number of clicks, the LED connected to GPIO 45
 * blinks the same number of times:
 *   - Single click  -> 1 blink
 *   - Double click  -> 2 blinks
 *   - Triple click  -> 3 blinks
 *
 * The program uses a short debounce delay to prevent accidental
 * multiple detections caused by mechanical button bouncing.
 * It also waits for a short period after a click to determine
 * whether another click follows.
 */

#include <Arduino.h>

//====================================================
// GPIO Configuration
//====================================================
#define BUTTON_PIN 17
#define USER_LED   45

//====================================================
// Click Configuration
//====================================================
#define DEBOUNCE_TIME       30
#define CLICK_TIMEOUT       400

//====================================================
// Blink LED
//====================================================
void blinkLED(int count)
{
    // Repeat the blink sequence for the requested number of times.
    for (int i = 0; i < count; i++)
    {
        digitalWrite(USER_LED, HIGH);
        delay(150);

        digitalWrite(USER_LED, LOW);
        delay(150);
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    // The button uses the internal pull-up resistor.
    // This means the button is normally HIGH and becomes
    // LOW when it is pressed.
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(USER_LED, OUTPUT);
    pinMode(16,OUTPUT);
    digitalWrite(16,LOW);
    // Keep the LED turned off when the board starts.
    digitalWrite(USER_LED, LOW);
}

//====================================================
// Loop
//====================================================
void loop()
{
    // Remember the previous button state so we can detect
    // when the button changes from HIGH to LOW.
    static bool lastButtonState = HIGH;

    // clickCount keeps track of how many clicks have been detected.
    // lastClickTime records when the most recent click occurred.
    static int clickCount = 0;
    static unsigned long lastClickTime = 0;

    bool buttonState = digitalRead(BUTTON_PIN);

    // Detect button press
    if (lastButtonState == HIGH && buttonState == LOW)
    {
        // Give the physical button a short time to settle
        // and prevent contact bounce from being detected
        // as multiple clicks.
        delay(DEBOUNCE_TIME);

        // Confirm button press
        if (digitalRead(BUTTON_PIN) == LOW)
        {
            // A valid press has been detected, so increase
            // the number of clicks and record the time.
            clickCount++;
            lastClickTime = millis();

            // Wait for release
            // The program waits here until the button is released
            // before looking for another click.
            while (digitalRead(BUTTON_PIN) == LOW)
            {
                delay(1);
            }
        }
    }

    // Save the current button state for comparison
    // during the next loop iteration.
    lastButtonState = buttonState;

    // Wait for additional clicks
    if (clickCount > 0 &&
        millis() - lastClickTime > CLICK_TIMEOUT)
    {
        // The timeout has expired, so no additional click
        // was detected. The program can now determine the
        // command based on the number of clicks received.
        if (clickCount == 1)
        {
            // Single click
            blinkLED(1);
        }
        else if (clickCount == 2)
        {
            // Double click
            blinkLED(2);
        }
        else
        {
            // Triple click or more
            // Any count greater than two produces three blinks.
            blinkLED(3);
        }

        // Reset the click counter so the next button press
        // starts a new click sequence.
        clickCount = 0;
    }
}
