/*
  Matrix Row Scanner

  Each button press moves a solid horizontal bar
  one row upward.

  After the fifth row, it returns to the first.

  Includes software button debouncing.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25
#define BUTTON_PIN 17

#define DEBOUNCE_TIME 50  // milliseconds

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

bool lastButtonReading = HIGH;
bool buttonState = HIGH;

unsigned long lastDebounceTime = 0;

int row = 0;


// Convert coordinates to LED number
int pixelXY(int x, int y) {

  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


// Draw the horizontal bar
void drawRow() {

  matrix.clear();

  uint32_t color =
    matrix.Color(0, 255, 80);

  for (int x = 0; x < 5; x++) {

    matrix.setPixelColor(
      pixelXY(x, row),
      color
    );
  }

  matrix.show();
}


void setup() {

  Serial.begin(115200);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  matrix.begin();
  matrix.setBrightness(25);

  drawRow();

  Serial.println("Row Scanner");
  Serial.println("Press button to move.");
}


void loop() {

  // Read the button
  bool reading = digitalRead(BUTTON_PIN);


  // If the raw reading changed,
  // restart the debounce timer
  if (reading != lastButtonReading) {

    lastDebounceTime = millis();
  }


  // Check if the reading has remained
  // stable for the debounce period
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {

    // If the confirmed button state changed
    if (reading != buttonState) {

      buttonState = reading;


      // Button has been pressed
      if (buttonState == LOW) {

        row++;

        if (row >= 5)
          row = 0;

        drawRow();

        Serial.print("Row: ");
        Serial.println(row + 1);
      }
    }
  }


  // Save the current raw reading
  lastButtonReading = reading;
}
