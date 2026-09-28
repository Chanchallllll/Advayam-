/*
  LED Matrix Strobe Light

  This program uses a push button to control a strobe effect
  on a 25-pixel NeoPixel LED matrix.

  The first button press turns the strobe ON at 1 Hz.
  Each subsequent press increases the flashing frequency:
  1, 2, 3, 5, 7, 10, 12, and 15 Hz.

  After reaching the highest frequency, the next press turns
  the strobe OFF and resets the frequency back to 1 Hz.

  The button uses INPUT_PULLUP, so a LOW signal indicates
  that the button is being pressed.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

//========== PIN CONFIG ==========
#define MATRIX_PIN 7    // GPIO pin connected to the LED matrix
#define NUM_LEDS 25     // Number of LEDs in the matrix
#define BUTTON_PIN 17   // GPIO pin connected to the push button
//================================

// Create the NeoPixel matrix object
Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// Available strobe frequencies in Hz
const int frequencies[] = {
  1, 2, 3, 5, 7, 10, 12, 15
};

int frequencyIndex = 0;  // Tracks the current frequency

bool strobeEnabled = false;  // Stores whether the strobe is ON or OFF
bool lastButton = HIGH;      // Stores the previous button state

unsigned long lastFlash = 0; // Time when the matrix last changed state
bool flashState = false;     // Stores whether the LEDs are currently ON or OFF


// Set every LED in the matrix to the specified color
void setMatrix(uint32_t color) {

  // Apply the color to every LED
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, color);
  }

  // Send the updated colors to the physical LEDs
  matrix.show();
}


void setup() {

  // Start serial communication for status messages
  Serial.begin(115200);

  // Use the internal pull-up resistor for the button
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Initialize the LED matrix
  matrix.begin();

  // Set the LED brightness to 50 out of 255
  matrix.setBrightness(50);

  // Make sure all LEDs start turned off
  matrix.clear();
  matrix.show();

  // Display startup information in the Serial Monitor
  Serial.println("LED Matrix Strobe");
  Serial.println("Press button to enable/disable.");
  Serial.println("Each press changes frequency.");
}


void loop() {

  // Read the current button state
  bool button = digitalRead(BUTTON_PIN);

  // Detect a new button press: HIGH -> LOW
  if (lastButton == HIGH && button == LOW) {

    // If the strobe is currently OFF, turn it ON
    if (!strobeEnabled) {
      strobeEnabled = true;
    } 
    else {
      // Otherwise, move to the next frequency
      frequencyIndex++;

      // If all frequencies have been used,
      // turn the strobe OFF and reset the frequency
      if (frequencyIndex >= 8) {
        strobeEnabled = false;
        frequencyIndex = 0;
      }
    }

    // Print the current strobe status
    Serial.print("Strobe: ");

    if (strobeEnabled) {
      Serial.print("ON, ");
      Serial.print(frequencies[frequencyIndex]);
      Serial.println(" Hz");
    } 
    else {
      Serial.println("OFF");
    }

    // Simple delay to reduce accidental multiple button detections
    delay(100);
  }

  // Save the current button state for the next loop
  lastButton = button;


  // Only run the flashing effect when the strobe is enabled
  if (strobeEnabled) {

    // Calculate the time between each ON/OFF transition.
    // The frequency is multiplied by 2 because one cycle
    // consists of both an ON and an OFF state.
    unsigned long interval =
      1000 / (frequencies[frequencyIndex] * 2);

    // Check whether it is time to change the LED state
    if (millis() - lastFlash >= interval) {

      // Record the current time
      lastFlash = millis();

      // Toggle between ON and OFF
      flashState = !flashState;

      // Turn the matrix white
      if (flashState) {
        setMatrix(matrix.Color(255, 255, 255));
      } 
      // Turn the matrix off
      else {
        setMatrix(matrix.Color(0, 0, 0));
      }
    }
  }
}
