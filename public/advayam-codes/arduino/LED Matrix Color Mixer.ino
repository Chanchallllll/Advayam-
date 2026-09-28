/*
 * This program mixes Red, Green, and Blue components on the
 * RGB matrix using the push button on GPIO 17.
 *
 * Short presses cycle through 8 brightness levels.
 * A long press confirms the selected component and moves to
 * the next color component.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

int redLevel = 0;
int greenLevel = 0;
int blueLevel = 0;

int selectedColor = 0;

bool lastButtonState = HIGH;

unsigned long pressStart = 0;

const unsigned long LONG_PRESS = 1000;

// Convert 0-7 level to 0-255
int levelToValue(int level)
{
  return level * 255 / 7;
}

void updateMatrix()
{
  int r = levelToValue(redLevel);
  int g = levelToValue(greenLevel);
  int b = levelToValue(blueLevel);

  matrix.fill(
    matrix.Color(r, g, b)
  );

  matrix.show();
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  updateMatrix();
}

void loop()
{
  bool buttonState =
    digitalRead(BUTTON_PIN);

  // Button pressed
  if (lastButtonState == HIGH &&
      buttonState == LOW)
  {
    pressStart = millis();
  }

  // Button released
  if (lastButtonState == LOW &&
      buttonState == HIGH)
  {
    unsigned long duration =
      millis() - pressStart;

    if (duration >= LONG_PRESS)
    {
      // Confirm and move to next component
      selectedColor++;

      if (selectedColor > 2)
        selectedColor = 0;
    }
    else
    {
      // Cycle selected component
      if (selectedColor == 0)
      {
        redLevel++;

        if (redLevel > 7)
          redLevel = 0;
      }
      else if (selectedColor == 1)
      {
        greenLevel++;

        if (greenLevel > 7)
          greenLevel = 0;
      }
      else
      {
        blueLevel++;

        if (blueLevel > 7)
          blueLevel = 0;
      }
    }

    updateMatrix();

    delay(30);
  }

  lastButtonState =
    buttonState;
}
