/*
 * =============================================================================
 * WHACK-A-MOLE - FreeRTOS
 * =============================================================================
 *
 * A random target appears on the 5x5 RGB matrix.
 *
 * PLAYER:
 *   Tilt the device to move the cursor.
 *   Press the button to whack the target.
 *
 * INPUT:
 *   Touch/button = GPIO 17
 *   Buzzer       = GPIO 16
 *   IMU          = LSM6DSM SPI
 *
 * IMU:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 *
 * MATRIX:
 *   GPIO 7
 *
 * IMU CONFIGURATION:
 *
 *   Physical Y axis -> horizontal movement.
 *   Physical X axis -> vertical movement.
 *
 *   Positive Y -> RIGHT
 *   Negative Y -> LEFT
 *
 *   Positive X -> DOWN
 *   Negative X -> UP
 *
 * The display is rotated 180 degrees.
 *
 * FreeRTOS tasks:
 *
 *   IMUTask
 *   ButtonTask
 *   GameTask
 *
 * A mutex protects shared game state.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

#define NUM_PIXELS 25

// Approximately 15 degrees for ±2g.
#define TILT_THRESHOLD 4240

#define CURSOR_MOVE_DELAY 180
#define TARGET_TIME 2500

#define BUTTON_SCAN_MS 10
#define GAME_SCAN_MS   10

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game State
//=============================================================================

int cursorX = 2;
int cursorY = 2;

int targetX = 0;
int targetY = 0;

unsigned long targetStart = 0;
unsigned long lastCursorMove = 0;

int score = 0;

SemaphoreHandle_t gameMutex;

//=============================================================================
// Matrix
//=============================================================================

int xyToIndex(int x, int y)
{
  // Rotate the game coordinates by 180 degrees.
  int displayX = 4 - x;
  int displayY = 4 - y;

  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 + (4 - displayX);
}

void setPixel(int x, int y, uint32_t color)
{
  if (x < 0 || x > 4 ||
      y < 0 || y > 4)
    return;

  matrix.setPixelColor(
    xyToIndex(x, y),
    color
  );
}

void draw()
{
  matrix.clear();

  setPixel(
    targetX,
    targetY,
    matrix.Color(255, 0, 0)
  );

  setPixel(
    cursorX,
    cursorY,
    matrix.Color(0, 255, 0)
  );

  matrix.show();
}

//=============================================================================
// New Target
//=============================================================================

void newTarget()
{
  targetX = random(0, 5);
  targetY = random(0, 5);

  targetStart = millis();
}

//=============================================================================
// IMU
//=============================================================================

void imuWrite(byte reg, byte value)
{
  digitalWrite(IMU_CS, LOW);

  SPI.transfer(reg & 0x7F);
  SPI.transfer(value);

  digitalWrite(IMU_CS, HIGH);
}

int16_t imuRead16(byte reg)
{
  digitalWrite(IMU_CS, LOW);

  SPI.transfer(reg | 0x80);

  byte lowByte = SPI.transfer(0);
  byte highByte = SPI.transfer(0);

  digitalWrite(IMU_CS, HIGH);

  return (int16_t)((highByte << 8) | lowByte);
}

void setupIMU()
{
  pinMode(
    IMU_CS,
    OUTPUT
  );

  digitalWrite(
    IMU_CS,
    HIGH
  );

  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );

  // LSM6DSM:
  // 104 Hz output data rate
  // ±2g accelerometer range
  // Normal operating mode
  imuWrite(
    0x10,
    0x40
  );
}

//=============================================================================
// Tilt Processing
//=============================================================================

void readTilt()
{
  int16_t ax = imuRead16(0x28);
  int16_t ay = imuRead16(0x2A);

  if (abs(ax) < TILT_THRESHOLD &&
      abs(ay) < TILT_THRESHOLD)
  {
    return;
  }

  if (millis() - lastCursorMove <
      CURSOR_MOVE_DELAY)
  {
    return;
  }

  lastCursorMove = millis();

  if (abs(ay) > abs(ax))
  {
    //===============================================================
    // Y axis controls horizontal movement.
    //===============================================================

    // Positive Y -> LEFT.
    if (ay < TILT_THRESHOLD)
    {
      if (cursorX < 4)
      {
        cursorX++;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }

    // Negative Y -> RIGHT.
    else if (ay > -TILT_THRESHOLD)
    {
      if (cursorX > 0)
      {
        cursorX--;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }
  }
  else
  {
    //===============================================================
    // X axis controls vertical movement.
    //===============================================================

    // Positive X -> DOWN.
    if (ax > TILT_THRESHOLD)
    {
      if (cursorY < 4)
      {
        cursorY++;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }

    // Negative X -> UP.
    else if (ax < -TILT_THRESHOLD)
    {
      if (cursorY > 0)
      {
        cursorY--;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }
  }
}

//=============================================================================
// IMU Task
//=============================================================================

void imuTask(void *parameter)
{
  while (true)
  {
    if (xSemaphoreTake(gameMutex, portMAX_DELAY) == pdTRUE)
    {
      readTilt();

      xSemaphoreGive(gameMutex);
    }

    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}

//=============================================================================
// Button Task
//=============================================================================

void buttonTask(void *parameter)
{
  bool oldButton = HIGH;

  while (true)
  {
    bool button = digitalRead(BUTTON_PIN);

    if (button == LOW &&
        oldButton == HIGH)
    {
      if (xSemaphoreTake(gameMutex, portMAX_DELAY) == pdTRUE)
      {
        if (cursorX == targetX &&
            cursorY == targetY)
        {
          score++;

          tone(
            BUZZER_PIN,
            1200,
            80
          );

          Serial.print("Score: ");
          Serial.println(score);

          newTarget();
        }
        else
        {
          tone(
            BUZZER_PIN,
            150,
            150
          );
        }

        xSemaphoreGive(gameMutex);
      }
    }

    oldButton = button;

    vTaskDelay(
      pdMS_TO_TICKS(BUTTON_SCAN_MS)
    );
  }
}

//=============================================================================
// Game Task
//=============================================================================

void gameTask(void *parameter)
{
  while (true)
  {
    if (xSemaphoreTake(gameMutex, portMAX_DELAY) == pdTRUE)
    {
      if (millis() - targetStart >= TARGET_TIME)
      {
        Serial.println("Miss!");

        tone(
          BUZZER_PIN,
          200,
          150
        );

        newTarget();
      }

      draw();

      xSemaphoreGive(gameMutex);
    }

    vTaskDelay(
      pdMS_TO_TICKS(GAME_SCAN_MS)
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
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  matrix.begin();
  matrix.setBrightness(10);
  matrix.clear();
  matrix.show();

  setupIMU();

  randomSeed(
    (uint32_t)esp_random()
  );

  gameMutex = xSemaphoreCreateMutex();

  newTarget();
  draw();

  Serial.println(
    "Whack-A-Mole started."
  );

  xTaskCreate(
    imuTask,
    "IMUTask",
    4096,
    NULL,
    2,
    NULL
  );

  xTaskCreate(
    buttonTask,
    "ButtonTask",
    4096,
    NULL,
    2,
    NULL
  );

  xTaskCreate(
    gameTask,
    "GameTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // Game operation is handled by the FreeRTOS tasks.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
