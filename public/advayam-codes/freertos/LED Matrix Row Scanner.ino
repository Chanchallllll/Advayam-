/*
 * =============================================================================
 * 5x5 RGB MATRIX ROW SCANNER
 * =============================================================================
 *
 * FreeRTOS version.
 *
 * A digital touch sensor connected to GPIO 8 is used as the button input.
 *
 * Each touch:
 *   - Advances the solid horizontal bar by one row.
 *   - After row 5, returns to row 1.
 *
 * Touch sensor:
 *   GPIO 8
 *
 * Matrix:
 *   GPIO 7
 *
 * The touch sensor is treated as an active-LOW push button.
 * Software debouncing is handled inside the FreeRTOS input task.
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25
#define TOUCH_PIN  8

#define DEBOUNCE_TIME_MS 50
#define TOUCH_SCAN_MS    10

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

bool lastTouchReading = HIGH;
bool touchState = HIGH;

unsigned long lastDebounceTime = 0;

int row = 0;

//------------------------------------------------------------------------------
// Convert X/Y coordinates to the physical NeoPixel index.
//------------------------------------------------------------------------------
int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//------------------------------------------------------------------------------
// Draw the currently selected horizontal row.
//------------------------------------------------------------------------------
void drawRow()
{
  matrix.clear();

  uint32_t color =
    matrix.Color(0, 255, 80);

  for (int x = 0; x < 5; x++)
  {
    matrix.setPixelColor(
      pixelXY(x, row),
      color
    );
  }

  matrix.show();
}

//------------------------------------------------------------------------------
// FreeRTOS touch-monitoring task.
//------------------------------------------------------------------------------
void touchTask(void *parameter)
{
  while (true)
  {
    bool reading = digitalRead(TOUCH_PIN);

    // Restart debounce timing whenever the raw input changes.
    if (reading != lastTouchReading)
    {
      lastDebounceTime = millis();
    }

    // Accept a state change only after it has remained stable.
    if (millis() - lastDebounceTime > DEBOUNCE_TIME_MS)
    {
      if (reading != touchState)
      {
        touchState = reading;

        // Active LOW = touch detected.
        if (touchState == LOW)
        {
          row++;

          if (row >= 5)
            row = 0;

          drawRow();

          Serial.print("Row: ");
          Serial.println(row + 1);
        }
      }
    }

    lastTouchReading = reading;

    vTaskDelay(
      pdMS_TO_TICKS(TOUCH_SCAN_MS)
    );
  }
}

void setup()
{
  Serial.begin(115200);

  pinMode(
    TOUCH_PIN,
    INPUT_PULLUP
  );

  matrix.begin();
  matrix.setBrightness(25);

  drawRow();

  Serial.println("Row Scanner");
  Serial.println("Touch sensor on GPIO 8.");

  xTaskCreate(
    touchTask,
    "TouchTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // The touch scanner is controlled by the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
