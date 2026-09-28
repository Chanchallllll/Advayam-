/*
  Matrix Column Scanner

  Each button press moves a solid vertical bar
  one column to the right.

  After the fifth column, it returns to the first.

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

int column = 0;


// Convert coordinates to LED number
int pixelXY(int x, int y) {

  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


// Draw the vertical bar
void drawColumn() {

  matrix.clear();

  uint32_t color =
    matrix.Color(0, 150, 255);

  for (int y = 0; y < 5; y++) {

    matrix.setPixelColor(
      pixelXY(column, y),
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

  drawColumn();

  Serial.println("Column Scanner");
  Serial.println("Press button to move.");
}


void loop() {

  // Read the button
  bool reading = digitalRead(BUTTON_PIN);


  // Has the raw button reading changed?
  if (reading != lastButtonReading) {

    // Reset debounce timer
    lastDebounceTime = millis();
  }


  // If the reading has stayed stable long enough
  if ((millis() - lastDebounceTime) > DEBOUNCE_TIME) {

    // Has the confirmed button state changed?
    if (reading != buttonState) {

      buttonState = reading;


      // Button is pressed
      if (buttonState == LOW) {

        column++;

        if (column >= 5)
          column = 0;

        drawColumn();

        Serial.print("Column: ");
        Serial.println(column + 1);
      }
    }
  }


  // Save current raw reading
  lastButtonReading = reading;
}
