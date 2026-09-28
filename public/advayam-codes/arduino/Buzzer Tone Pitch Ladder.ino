/*
  Buzzer Tone Pitch Ladder

  This program uses a push button to cycle through 8 musical notes,
  from C4 to C5. Each time the button is pressed, the buzzer plays
  the next note in the sequence.

  The button uses INPUT_PULLUP, so a LOW signal indicates a press.
  A debounce system is included to prevent a single button press
  from being detected multiple times due to mechanical button bounce.

  After the final note (C5), the sequence starts again from C4.
*/

#include <Arduino.h>

//========== PIN CONFIG ==========
#define BUTTON_PIN 17  // GPIO pin connected to the push button
#define BUZZER_PIN 16  // GPIO pin connected to the buzzer
//================================


// Musical note frequencies in Hz
const int notes[] = {
  262,  // C4
  294,  // D4
  330,  // E4
  349,  // F4
  392,  // G4
  440,  // A4
  494,  // B4
  523   // C5
};

// Names corresponding to the note frequencies
const char* noteNames[] = {
  "C4", "D4", "E4", "F4",
  "G4", "A4", "B4", "C5"
};

int currentNote = 0;  // Keeps track of which note will be played next

// Button states used for detecting a press
bool lastButtonState = HIGH;
bool currentButtonState = HIGH;

// Debouncing variables
unsigned long lastDebounceTime = 0;  // Stores when the button state last changed
const unsigned long debounceDelay = 50;  // Ignore changes for 50 ms

void setup() {
  Serial.begin(115200);  // Start serial communication

  // Button uses the ESP32's internal pull-up resistor
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Set buzzer pin as an output
  pinMode(BUZZER_PIN, OUTPUT);

  Serial.println("Buzzer Tone Pitch Ladder");
  Serial.println("Press button to cycle notes.");
}

void loop() {
  // Read the current physical state of the button
  bool reading = digitalRead(BUTTON_PIN);

  // If the physical reading changed, restart the debounce timer
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  // Only accept the new state if it has remained stable long enough
  if ((millis() - lastDebounceTime) > debounceDelay) {

    // Check whether the stable button state has actually changed
    if (reading != currentButtonState) {
      currentButtonState = reading;

      // Because INPUT_PULLUP is used, LOW means the button is pressed
      if (currentButtonState == LOW) {

        Serial.print("Playing: ");
        Serial.println(noteNames[currentNote]);

        // Play the current note for 300 ms
        tone(BUZZER_PIN, notes[currentNote], 300);

        // Move to the next note
        currentNote++;

        // Start again at C4 after reaching C5
        if (currentNote >= 8) {
          currentNote = 0;
        }
      }
    }
  }

  // Save the physical reading for comparison during the next loop
  lastButtonState = reading;
}
