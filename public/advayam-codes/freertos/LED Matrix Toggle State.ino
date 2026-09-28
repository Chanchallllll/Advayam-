/*
 * =============================================================================
 * TOUCH SENSOR TOGGLE WITH AUDIBLE FEEDBACK
 * =============================================================================
 *
 * FreeRTOS version.
 *
 * A digital touch sensor on GPIO 8 behaves like the original push button.
 *
 * Each touch:
 *   - Toggles the RGB matrix ON/OFF.
 *   - Produces a short buzzer click.
 *
 * Touch sensor:
 *   GPIO 8
 *
 * Matrix:
 *   GPIO 7
 *
 * Buzzer:
 *   GPIO 16
 *
 * The button polling and debounce logic run inside a dedicated FreeRTOS task.
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define TOUCH_PIN  8
#define BUZZER_PIN 16

#define DEBOUNCE_TIME_MS 50
#define TOUCH_SCAN_MS    10

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

bool ledState = false;

void setMatrix(uint32_t color)
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    matrix.setPixelColor(i, color);
  }

  matrix.show();
}

void clickSound()
{
  tone(
    BUZZER_PIN,
    1200,
    50
  );
}

//------------------------------------------------------------------------------
// Touch monitoring task.
//
// The touch sensor is treated exactly like an active-LOW push button.
//------------------------------------------------------------------------------
void touchTask(void *parameter)
{
  bool lastReading = HIGH;
  bool stableState = HIGH;

  unsigned long lastChangeTime = 0;

  while (true)
  {
    bool reading = digitalRead(TOUCH_PIN);

    // Restart debounce timing whenever the raw state changes.
    if (reading != lastReading)
    {
      lastChangeTime = millis();
    }

    // Accept the new state only after it has remained stable.
    if (millis() - lastChangeTime >= DEBOUNCE_TIME_MS)
    {
      if (reading != stableState)
      {
        stableState = reading;

        // Active LOW means a LOW transition is a touch.
        if (stableState == LOW)
        {
          ledState = !ledState;

          if (ledState)
          {
            setMatrix(
              matrix.Color(0, 255, 0)
            );

            Serial.println("LED: ON");
          }
          else
          {
            setMatrix(
              matrix.Color(0, 0, 0)
            );

            Serial.println("LED: OFF");
          }

          clickSound();
        }
      }
    }

    lastReading = reading;

    vTaskDelay(
      pdMS_TO_TICKS(TOUCH_SCAN_MS)
    );
  }
}

void setup()
{
  Serial.begin(115200);

  // Digital touch sensor behaves like an active-LOW button.
  pinMode(
    TOUCH_PIN,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  matrix.begin();
  matrix.setBrightness(25);

  setMatrix(
    matrix.Color(0, 0, 0)
  );

  Serial.println("Touch Toggle");
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
  // Application work is performed by the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
