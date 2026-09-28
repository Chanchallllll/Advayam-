/*
 * =============================================================================
 * SIMON SAYS MEMORY GAME - FreeRTOS VERSION
 * =============================================================================
 *
 * The matrix flashes a growing sequence of four colours.
 *
 * PLAYER INPUT:
 *   Touch GPIO 8 -> Cycle through colours
 *   Button GPIO 17 -> Select/confirm current colour
 *
 * COLOUR ORDER:
 *   RED -> GREEN -> BLUE -> YELLOW -> RED...
 *
 * Every touch gives a short beep.
 *
 * The sequence gets longer after every successful round.
 *
 * FreeRTOS:
 *   Game task       -> Controls the Simon Says game
 *   Button/touch    -> Read directly by the game task
 *   Matrix access   -> Protected by a mutex
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Touch  = GPIO 8
 * Buzzer = GPIO 16
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define TOUCH_PIN 8
#define BUZZER_PIN 16

#define MAX_SEQUENCE 20

Adafruit_NeoPixel matrix(
  25,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

byte sequence[MAX_SEQUENCE];
int sequenceLength = 1;

// Used for detecting a new touch.
bool lastTouchState = HIGH;

// Mutex protects NeoPixel access.
SemaphoreHandle_t matrixMutex;

// FreeRTOS task handle.
TaskHandle_t gameTaskHandle = NULL;


//=============================================================================
// Matrix / Colour Functions
//=============================================================================

void allColor(uint32_t c)
{
  if (xSemaphoreTake(matrixMutex, portMAX_DELAY) == pdTRUE)
  {
    for (int i = 0; i < 25; i++)
      matrix.setPixelColor(i, c);

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }
}

void clearMatrix()
{
  if (xSemaphoreTake(matrixMutex, portMAX_DELAY) == pdTRUE)
  {
    matrix.clear();
    matrix.show();

    xSemaphoreGive(matrixMutex);
  }
}

void showColor(byte color)
{
  uint32_t c;

  if (color == 0)
    c = matrix.Color(255, 0, 0);

  else if (color == 1)
    c = matrix.Color(0, 255, 0);

  else if (color == 2)
    c = matrix.Color(0, 0, 255);

  else
    c = matrix.Color(255, 180, 0);

  allColor(c);

  tone(
    BUZZER_PIN,
    400 + color * 250,
    200
  );

  vTaskDelay(pdMS_TO_TICKS(300));

  clearMatrix();

  vTaskDelay(pdMS_TO_TICKS(150));
}


//=============================================================================
// Show Currently Selected Colour
//=============================================================================

void showSelection(byte color)
{
  uint32_t c;

  if (color == 0)
    c = matrix.Color(255, 0, 0);

  else if (color == 1)
    c = matrix.Color(0, 255, 0);

  else if (color == 2)
    c = matrix.Color(0, 0, 255);

  else
    c = matrix.Color(255, 180, 0);

  allColor(c);
}


//=============================================================================
// Touch Sensor
//=============================================================================

bool touchPressed()
{
  bool touchState = digitalRead(TOUCH_PIN);

  bool pressed =
    (lastTouchState == HIGH &&
     touchState == LOW);

  lastTouchState = touchState;

  return pressed;
}


//=============================================================================
// Player Selection
//=============================================================================

byte waitForSelection()
{
  byte selected = 0;

  // Start with RED.
  showSelection(selected);

  while (true)
  {
    //=========================================================
    // Touch -> cycle to next colour
    //=========================================================

    if (touchPressed())
    {
      selected++;

      if (selected > 3)
        selected = 0;

      // Short beep for every touch.
      digitalWrite(BUZZER_PIN, HIGH);

      vTaskDelay(
        pdMS_TO_TICKS(100)
      );

      digitalWrite(BUZZER_PIN, LOW);

      // Touch debounce.
      vTaskDelay(
        pdMS_TO_TICKS(200)
      );

      showSelection(selected);
    }

    //=========================================================
    // Button -> confirm colour
    //=========================================================

    if (digitalRead(BUTTON_PIN) == LOW)
    {
      // Debounce before checking release.
      vTaskDelay(
        pdMS_TO_TICKS(150)
      );

      while (digitalRead(BUTTON_PIN) == LOW)
      {
        vTaskDelay(
          pdMS_TO_TICKS(10)
        );
      }

      return selected;
    }

    // Allow other FreeRTOS tasks to execute.
    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}


//=============================================================================
// Failure / Success
//=============================================================================

void failure()
{
  for (int i = 0; i < 3; i++)
  {
    allColor(
      matrix.Color(255, 0, 0)
    );

    tone(
      BUZZER_PIN,
      150,
      150
    );

    vTaskDelay(
      pdMS_TO_TICKS(200)
    );

    clearMatrix();

    vTaskDelay(
      pdMS_TO_TICKS(100)
    );
  }
}

void success()
{
  allColor(
    matrix.Color(0, 255, 0)
  );

  tone(
    BUZZER_PIN,
    1200,
    200
  );

  vTaskDelay(
    pdMS_TO_TICKS(300)
  );

  clearMatrix();
}


//=============================================================================
// Simon Says Game Task
//=============================================================================

void gameTask(void *parameter)
{
  while (true)
  {
    Serial.print("Round: ");
    Serial.println(sequenceLength);

    //===========================================================
    // Show the entire sequence.
    //===========================================================

    for (int i = 0;
         i < sequenceLength;
         i++)
    {
      showColor(sequence[i]);

      vTaskDelay(
        pdMS_TO_TICKS(150)
      );
    }

    //===========================================================
    // Player reproduces the sequence.
    //===========================================================

    bool failed = false;

    for (int i = 0;
         i < sequenceLength;
         i++)
    {
      byte answer =
        waitForSelection();

      Serial.print("Selected: ");
      Serial.println(answer);

      if (answer != sequence[i])
      {
        Serial.println("Wrong!");

        failure();

        sequenceLength = 1;

        vTaskDelay(
          pdMS_TO_TICKS(1000)
        );

        failed = true;
        break;
      }
    }

    if (failed)
      continue;

    //===========================================================
    // Round completed successfully.
    //===========================================================

    Serial.println("Correct!");

    success();

    sequenceLength++;

    if (sequenceLength > MAX_SEQUENCE)
      sequenceLength = MAX_SEQUENCE;

    vTaskDelay(
      pdMS_TO_TICKS(800)
    );
  }
}


//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  pinMode(
    TOUCH_PIN,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  matrix.begin();
  matrix.setBrightness(7);
  matrix.clear();
  matrix.show();

  // Create mutex before starting the task.
  matrixMutex = xSemaphoreCreateMutex();

  randomSeed(
    (uint32_t)esp_random()
  );

  // Generate the complete random sequence once.
  for (int i = 0;
       i < MAX_SEQUENCE;
       i++)
  {
    sequence[i] = random(0, 4);
  }

  Serial.println(
    "Simon Says FreeRTOS started."
  );

  // Create the main game task.
  xTaskCreatePinnedToCore(
    gameTask,
    "SimonGame",
    4096,
    NULL,
    1,
    &gameTaskHandle,
    1
  );
}


//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  // The application is running inside the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
