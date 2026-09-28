/*
 * =============================================================================
 * RGB COLOUR MIXER
 * =============================================================================
 *
 * This program mixes Red, Green, and Blue components on the
 * RGB matrix.
 *
 * BUTTON:
 *   GPIO 17 cycles through the available colour components.
 *
 * TOUCH SENSOR:
 *   GPIO 8 controls the brightness level of the currently
 *   selected colour component.
 *
 * Short touch:
 *   Increases the brightness level.
 *
 * Long touch:
 *   Also increases the brightness level.
 *
 * Brightness levels:
 *   0 = OFF
 *   1 = 14%
 *   2 = 28%
 *   3 = 42%
 *   4 = 57%
 *   5 = 71%
 *   6 = 85%
 *   7 = 100%
 *
 * Colour components:
 *
 *   0 = Red
 *   1 = Green
 *   2 = Blue
 *
 * Both the button and touch sensor are handled inside
 * a dedicated FreeRTOS task.
 *
 * Hardware:
 *   Matrix      -> GPIO 7
 *   Button      -> GPIO 17
 *   Touch       -> GPIO 8
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define TOUCH_PIN 8

#define NUM_LEDS 25

// =============================================================================
// NeoPixel matrix
// =============================================================================

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Colour state
// =============================================================================

int redLevel = 0;
int greenLevel = 0;
int blueLevel = 0;

// Currently selected colour.
//
// 0 = Red
// 1 = Green
// 2 = Blue
//
int selectedColour = 0;

// =============================================================================
// Convert brightness level 0-7 to NeoPixel value 0-255
// =============================================================================

int levelToValue(
  int level
)
{
  return level * 255 / 7;
}

// =============================================================================
// Print selected colour
// =============================================================================

void printSelectedColour()
{
  Serial.print(
    "Selected colour: "
  );

  if (selectedColour == 0)
    Serial.println("RED");
  else if (selectedColour == 1)
    Serial.println("GREEN");
  else
    Serial.println("BLUE");
}

// =============================================================================
// Print current brightness
// =============================================================================

void printBrightness()
{
  int level = 0;

  if (selectedColour == 0)
  {
    level = redLevel;
  }
  else if (selectedColour == 1)
  {
    level = greenLevel;
  }
  else
  {
    level = blueLevel;
  }

  Serial.print(
    "Brightness level: "
  );

  Serial.print(level);

  Serial.print(
    " / 7"
  );

  Serial.print(
    " ("
  );

  Serial.print(
    level * 100 / 7
  );

  Serial.println(
    "%)"
  );
}

// =============================================================================
// Update the RGB matrix
// =============================================================================

void updateMatrix()
{
  int r =
    levelToValue(redLevel);

  int g =
    levelToValue(greenLevel);

  int b =
    levelToValue(blueLevel);

  matrix.fill(
    matrix.Color(
      r,
      g,
      b
    )
  );

  matrix.show();
}

// =============================================================================
// Increase brightness of selected colour
// =============================================================================

void increaseBrightness()
{
  if (selectedColour == 0)
  {
    redLevel++;

    if (redLevel > 7)
      redLevel = 0;
  }
  else if (selectedColour == 1)
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

  printBrightness();

  updateMatrix();
}

// =============================================================================
// Colour mixer FreeRTOS task
// =============================================================================

void colourMixerTask(void *parameter)
{
  // ---------------------------------------------------------------------------
  // Button state
  // ---------------------------------------------------------------------------

  bool lastButtonState = HIGH;

  // ---------------------------------------------------------------------------
  // Touch sensor state
  // ---------------------------------------------------------------------------
  //
  // The touch sensor is treated like the original button:
  //
  // HIGH = not active
  // LOW  = active
  //
  bool lastTouchState = HIGH;

  // ---------------------------------------------------------------------------
  // Timing
  // ---------------------------------------------------------------------------

  unsigned long touchStart = 0;

  const unsigned long LONG_PRESS = 1000;

  while (true)
  {
    // =======================================================================
    // READ BUTTON
    // =======================================================================

    bool buttonState =
      digitalRead(BUTTON_PIN);

    // -----------------------------------------------------------------------
    // Button pressed
    // -----------------------------------------------------------------------

    if (
      lastButtonState == HIGH &&
      buttonState == LOW
    )
    {
      // Cycle to the next colour.
      selectedColour++;

      if (selectedColour > 2)
        selectedColour = 0;

      printSelectedColour();
      printBrightness();
    }

    // Save button state.
    lastButtonState =
      buttonState;

    // =======================================================================
    // READ TOUCH SENSOR
    // =======================================================================

    bool touchState =
      digitalRead(TOUCH_PIN);

    // -----------------------------------------------------------------------
    // Touch started
    // -----------------------------------------------------------------------

    if (
      lastTouchState == HIGH &&
      touchState == LOW
    )
    {
      touchStart = millis();
    }

    // -----------------------------------------------------------------------
    // Touch released
    // -----------------------------------------------------------------------

    if (
      lastTouchState == LOW &&
      touchState == HIGH
    )
    {
      unsigned long duration =
        millis() - touchStart;

      // ---------------------------------------------------------------------
      // Short touch
      // ---------------------------------------------------------------------

      if (duration < LONG_PRESS)
      {
        Serial.println(
          "Short touch - changing brightness."
        );

        increaseBrightness();
      }

      // ---------------------------------------------------------------------
      // Long touch
      // ---------------------------------------------------------------------

      else
      {
        Serial.println(
          "Long touch - changing brightness."
        );

        increaseBrightness();
      }

      // Small debounce delay.
      vTaskDelay(
        pdMS_TO_TICKS(30)
      );
    }

    // Save touch state.
    lastTouchState =
      touchState;

    // Poll inputs every 10 ms.
    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup()
{
  // ---------------------------------------------------------------------------
  // Serial
  // ---------------------------------------------------------------------------

  Serial.begin(115200);

  // ---------------------------------------------------------------------------
  // Matrix
  // ---------------------------------------------------------------------------

  matrix.begin();

  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  // ---------------------------------------------------------------------------
  // Button
  // ---------------------------------------------------------------------------

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );
  pinMode(16,
    OUTPUT
  );
  digitalWrite(
    16,
    LOW
  );
  // ---------------------------------------------------------------------------
  // Touch sensor
  // ---------------------------------------------------------------------------
  //
  // This assumes the external touch sensor has a digital output:
  //
  // HIGH = not touched
  // LOW  = touched
  //

  pinMode(
    TOUCH_PIN,
    INPUT
  );

  // ---------------------------------------------------------------------------
  // Initial display
  // ---------------------------------------------------------------------------

  updateMatrix();

  Serial.println(
    "RGB Colour Mixer"
  );

  Serial.println(
    "Button: cycle colour."
  );

  Serial.println(
    "Touch: change brightness."
  );

  Serial.println(
    "Colours: RED -> GREEN -> BLUE"
  );

  printSelectedColour();
  printBrightness();

  // ---------------------------------------------------------------------------
  // Create colour mixer task
  // ---------------------------------------------------------------------------

  xTaskCreatePinnedToCore(
    colourMixerTask,
    "Colour Mixer Task",
    2048,
    NULL,
    1,
    NULL,
    1
  );
}

// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // Colour mixing is handled by colourMixerTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
