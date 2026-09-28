/*
  8-bit Binary Counter

  Each press of the button increases an 8-bit counter by 1.

  The first 8 LEDs of the matrix represent the binary value:
  - LED ON  = 1
  - LED DIM = 0

  The counter goes from 0 to 255 and then wraps back to 0.

  The decimal and binary values are also printed to
  the Serial Monitor.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7       // LED matrix data pin
#define NUM_LEDS 25        // Number of LEDs
#define BUTTON_PIN 17      // Counter button pin


// Create the NeoPixel matrix
Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


uint8_t counter = 0;       // 8-bit counter value
bool lastButton = HIGH;    // Previous button state


// Display the counter as binary on the matrix
void displayBinary() {

  // Clear the matrix
  matrix.clear();

  // Use the first 8 LEDs for the 8 bits
  for (int bit = 0; bit < 8; bit++) {

    // Check whether this bit is 1
    bool state =
      counter & (1 << bit);

    if (state) {

      // ON = binary 1
      matrix.setPixelColor(
        bit,
        matrix.Color(0, 255, 0)
      );
    }

    else {

      // DIM = binary 0
      matrix.setPixelColor(
        bit,
        matrix.Color(10, 10, 10)
      );
    }
  }

  // Update the matrix
  matrix.show();
}


// Print decimal and binary values
void printBinary() {

  Serial.print("Decimal: ");
  Serial.print(counter);

  Serial.print(" Binary: ");

  // Print bits from most to least significant
  for (int bit = 7; bit >= 0; bit--) {

    Serial.print(
      (counter >> bit) & 1
    );
  }

  Serial.println();
}


void setup() {

  // Start Serial Monitor
  Serial.begin(115200);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  // Configure button with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize the matrix
  matrix.begin();

  // Set LED brightness
  matrix.setBrightness(25);

  // Display the initial value
  displayBinary();

  // Display instructions
  Serial.println("8-bit Binary Counter");
  Serial.println("Press button to increment.");

  // Print initial counter value
  printBinary();
}


void loop() {

  // Read the button
  bool button = digitalRead(BUTTON_PIN);

  // Detect a new button press
  if (lastButton == HIGH && button == LOW) {

    // Increase the counter
    counter++;

    // Update the matrix
    displayBinary();

    // Print the new value
    printBinary();

    // Basic button debounce
    delay(150);
  }

  // Save the button state
  lastButton = button;
}
