/*
 * =============================================================================
 * 8-BIT BINARY COUNTER
 * =============================================================================
 *
 * Each press of the touch sensor increases an 8-bit counter by 1.
 *
 * The first 8 LEDs of the matrix represent the binary value:
 *
 *   LED ON  = 1
 *   LED DIM = 0
 *
 * The counter goes from 0 to 255 and then wraps back to 0.
 *
 * The decimal and binary values are also printed to the Serial Monitor.
 *
 * Button:
 *   GPIO 17
 *
 * LED matrix:
 *   GPIO 7
 *
 * The touch monitoring runs inside a dedicated FreeRTOS task.
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
#define NUM_LEDS 25
#define TOUCH_PIN 8

// =============================================================================
// NeoPixel matrix
// =============================================================================

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Counter state
// =============================================================================

uint8_t counter = 0;

// =============================================================================
// Display the counter as binary
// =============================================================================

void displayBinary()
{
  // Clear the matrix.
  matrix.clear();

  // Use the first 8 LEDs for the 8 bits.
  for (int bit = 0; bit < 8; bit++)
  {
    // Check whether this bit is 1.
    bool state =
      counter & (1 << bit);

    if (state)
    {
      // Binary 1 = bright green.
      matrix.setPixelColor(
        bit,
        matrix.Color(0, 255, 0)
      );
    }
    else
    {
      // Binary 0 = dim green.
      matrix.setPixelColor(
        bit,
        matrix.Color(10, 10, 10)
      );
    }
  }

  // Send the updated data to the matrix.
  matrix.show();
}

// =============================================================================
// Print decimal and binary values
// =============================================================================

void printBinary()
{
  Serial.print("Decimal: ");
  Serial.print(counter);

  Serial.print(" Binary: ");

  // Print bits from most significant to least significant.
  for (int bit = 7; bit >= 0; bit--)
  {
    Serial.print(
      (counter >> bit) & 1
    );
  }

  Serial.println();
}

// =============================================================================
// Button monitoring FreeRTOS task
// =============================================================================

void buttonTask(void *parameter)
{
  bool lastButton = HIGH;

  while (true)
  {
    // Read the button.
    bool button =
      digitalRead(TOUCH_PIN);

    // Detect a new button press.
    //
    // The input uses INPUT_PULLUP, so:
    //   HIGH = released
    //   LOW  = pressed
    //
    if (
      lastButton == HIGH &&
      button == LOW
    )
    {
      // Increase the counter.
      //
      // uint8_t automatically wraps from 255 to 0.
      counter++;

      // Update the LED matrix.
      displayBinary();

      // Print the new value.
      printBinary();

      // Basic button debounce.
      //
      // vTaskDelay() allows other FreeRTOS tasks to execute
      // while the debounce period is active.
      vTaskDelay(
        pdMS_TO_TICKS(150)
      );
    }

    // Save the current button state.
    lastButton = button;

    // Small polling interval.
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
  // Start Serial Monitor.
  Serial.begin(115200);

  // The original program explicitly held GPIO 16 LOW.
  pinMode(
    16,
    OUTPUT
  );

  digitalWrite(
    16,
    LOW
  );

  // Configure button using the internal pull-up resistor.
  pinMode(
    TOUCH_PIN,
    INPUT_PULLUP
  );

  // Initialise the matrix.
  matrix.begin();

  // Set LED brightness.
  matrix.setBrightness(25);

  // Display the initial counter value.
  displayBinary();

  // Display instructions.
  Serial.println(
    "8-bit Binary Counter"
  );

  Serial.println(
    "Press button to increment."
  );

  // Print initial counter value.
  printBinary();

  // Create the button task.
  xTaskCreatePinnedToCore(
    buttonTask,
    "Button Task",
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
  // Button processing is handled by buttonTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
