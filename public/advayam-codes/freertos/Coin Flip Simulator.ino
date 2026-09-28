/*
 * =============================================================================
 * COIN FLIP SIMULATOR
 * =============================================================================
 *
 * Press the button to flip the coin.
 *
 * The matrix shows a spinning coin animation, then stops on
 * HEADS or TAILS and plays a chime.
 *
 * The coin animation and button handling run inside a dedicated
 * FreeRTOS task.
 *
 * Hardware:
 *   Matrix -> GPIO 7
 *   Button -> GPIO 17
 *   Buzzer -> GPIO 16
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

#define BUTTON_PIN 17
#define BUZZER_PIN 16

// =============================================================================
// NeoPixel matrix
// =============================================================================

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Matrix coordinate mapping
// =============================================================================

int pixelXY(
  int x,
  int y
)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

// =============================================================================
// Set one matrix pixel
// =============================================================================

void setPixelXY(
  int x,
  int y,
  uint32_t colour
)
{
  if (
    x < 0 ||
    x >= 5 ||
    y < 0 ||
    y >= 5
  )
    return;

  matrix.setPixelColor(
    pixelXY(x, y),
    colour
  );
}

// =============================================================================
// Draw the coin
//
// mode:
//   0 = heads
//   1 = tails
//   2 = thin coin / edge-on
//   3 = blank
// =============================================================================

void drawCoin(
  int mode
)
{
  matrix.clear();

  uint32_t colour =
    matrix.Color(
      255,
      180,
      0
    );

  // ---------------------------------------------------------------------------
  // Normal coin
  // ---------------------------------------------------------------------------

  if (
    mode == 0 ||
    mode == 1
  )
  {
    // Coin outline.
    for (int x = 1; x <= 3; x++)
    {
      setPixelXY(x, 0, colour);
      setPixelXY(x, 4, colour);
    }

    for (int y = 1; y <= 3; y++)
    {
      setPixelXY(0, y, colour);
      setPixelXY(4, y, colour);
    }

    // HEADS.
    if (mode == 0)
    {
      setPixelXY(2, 1, colour);
      setPixelXY(1, 2, colour);
      setPixelXY(2, 2, colour);
      setPixelXY(3, 2, colour);
      setPixelXY(2, 3, colour);
    }

    // TAILS.
    else
    {
      setPixelXY(1, 1, colour);
      setPixelXY(3, 1, colour);
      setPixelXY(2, 2, colour);
      setPixelXY(1, 3, colour);
      setPixelXY(3, 3, colour);
    }
  }

  // ---------------------------------------------------------------------------
  // Coin viewed edge-on
  // ---------------------------------------------------------------------------

  else if (mode == 2)
  {
    for (int y = 0; y < 5; y++)
    {
      setPixelXY(
        2,
        y,
        colour
      );
    }
  }

  matrix.show();
}

// =============================================================================
// Coin flipping animation
// =============================================================================

void flipAnimation(
  bool result
)
{
  /*
   * The animation starts fast and gradually slows down.
   * Heads and tails alternate like a spinning coin.
   */

  int delayTime = 60;

  // Number of complete flips.
  int totalFlips = 12;

  for (int i = 0; i < totalFlips; i++)
  {
    // Show heads.
    drawCoin(0);

    vTaskDelay(
      pdMS_TO_TICKS(delayTime)
    );

    // Show edge-on.
    drawCoin(2);

    vTaskDelay(
      pdMS_TO_TICKS(delayTime / 2)
    );

    // Show tails.
    drawCoin(1);

    vTaskDelay(
      pdMS_TO_TICKS(delayTime)
    );

    // Show edge-on.
    drawCoin(2);

    vTaskDelay(
      pdMS_TO_TICKS(delayTime / 2)
    );

    // Gradually slow down.
    if (i > 5)
      delayTime += 25;
    else
      delayTime += 8;
  }

  // Display the final result.
  drawCoin(
    result ? 0 : 1
  );
}

// =============================================================================
// Play result chime
// =============================================================================

void playChime(
  bool heads
)
{
  if (heads)
  {
    tone(
      BUZZER_PIN,
      880,
      150
    );

    vTaskDelay(
      pdMS_TO_TICKS(170)
    );

    tone(
      BUZZER_PIN,
      1175,
      180
    );
  }
  else
  {
    tone(
      BUZZER_PIN,
      660,
      150
    );

    vTaskDelay(
      pdMS_TO_TICKS(170)
    );

    tone(
      BUZZER_PIN,
      440,
      180
    );
  }
}

// =============================================================================
// Coin FreeRTOS task
// =============================================================================

void coinTask(void *parameter)
{
  bool lastButton = HIGH;

  while (true)
  {
    bool button =
      digitalRead(BUTTON_PIN);

    // Detect a new button press.
    if (
      lastButton == HIGH &&
      button == LOW
    )
    {
      // Decide the result before the animation.
      // The result is not revealed until the animation ends.
      bool heads =
        random(0, 2);

      Serial.println(
        "Flipping..."
      );

      // Play the spinning animation.
      flipAnimation(heads);

      // Reveal the result.
      if (heads)
        Serial.println("HEADS");
      else
        Serial.println("TAILS");

      // Play the result sound.
      playChime(heads);

      // Button debounce.
      vTaskDelay(
        pdMS_TO_TICKS(300)
      );
    }

    lastButton = button;

    // Poll the button every 10 ms.
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
  Serial.begin(115200);

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  matrix.begin();

  matrix.setBrightness(10);

  matrix.clear();
  matrix.show();

  randomSeed(
    micros()
  );

  Serial.println(
    "Coin Flip Simulator"
  );

  Serial.println(
    "Press button to flip."
  );

  // Create the coin task.
  xTaskCreatePinnedToCore(
    coinTask,
    "Coin Task",
    4096,
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
  // Coin handling is performed by coinTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
