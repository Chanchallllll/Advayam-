/*
  Matrix Column Scanner - FreeRTOS Version

  Each button press moves a solid vertical bar
  one column to the right.

  After the fifth column, it returns to the first.

  Includes software button debouncing.

  FreeRTOS tasks:
    1. ButtonTask  - Reads and debounces the button.
    2. DisplayTask - Displays the selected column.

  A mutex protects the shared column variable.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25
#define BUTTON_PIN 17
#define POWER_PIN 16

#define DEBOUNCE_TIME 50


Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// ============================================================================
// Button state
// ============================================================================

bool lastButtonReading = HIGH;
bool buttonState = HIGH;


// ============================================================================
// Current column
// ============================================================================

int column = 0;


// Mutex protecting the column variable.
SemaphoreHandle_t columnMutex;


// ============================================================================
// Convert coordinates to LED number
// ============================================================================

int pixelXY(
  int x,
  int y
)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


// ============================================================================
// Draw the vertical bar
// ============================================================================

void drawColumn(
  int selectedColumn
)
{
  matrix.clear();

  uint32_t color =
    matrix.Color(0, 150, 255);

  for (int y = 0; y < 5; y++)
  {
    matrix.setPixelColor(
      pixelXY(
        selectedColumn,
        y
      ),
      color
    );
  }

  matrix.show();
}


// ============================================================================
// Button task
// ============================================================================

void ButtonTask(void *parameter)
{
  TickType_t lastDebounceTime = 0;

  while (true)
  {
    bool reading =
      digitalRead(BUTTON_PIN);


    // Has the raw button reading changed?
    if (reading != lastButtonReading)
    {
      // Reset debounce timer.
      lastDebounceTime =
        xTaskGetTickCount();
    }


    // If the reading has remained stable
    // for the debounce period...
    if (
      (xTaskGetTickCount() - lastDebounceTime)
      >
      pdMS_TO_TICKS(DEBOUNCE_TIME)
    )
    {
      // Has the confirmed button state changed?
      if (reading != buttonState)
      {
        buttonState = reading;


        // Button is pressed.
        if (buttonState == LOW)
        {
          if (xSemaphoreTake(
                columnMutex,
                portMAX_DELAY
              ) == pdTRUE)
          {
            column++;

            if (column >= 5)
              column = 0;

            Serial.print("Column: ");
            Serial.println(
              column + 1
            );

            xSemaphoreGive(
              columnMutex
            );
          }
        }
      }
    }


    // Save current raw reading.
    lastButtonReading =
      reading;


    // Button scanning interval.
    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}


// ============================================================================
// Matrix display task
// ============================================================================

void DisplayTask(void *parameter)
{
  while (true)
  {
    int selectedColumn;

    if (xSemaphoreTake(
          columnMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      selectedColumn =
        column;

      xSemaphoreGive(
        columnMutex
      );
    }


    drawColumn(
      selectedColumn
    );


    // Refresh display.
    vTaskDelay(
      pdMS_TO_TICKS(50)
    );
  }
}


// ============================================================================
// Setup
// ============================================================================

void setup()
{
  Serial.begin(115200);


  pinMode(
    POWER_PIN,
    OUTPUT
  );

  digitalWrite(
    POWER_PIN,
    LOW
  );


  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );


  matrix.begin();

  matrix.setBrightness(25);


  // Create column mutex.
  columnMutex =
    xSemaphoreCreateMutex();

  if (columnMutex == NULL)
  {
    Serial.println(
      "ERROR: Failed to create column mutex."
    );

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }


  drawColumn(0);


  Serial.println(
    "Column Scanner"
  );

  Serial.println(
    "Press button to move."
  );


  // Create button task.
  xTaskCreatePinnedToCore(
    ButtonTask,
    "ButtonTask",
    2048,
    NULL,
    2,
    NULL,
    0
  );


  // Create display task.
  xTaskCreatePinnedToCore(
    DisplayTask,
    "DisplayTask",
    2048,
    NULL,
    1,
    NULL,
    1
  );
}


// ============================================================================
// Arduino loop
// ============================================================================

void loop()
{
  // All application logic is handled by FreeRTOS.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
