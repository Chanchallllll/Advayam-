/*
 * =============================================================================
 * CODE 2 - BUZZER TONE PITCH LADDER
 * =============================================================================
 *
 * Each button press plays the next musical note:
 *
 * C4 -> D4 -> E4 -> F4 -> G4 -> A4 -> B4 -> C5 -> C4 ...
 *
 * Button:
 *   GPIO 17
 *
 * Passive buzzer:
 *   GPIO 16
 *
 * Uses ESP32 LEDC directly instead of Arduino tone().
 * This avoids LEDC initialization problems on ESP32 Arduino core 2.0.13.
 * =============================================================================
 */

#include <Arduino.h>

//====================================================
// GPIO Configuration
//====================================================

#define BUTTON_PIN 17
#define BUZZER_PIN 16

//====================================================
// LEDC Configuration
//====================================================

#define BUZZER_CHANNEL 0
#define BUZZER_RESOLUTION 8

//====================================================
// Musical Notes
//====================================================

const int notes[] =
{
    262,  // C4
    294,  // D4
    330,  // E4
    349,  // F4
    392,  // G4
    440,  // A4
    494,  // B4
    523   // C5
};

const char* noteNames[] =
{
    "C4",
    "D4",
    "E4",
    "F4",
    "G4",
    "A4",
    "B4",
    "C5"
};

const int NUMBER_OF_NOTES = 8;

int currentNote = 0;

//====================================================
// Button State
//====================================================

bool lastButtonState = HIGH;
bool currentButtonState = HIGH;

unsigned long lastDebounceTime = 0;

const unsigned long debounceDelay = 50;

//====================================================
// Play Note
//====================================================

void playNote(int frequency, int duration)
{
    // Configure the LEDC channel for this frequency.
    ledcSetup(
        BUZZER_CHANNEL,
        frequency,
        BUZZER_RESOLUTION
    );

    // Connect the buzzer pin to the LEDC channel.
    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    // 50% duty cycle.
    ledcWrite(
        BUZZER_CHANNEL,
        128
    );

    // Keep note playing.
    delay(duration);

    // Stop the buzzer.
    ledcWrite(
        BUZZER_CHANNEL,
        0
    );
}

//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(115200);

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    digitalWrite(
        BUZZER_PIN,
        LOW
    );

    // Initialize LEDC once at startup.
    ledcSetup(
        BUZZER_CHANNEL,
        1000,
        BUZZER_RESOLUTION
    );

    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    // Make sure the buzzer starts silent.
    ledcWrite(
        BUZZER_CHANNEL,
        0
    );

    Serial.println("Buzzer Tone Pitch Ladder");
    Serial.println("Press button to cycle notes.");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    // Read physical button state.
    bool reading = digitalRead(BUTTON_PIN);

    // If physical reading changed,
    // restart debounce timer.
    if (reading != lastButtonState)
    {
        lastDebounceTime = millis();
    }

    // Accept state only after it has remained stable.
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // Check whether stable state changed.
        if (reading != currentButtonState)
        {
            currentButtonState = reading;

            // INPUT_PULLUP:
            // LOW means button pressed.
            if (currentButtonState == LOW)
            {
                Serial.print("Playing: ");
                Serial.println(noteNames[currentNote]);

                playNote(
                    notes[currentNote],
                    300
                );

                // Advance to next note.
                currentNote++;

                // Wrap around after C5.
                if (currentNote >= NUMBER_OF_NOTES)
                {
                    currentNote = 0;
                }
            }
        }
    }

    lastButtonState = reading;
}
