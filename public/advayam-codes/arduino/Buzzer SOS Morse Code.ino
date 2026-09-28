/*
 * This program sends the Morse code distress signal "SOS" (... --- ...)
 * through a buzzer connected to GPIO 16.
 *
 * Pressing the button connected to GPIO 17 once triggers the buzzer to
 * play the complete SOS signal. The button uses the internal pull-up
 * resistor, so it is normally HIGH and becomes LOW when pressed.
 */

#include <Arduino.h>

//====================================================
// GPIO Configuration
//====================================================
#define BUTTON_PIN 17
#define BUZZER_PIN 16

//====================================================
// Morse Timing
//====================================================
#define DOT_TIME       100
#define DASH_TIME      300
#define SYMBOL_GAP     100
#define LETTER_GAP     300

//====================================================
// Generate Morse Tone
//====================================================
void beep(int duration)
{
    // Turn the buzzer on and keep it on for the required
    // amount of time. A short duration represents a dot,
    // while a longer duration represents a dash.
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);

    // Turn the buzzer off and wait before the next symbol.
    // This creates the gap between individual Morse symbols.
    digitalWrite(BUZZER_PIN, LOW);
    delay(SYMBOL_GAP);
}

//====================================================
// Send SOS
//====================================================
void sendSOS()
{
    // S = ...
    // Three short beeps represent the letter S.
    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);

    // Pause between S and O.
    delay(LETTER_GAP);

    // O = ---
    // Three long beeps represent the letter O.
    beep(DASH_TIME);
    beep(DASH_TIME);
    beep(DASH_TIME);

    // Pause between O and the final S.
    delay(LETTER_GAP);

    // S = ...
    // Three short beeps complete the SOS signal.
    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);
}

//====================================================
// Setup
//====================================================
void setup()
{
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    // Make sure the buzzer is silent when the board starts.
    digitalWrite(BUZZER_PIN, LOW);
}

//====================================================
// Loop
//====================================================
void loop()
{
    // Store the previous button state so that we can detect
    // the moment the button changes from HIGH to LOW.
    static bool lastButtonState = HIGH;

    bool buttonState = digitalRead(BUTTON_PIN);

    // Detect button press
    if (lastButtonState == HIGH && buttonState == LOW)
    {
        // A HIGH-to-LOW transition means the button was pressed.
        // Send the complete SOS signal once.
        sendSOS();

        // Debounce
        // Ignore very quick changes caused by mechanical
        // bouncing of the physical button.
        delay(200);
    }

    // Save the current state so it can be compared with the
    // button state during the next loop iteration.
    lastButtonState = buttonState;
}
