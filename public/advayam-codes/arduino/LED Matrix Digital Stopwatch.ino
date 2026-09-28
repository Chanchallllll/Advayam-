/*
  Stopwatch on Matrix

  This program uses a push button to start and stop a stopwatch.
  The LED matrix progressively fills with one LED for each
  elapsed second.

  After 25 seconds, the display cycles through the matrix again
  while the stopwatch continues counting.

  The elapsed time is also displayed on the Serial Monitor.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7       // LED matrix data pin
#define NUM_LEDS 25        // Number of LEDs
#define BUTTON_PIN 17      // Stopwatch button pin


// Create the NeoPixel matrix
Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


bool running = false;       // Stopwatch running state
bool lastButton = HIGH;     // Previous button state

unsigned long startMillis = 0;          // Start time
unsigned long elapsedBeforePause = 0;   // Time saved before stopping


// Draw the stopwatch progress on the matrix
void drawStopwatch(unsigned long seconds) {

  // Clear the previous display
  matrix.clear();

  // Cycle through the 25 LEDs
  int count = seconds % 25;

  // Fill one LED for each elapsed second
  for (int i = 0; i < count; i++) {

    matrix.setPixelColor(
      i,
      matrix.Color(0, 150, 255)
    );
  }

  // Show the current position in orange
  if (count < 25) {

    matrix.setPixelColor(
      count,
      matrix.Color(255, 80, 0)
    );
  }

  // Update the matrix
  matrix.show();
}


void setup() {

  // Start Serial Monitor
  Serial.begin(115200);

  // Configure button with internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  //initialize the matrix
  matrix.begin();

  // Set LED brightness
  matrix.setBrightness(50);

  // Start with LEDs off
  matrix.clear();
  matrix.show();

  // Display instructions
  Serial.println("Stopwatch");
  Serial.println("Button = Start / Stop");
}


void loop() {

  // Read the button
  bool button = digitalRead(BUTTON_PIN);

  // Detect a new button press
  if (lastButton == HIGH && button == LOW) {

    // Start or resume the stopwatch
    if (!running) {

      startMillis = millis();
      running = true;

      Serial.println("START");
    }

    // Stop and save the elapsed time
    else {

      elapsedBeforePause +=
        millis() - startMillis;

      running = false;

      Serial.println("STOP");
    }

    // Basic button debounce
    delay(100);
  }

  // Save the current button state
  lastButton = button;


  unsigned long elapsed;

  // Calculate current elapsed time
  if (running) {

    elapsed =
      elapsedBeforePause +
      (millis() - startMillis);

  } else {

    elapsed = elapsedBeforePause;
  }


  // Convert milliseconds to seconds
  unsigned long seconds = elapsed / 1000;

  // Update the matrix
  drawStopwatch(seconds);


  // Print only when the second changes
  static unsigned long lastPrint = 0;

  if (seconds != lastPrint) {

    Serial.print("Elapsed: ");
    Serial.print(seconds);
    Serial.println(" seconds");

    lastPrint = seconds;
  }

  // Small delay to reduce unnecessary processing
  delay(20);
}
