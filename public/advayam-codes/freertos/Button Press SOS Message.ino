/*
 * =============================================================================
 * CODE 1 - SOS MORSE SIGNAL
 * =============================================================================
 *
 * Press the button once to transmit the Morse code distress signal:
 *
 * SOS = ... --- ...
 *
 * Button:
 *   GPIO 17
 *
 * Buzzer:
 *   GPIO 16
 *
 * The button uses INPUT_PULLUP:
 *   HIGH = not pressed
 *   LOW  = pressed
 * =============================================================================
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
// Generate Morse Beep
//====================================================

void beep(int duration)
{
    // Turn the buzzer on.
    digitalWrite(BUZZER_PIN, HIGH);

    // Keep it on for the required duration.
    delay(duration);

    // Turn the buzzer off.
    digitalWrite(BUZZER_PIN, LOW);

    // Gap between Morse symbols.
    delay(SYMBOL_GAP);
}

//====================================================
// Send SOS
//====================================================

void sendSOS()
{
    // ------------------------------------------------
    // S = ...
    // ------------------------------------------------

    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);

    // Gap between S and O.
    delay(LETTER_GAP);

    // ------------------------------------------------
    // O = ---
    // ------------------------------------------------

    beep(DASH_TIME);
    beep(DASH_TIME);
    beep(DASH_TIME);

    // Gap between O and final S.
    delay(LETTER_GAP);

    // ------------------------------------------------
    // S = ...
    // ------------------------------------------------

    beep(DOT_TIME);
    beep(DOT_TIME);
    beep(DOT_TIME);
}

//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    // Make sure buzzer is off at startup.
    digitalWrite(BUZZER_PIN, LOW);

    Serial.println("SOS Morse Signal");
    Serial.println("Press button to transmit SOS.");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    static bool lastButtonState = HIGH;

    bool buttonState = digitalRead(BUTTON_PIN);

    // Detect HIGH -> LOW transition.
    if (lastButtonState == HIGH && buttonState == LOW)
    {
        Serial.println("Sending SOS...");

        sendSOS();

        Serial.println("SOS complete.");

        // Simple debounce.
        delay(200);
    }

    lastButtonState = buttonState;
}
