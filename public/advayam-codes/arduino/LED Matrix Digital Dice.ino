/*
  Digital Dice Roller

  Pressing the button starts an animated dice-rolling effect.
  After the animation, a random number from 1 to 6 is selected
  and displayed on the 5x5 NeoPixel matrix using dice dots.

  The rolling dots are shown in orange, while the final result
  is displayed in green.

  The result is also printed to the Serial Monitor.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7       // LED matrix data pin
#define NUM_LEDS 25        // Number of LEDs in the matrix
#define BUTTON_PIN 17      // Roll button pin


// Create the NeoPixel matrix
Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// Store the previous button state
bool lastButton = HIGH;


// Matrix coordinates:
// x = 0..4
// y = 0..4
//
// LED 0 = bottom-left
//
// Assumes simple row-major wiring


// Convert x/y coordinates to a LED index
int pixelXY(int x, int y) {
  return y * 5 + x;
}


// Turn off all LEDs
void clearMatrix() {

  matrix.clear();
  matrix.show();
}


// Set one LED using x/y coordinates
void setPixelXY(int x, int y, uint32_t color) {

  // Ignore coordinates outside the matrix
  if (x < 0 || x > 4 || y < 0 || y > 4)
    return;

  // Set the selected LED color
  matrix.setPixelColor(
    pixelXY(x, y),
    color
  );
}


// Draw a dice number from 1 to 6
void drawDie(int number, uint32_t color) {

  // Start with a blank matrix
  matrix.clear();


  // Center dot
  if (number == 1 ||
      number == 3 ||
      number == 5) {

    setPixelXY(2, 2, color);
  }


  // Top-left and bottom-right dots
  if (number == 2 ||
      number == 3 ||
      number == 4 ||
      number == 5 ||
      number == 6) {

    setPixelXY(1, 1, color);
    setPixelXY(3, 3, color);
  }


  // Top-right and bottom-left dots
  if (number == 4 ||
      number == 5 ||
      number == 6) {

    setPixelXY(3, 1, color);
    setPixelXY(1, 3, color);
  }


  // Middle-left and middle-right dots
  if (number == 6) {

    setPixelXY(1, 2, color);
    setPixelXY(3, 2, color);
  }


  // Update the physical LEDs
  matrix.show();
}


// Animate the dice and display the final result
void rollDice() {

  Serial.println("Rolling...");


  // Show random dice values during the animation
  for (int i = 0; i < 15; i++) {

    int randomNumber = random(1, 7);

    // Display the rolling number in orange
    drawDie(
      randomNumber,
      matrix.Color(255, 80, 0)
    );

    // Gradually slow down the animation
    delay(80 + i * 10);
  }


  // Select the final dice result
  int result = random(1, 7);


  // Display the final result in green
  drawDie(
    result,
    matrix.Color(0, 255, 0)
  );


  // Print the result to Serial Monitor
  Serial.print("Dice result: ");
  Serial.println(result);
}


void setup() {

  // Start Serial communication
  Serial.begin(115200);


  // Configure the button with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);


  // Configure GPIO 16 as an output
  pinMode(16, OUTPUT);

  // Keep GPIO 16 LOW
  digitalWrite(16, LOW);


  // Seed the random number generator
  randomSeed(micros());


  // Initialize the LED matrix
  matrix.begin();

  // Set LED brightness
  matrix.setBrightness(50);


  // Start with the matrix turned off
  clearMatrix();


  // Display instructions
  Serial.println("Digital Dice Roller");
  Serial.println("Press button to roll.");
}


void loop() {

  // Read the current button state
  bool button = digitalRead(BUTTON_PIN);


  // Detect a new button press
  if (lastButton == HIGH && button == LOW) {

    // Start the dice animation
    rollDice();

    // Basic button debounce
    delay(200);
  }


  // Save the button state for the next loop
  lastButton = button;
}
