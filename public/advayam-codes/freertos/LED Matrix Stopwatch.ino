/*
  Stopwatch on Matrix - FreeRTOS Version

  This program uses a push button to start and stop a stopwatch.
  The LED matrix progressively fills with one LED for each
  elapsed second.

  After 25 seconds, the display cycles through the matrix again
  while the stopwatch continues counting.

  The elapsed time is also displayed on the Serial Monitor.

  FreeRTOS tasks:
    1. ButtonTask    - Detects and debounces the button.
    2. StopwatchTask - Maintains stopwatch timing and Serial output.
    3. DisplayTask   - Updates the LED matrix.

  Shared stopwatch data is protected with a mutex.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25
#define BUTTON_PIN 17
#define POWER_PIN 16

#define BUTTON_DEBOUNCE_MS 100

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// ============================================================================
// Shared stopwatch state
// ============================================================================

bool running = false;

unsigned long startMillis = 0;
unsigned long elapsedBeforePause = 0;

unsigned long currentElapsed = 0;


// Mutex protecting shared stopwatch variables
SemaphoreHandle_t stopwatchMutex;


// ============================================================================
// Button state
// ============================================================================

bool lastButton = HIGH;


// ============================================================================
// Draw stopwatch progress
// ============================================================================

void drawStopwatch(unsigned long seconds)
{
  matrix.clear();

  // Cycle through the 25 LEDs.
  int count = seconds % 25;

  // Fill one LED for each elapsed second.
  for (int i = 0; i < count; i++)
  {
    matrix.setPixelColor(
      i,
      matrix.Color(0, 150, 255)
    );
  }

  // Show the current position in orange.
  if (count < 25)
  {
    matrix.setPixelColor(
      count,
      matrix.Color(255, 80, 0)
    );
  }

  matrix.show();
}


// ============================================================================
// Button task
// ============================================================================

void ButtonTask(void *parameter)
{
  while (true)
  {
    bool button = digitalRead(BUTTON_PIN);

    // Detect a new button press.
    if (lastButton == HIGH && button == LOW)
    {
      if (xSemaphoreTake(stopwatchMutex, portMAX_DELAY) == pdTRUE)
      {
        // Start or resume the stopwatch.
        if (!running)
        {
          startMillis = millis();
          running = true;

          Serial.println("START");
        }

        // Stop and save the elapsed time.
        else
        {
          elapsedBeforePause +=
            millis() - startMillis;

          running = false;

          Serial.println("STOP");
        }

        xSemaphoreGive(stopwatchMutex);
      }

      // Basic button debounce.
      vTaskDelay(
        pdMS_TO_TICKS(BUTTON_DEBOUNCE_MS)
      );
    }

    lastButton = button;

    // Run the button task every 10 ms.
    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}


// ============================================================================
// Stopwatch timing task
// ============================================================================

void StopwatchTask(void *parameter)
{
  unsigned long lastPrint = 0;

  while (true)
  {
    unsigned long elapsed = 0;

    if (xSemaphoreTake(stopwatchMutex, portMAX_DELAY) == pdTRUE)
    {
      if (running)
      {
        elapsed =
          elapsedBeforePause +
          (millis() - startMillis);
      }
      else
      {
        elapsed = elapsedBeforePause;
      }

      currentElapsed = elapsed;

      xSemaphoreGive(stopwatchMutex);
    }

    unsigned long seconds =
      elapsed / 1000;

    // Print only when the displayed second changes.
    if (seconds != lastPrint)
    {
      Serial.print("Elapsed: ");
      Serial.print(seconds);
      Serial.println(" seconds");

      lastPrint = seconds;
    }

    // Stopwatch timing does not need to run extremely quickly.
    vTaskDelay(
      pdMS_TO_TICKS(20)
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
    unsigned long elapsed;

    if (xSemaphoreTake(stopwatchMutex, portMAX_DELAY) == pdTRUE)
    {
      elapsed = currentElapsed;

      xSemaphoreGive(stopwatchMutex);
    }

    unsigned long seconds =
      elapsed / 1000;

    drawStopwatch(seconds);

    // Refresh the display periodically.
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

  // Configure button with internal pull-up.
  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  // GPIO 16 is kept LOW as in the original program.
  pinMode(
    POWER_PIN,
    OUTPUT
  );

  digitalWrite(
    POWER_PIN,
    LOW
  );

  // Initialize the matrix.
  matrix.begin();

  matrix.setBrightness(10);

  matrix.clear();
  matrix.show();

  // Create mutex for shared stopwatch data.
  stopwatchMutex =
    xSemaphoreCreateMutex();

  if (stopwatchMutex == NULL)
  {
    Serial.println(
      "ERROR: Failed to create mutex."
    );

    while (true)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }

  Serial.println("Stopwatch");
  Serial.println("Button = Start / Stop");

  // Create FreeRTOS tasks.

  xTaskCreatePinnedToCore(
    ButtonTask,
    "ButtonTask",
    2048,
    NULL,
    2,
    NULL,
    0
  );

  xTaskCreatePinnedToCore(
    StopwatchTask,
    "StopwatchTask",
    2048,
    NULL,
    1,
    NULL,
    1
  );

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
// Main Arduino loop
// ============================================================================

void loop()
{
  // The application is now controlled by FreeRTOS tasks.
  // Nothing needs to run from the Arduino loop.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
