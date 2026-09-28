/*
  Reaction Timer Game

  The ESP32 waits for the user to press the button, then waits
  for a random amount of time before sounding the buzzer.

  When the buzzer sounds, the player must press the button as
  quickly as possible. The reaction time is measured in
  milliseconds and displayed on the Serial Monitor.

  The random delay helps prevent the player from predicting
  when the buzzer will sound.
*/

#include <Arduino.h>

#define BUTTON_PIN 17  // Button pin
#define BUZZER_PIN 16  // Buzzer pin


// Wait until the button is released
void waitForRelease() {

  while (digitalRead(BUTTON_PIN) == LOW) {
    delay(10);
  }
}


void setup() {

  Serial.begin(115200);  // Start Serial Monitor

  pinMode(BUTTON_PIN, INPUT_PULLUP);  // Enable button pull-up
  pinMode(BUZZER_PIN, OUTPUT);        // Set buzzer as output

  randomSeed(analogRead(0));  // Initialize random numbers

  // Display game title
  Serial.println();
  Serial.println("================================");
  Serial.println("       REACTION TIMER");
  Serial.println("================================");
  Serial.println("Press the button to start.");
}


void loop() {

  // Check if the button is pressed
  if (digitalRead(BUTTON_PIN) == LOW) {

    // Make sure the button is released
    waitForRelease();

    Serial.println();
    Serial.println("Get ready...");

    // Wait for a random time before starting
    delay(random(1000, 4000));

    Serial.println("GO!");

    // Start the buzzer
    tone(BUZZER_PIN, 1000);

    // Record the start time
    unsigned long startTime = millis();

    // Wait for the player to press the button
    while (digitalRead(BUTTON_PIN) == HIGH) {
      // Waiting for response
    }

    // Calculate reaction time
    unsigned long reactionTime =
      millis() - startTime;

    // Stop the buzzer
    noTone(BUZZER_PIN);

    // Display the result
    Serial.print("Reaction time: ");
    Serial.print(reactionTime);
    Serial.println(" ms");

    // Short pause before the next round
    delay(1000);

    Serial.println();
    Serial.println("Press button to play again.");
  }
}
