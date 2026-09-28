/*
 * =============================================================================
 * TUG OF WAR - PLAYER VS CPU - FREERTOS
 * =============================================================================
 *
 * Designed for Arduino IDE + ESP32 / boards with FreeRTOS.
 *
 * TASKS:
 *
 *   PlayerTask
 *      Reads the button and pulls the marker toward the player.
 *
 *   CpuTask
 *      Automatically pulls the marker toward the CPU.
 *
 *   GameTask
 *      Checks the player's 3-second hold.
 *      Detects the winner.
 *      Runs the win animation.
 *      Resets the game.
 *
 *   DisplayTask
 *      Continuously updates the NeoPixel display.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// Hardware
//=============================================================================

#define MATRIX_PIN  7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define NUM_PIXELS 25

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game Configuration
//=============================================================================

const unsigned long INITIAL_CPU_INTERVAL = 450;

const unsigned long MIN_CPU_INTERVAL = 280;

const unsigned long CPU_SPEEDUP = 8;

const unsigned long PLAYER_HOLD_TIME = 3000;

//=============================================================================
// Game State
//=============================================================================

enum GameState
{
  GAME_RUNNING,
  PLAYER_WON,
  CPU_WON,
  GAME_RESETTING
};

GameState gameState = GAME_RUNNING;

//=============================================================================
// Game Variables
//=============================================================================

int marker = 2;

unsigned long cpuInterval = INITIAL_CPU_INTERVAL;

bool holdingPlayerSide = false;

unsigned long playerHoldStart = 0;

unsigned long lastCPU = 0;

//=============================================================================
// FreeRTOS Objects
//=============================================================================

SemaphoreHandle_t gameMutex;
SemaphoreHandle_t displayMutex;
SemaphoreHandle_t buzzerMutex;

// Task handles are not needed for suspend/resume anymore,
// but keeping them available can be useful for debugging.

TaskHandle_t playerTaskHandle  = NULL;
TaskHandle_t cpuTaskHandle     = NULL;
TaskHandle_t gameTaskHandle    = NULL;
TaskHandle_t displayTaskHandle = NULL;

//=============================================================================
// Matrix Coordinate
//=============================================================================

int xy(int x, int y)
{
  if (y % 2 == 0)
  {
    return y * 5 + x;
  }

  return y * 5 + (4 - x);
}

//=============================================================================
// Draw Matrix
//=============================================================================

void draw()
{
  int localMarker;

  //-------------------------------------------------------------------------
  // Get a safe copy of the marker.
  //-------------------------------------------------------------------------

  if (xSemaphoreTake(gameMutex, portMAX_DELAY) == pdTRUE)
  {
    localMarker = marker;

    xSemaphoreGive(gameMutex);
  }

  //-------------------------------------------------------------------------
  // Protect NeoPixel hardware.
  //-------------------------------------------------------------------------

  if (xSemaphoreTake(displayMutex, portMAX_DELAY) != pdTRUE)
  {
    return;
  }

  matrix.clear();

  //-------------------------------------------------------------------------
  // PLAYER SIDE - GREEN
  //-------------------------------------------------------------------------

  for (int x = 0; x < localMarker; x++)
  {
    matrix.setPixelColor(
      xy(x, 2),
      matrix.Color(0, 255, 0)
    );
  }

  //-------------------------------------------------------------------------
  // CPU SIDE - RED
  //-------------------------------------------------------------------------

  for (int x = localMarker + 1; x < 5; x++)
  {
    matrix.setPixelColor(
      xy(x, 2),
      matrix.Color(255, 0, 0)
    );
  }

  //-------------------------------------------------------------------------
  // MARKER - WHITE
  //-------------------------------------------------------------------------

  matrix.setPixelColor(
    xy(localMarker, 2),
    matrix.Color(255, 255, 255)
  );

  matrix.show();

  xSemaphoreGive(displayMutex);
}

//=============================================================================
// Buzzer
//=============================================================================

void beep(
  unsigned int frequency,
  unsigned long duration
)
{
  if (xSemaphoreTake(
        buzzerMutex,
        pdMS_TO_TICKS(100)
      ) != pdTRUE)
  {
    return;
  }

  tone(
    BUZZER_PIN,
    frequency,
    duration
  );

  vTaskDelay(
    pdMS_TO_TICKS(duration)
  );

  noTone(BUZZER_PIN);

  xSemaphoreGive(buzzerMutex);
}

//=============================================================================
// Reset Game
//=============================================================================

void resetGame()
{
  //-------------------------------------------------------------------------
  // Reset all game variables atomically.
  //-------------------------------------------------------------------------

  if (xSemaphoreTake(gameMutex, portMAX_DELAY) == pdTRUE)
  {
    marker = 2;

    cpuInterval = INITIAL_CPU_INTERVAL;

    holdingPlayerSide = false;

    playerHoldStart = 0;

    lastCPU = millis();

    gameState = GAME_RUNNING;

    xSemaphoreGive(gameMutex);
  }

  //-------------------------------------------------------------------------
  // Draw starting position.
  //-------------------------------------------------------------------------

  draw();

  Serial.println();
  Serial.println("==============================");
  Serial.println("GAME RESET");
  Serial.println("==============================");
  Serial.println("Tug of War started.");
}

//=============================================================================
// End Game Animation
//=============================================================================

void endGameAnimation(bool playerWon)
{
  uint32_t color;

  if (playerWon)
  {
    color = matrix.Color(0, 255, 0);
  }
  else
  {
    color = matrix.Color(255, 0, 0);
  }

  //-------------------------------------------------------------------------
  // Flash result three times.
  //-------------------------------------------------------------------------

  for (int i = 0; i < 3; i++)
  {
    // Protect NeoPixel access.
    if (xSemaphoreTake(
          displayMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      for (int j = 0; j < NUM_PIXELS; j++)
      {
        matrix.setPixelColor(j, color);
      }

      matrix.show();

      xSemaphoreGive(displayMutex);
    }

    //-------------------------------------------------------------------------
    // Win/loss tone
    //-------------------------------------------------------------------------

    if (playerWon)
    {
      beep(1200, 120);
    }
    else
    {
      beep(150, 120);
    }

    //-------------------------------------------------------------------------
    // LEDs OFF
    //-------------------------------------------------------------------------

    if (xSemaphoreTake(
          displayMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      matrix.clear();
      matrix.show();

      xSemaphoreGive(displayMutex);
    }

    vTaskDelay(
      pdMS_TO_TICKS(100)
    );
  }

  //-------------------------------------------------------------------------
  // Short pause before restarting.
  //-------------------------------------------------------------------------

  vTaskDelay(
    pdMS_TO_TICKS(500)
  );
}

//=============================================================================
// Player Pull
//=============================================================================

void playerPull()
{
  bool successfulPull = false;
  bool startedHold = false;

  if (xSemaphoreTake(
        gameMutex,
        portMAX_DELAY
      ) != pdTRUE)
  {
    return;
  }

  //-------------------------------------------------------------------------
  // Ignore button if game isn't running.
  //-------------------------------------------------------------------------

  if (gameState != GAME_RUNNING)
  {
    xSemaphoreGive(gameMutex);
    return;
  }

  //-------------------------------------------------------------------------
  // Pull marker toward player.
  //-------------------------------------------------------------------------

  if (marker > 0)
  {
    marker--;

    successfulPull = true;

    //-------------------------------------------------------------------------
    // Speed up CPU.
    //-------------------------------------------------------------------------

    if (cpuInterval > MIN_CPU_INTERVAL)
    {
      if (cpuInterval > CPU_SPEEDUP)
      {
        cpuInterval -= CPU_SPEEDUP;
      }

      if (cpuInterval < MIN_CPU_INTERVAL)
      {
        cpuInterval = MIN_CPU_INTERVAL;
      }
    }
  }

  //-------------------------------------------------------------------------
  // Reached player side.
  //-------------------------------------------------------------------------

  if (marker <= 0)
  {
    marker = 0;

    if (!holdingPlayerSide)
    {
      holdingPlayerSide = true;

      playerHoldStart = millis();

      startedHold = true;
    }
  }

  xSemaphoreGive(gameMutex);

  //-------------------------------------------------------------------------
  // Player pull sound.
  //-------------------------------------------------------------------------

  if (successfulPull)
  {
    beep(900, 30);
  }

  //-------------------------------------------------------------------------
  // Hold-start sound.
  //-------------------------------------------------------------------------

  if (startedHold)
  {
    Serial.println(
      "PLAYER SIDE! Hold for 3 seconds!"
    );

    beep(1100, 50);
  }
}

//=============================================================================
// PLAYER TASK
//=============================================================================

void PlayerTask(void *parameter)
{
  bool oldButton = HIGH;

  while (true)
  {
    bool button = digitalRead(BUTTON_PIN);

    //-------------------------------------------------------------------------
    // Detect button press.
    //-------------------------------------------------------------------------

    if (button == LOW &&
        oldButton == HIGH)
    {
      playerPull();
    }

    oldButton = button;

    //-------------------------------------------------------------------------
    // Run every 10 ms.
    //-------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}

//=============================================================================
// CPU TASK
//=============================================================================

void CpuTask(void *parameter)
{
  while (true)
  {
    unsigned long currentTime;
    unsigned long interval;
    bool gameRunning;

    //-------------------------------------------------------------------------
    // Read current game state and CPU interval.
    //-------------------------------------------------------------------------

    if (xSemaphoreTake(
          gameMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      currentTime = millis();

      interval = cpuInterval;

      gameRunning =
        (gameState == GAME_RUNNING);

      xSemaphoreGive(gameMutex);
    }

    //-------------------------------------------------------------------------
    // If the game is not running, wait briefly and check again.
    //-------------------------------------------------------------------------

    if (!gameRunning)
    {
      vTaskDelay(
        pdMS_TO_TICKS(10)
      );

      continue;
    }

    //-------------------------------------------------------------------------
    // Check whether it is time for the CPU to pull.
    //-------------------------------------------------------------------------

    if (currentTime - lastCPU >= interval)
    {
      bool brokeHold = false;

      if (xSemaphoreTake(
            gameMutex,
            portMAX_DELAY
          ) == pdTRUE)
      {
        // Re-check state after obtaining mutex.
        if (gameState == GAME_RUNNING)
        {
          marker++;

          lastCPU = millis();

          //-------------------------------------------------------------------------
          // CPU broke player's hold.
          //-------------------------------------------------------------------------

          if (marker > 1 &&
              holdingPlayerSide)
          {
            holdingPlayerSide = false;

            playerHoldStart = 0;

            brokeHold = true;
          }

          //-------------------------------------------------------------------------
          // CPU reaches position 4.
          //-------------------------------------------------------------------------

          if (marker >= 4)
          {
            marker = 4;

            gameState = CPU_WON;
          }
        }

        xSemaphoreGive(gameMutex);
      }

      if (brokeHold)
      {
        Serial.println(
          "CPU broke the hold!"
        );
      }
    }

    //-------------------------------------------------------------------------
    // Check frequently so CPU timing remains accurate.
    //-------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(5)
    );
  }
}

//=============================================================================
// GAME TASK
//=============================================================================

void GameTask(void *parameter)
{
  while (true)
  {
    bool playerWon = false;
    bool cpuWon = false;

    //-------------------------------------------------------------------------
    // Check game state.
    //-------------------------------------------------------------------------

    if (xSemaphoreTake(
          gameMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      //=======================================================================
      // PLAYER HOLD CHECK
      //=======================================================================

      if (gameState == GAME_RUNNING &&
          holdingPlayerSide)
      {
        //-------------------------------------------------------------------------
        // CPU moved marker away.
        //-------------------------------------------------------------------------

        if (marker > 1)
        {
          holdingPlayerSide = false;

          playerHoldStart = 0;

          Serial.println(
            "Hold interrupted!"
          );
        }

        //-------------------------------------------------------------------------
        // Player held long enough.
        //-------------------------------------------------------------------------

        else if (
          millis() - playerHoldStart >=
          PLAYER_HOLD_TIME
        )
        {
          gameState = PLAYER_WON;

          playerWon = true;
        }
      }

      //=======================================================================
      // CPU WIN CHECK
      //=======================================================================

      if (gameState == CPU_WON)
      {
        cpuWon = true;
      }

      xSemaphoreGive(gameMutex);
    }

    //-------------------------------------------------------------------------
    // PLAYER WINS
    //-------------------------------------------------------------------------

    if (playerWon)
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("PLAYER WINS!");
      Serial.println("==============================");

      //-------------------------------------------------------------------------
      // Mark game as resetting so other tasks stop modifying it.
      //-------------------------------------------------------------------------

      if (xSemaphoreTake(
            gameMutex,
            portMAX_DELAY
          ) == pdTRUE)
      {
        gameState = GAME_RESETTING;

        xSemaphoreGive(gameMutex);
      }

      //-------------------------------------------------------------------------
      // Run animation.
      //-------------------------------------------------------------------------

      endGameAnimation(true);

      //-------------------------------------------------------------------------
      // Restart.
      //-------------------------------------------------------------------------

      resetGame();
    }

    //-------------------------------------------------------------------------
    // CPU WINS
    //-------------------------------------------------------------------------

    if (cpuWon)
    {
      Serial.println();
      Serial.println("==============================");
      Serial.println("CPU WINS!");
      Serial.println("==============================");

      //-------------------------------------------------------------------------
      // Mark game as resetting.
      //-------------------------------------------------------------------------

      if (xSemaphoreTake(
            gameMutex,
            portMAX_DELAY
          ) == pdTRUE)
      {
        gameState = GAME_RESETTING;

        xSemaphoreGive(gameMutex);
      }

      //-------------------------------------------------------------------------
      // Run animation.
      //-------------------------------------------------------------------------

      endGameAnimation(false);

      //-------------------------------------------------------------------------
      // Restart.
      //-------------------------------------------------------------------------

      resetGame();
    }

    //-------------------------------------------------------------------------
    // Game logic checks every 10 ms.
    //-------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}

//=============================================================================
// DISPLAY TASK
//=============================================================================

void DisplayTask(void *parameter)
{
  while (true)
  {
    bool running;

    //-------------------------------------------------------------------------
    // Only draw the normal game display while the game is running.
    //
    // This is important because endGameAnimation() owns the display
    // during the result animation.
    //-------------------------------------------------------------------------

    if (xSemaphoreTake(
          gameMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      running =
        (gameState == GAME_RUNNING);

      xSemaphoreGive(gameMutex);
    }

    if (running)
    {
      draw();
    }

    //-------------------------------------------------------------------------
    // Refresh display approximately 50 times per second.
    //-------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(20)
    );
  }
}

//=============================================================================
// SETUP
//=============================================================================

void setup()
{
  Serial.begin(115200);

  //-------------------------------------------------------------------------
  // Hardware
  //-------------------------------------------------------------------------

  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  //-------------------------------------------------------------------------
  // NeoPixels
  //-------------------------------------------------------------------------

  matrix.begin();

  matrix.setBrightness(10);

  matrix.clear();

  matrix.show();

  //-------------------------------------------------------------------------
  // Create mutexes.
  //-------------------------------------------------------------------------

  gameMutex = xSemaphoreCreateMutex();

  displayMutex = xSemaphoreCreateMutex();

  buzzerMutex = xSemaphoreCreateMutex();

  if (gameMutex == NULL ||
      displayMutex == NULL ||
      buzzerMutex == NULL)
  {
    Serial.println(
      "ERROR: Could not create FreeRTOS mutex!"
    );

    while (true)
    {
      delay(1000);
    }
  }

  //-------------------------------------------------------------------------
  // Initialize game.
  //-------------------------------------------------------------------------

  resetGame();

  Serial.println(
    "FreeRTOS Tug of War ready."
  );

  //-------------------------------------------------------------------------
  // PLAYER TASK
  //-------------------------------------------------------------------------

  xTaskCreate(
    PlayerTask,
    "Player Task",
    4096,
    NULL,
    2,
    &playerTaskHandle
  );

  //-------------------------------------------------------------------------
  // CPU TASK
  //-------------------------------------------------------------------------

  xTaskCreate(
    CpuTask,
    "CPU Task",
    4096,
    NULL,
    2,
    &cpuTaskHandle
  );

  //-------------------------------------------------------------------------
  // GAME TASK
  //-------------------------------------------------------------------------

  xTaskCreate(
    GameTask,
    "Game Task",
    4096,
    NULL,
    3,
    &gameTaskHandle
  );

  //-------------------------------------------------------------------------
  // DISPLAY TASK
  //-------------------------------------------------------------------------

  xTaskCreate(
    DisplayTask,
    "Display Task",
    4096,
    NULL,
    1,
    &displayTaskHandle
  );
}

//=============================================================================
// LOOP
//=============================================================================

void loop()
{
  /*
   * All game functionality is handled by FreeRTOS tasks.
   *
   * Keep Arduino's loop task asleep.
   */

  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
