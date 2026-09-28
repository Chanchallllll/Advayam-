/*
 * =============================================================================
 * 5x5 MATRIX SNAKE GAME
 * =============================================================================
 *
 * Classic Snake running on a 5x5 WS2812B matrix.
 *
 * CONTROLS:
 *   Tilt left/right    -> move horizontally
 *   Tilt forward/back  -> move vertically
 *
 * AXIS MAPPING:
 *   Physical Y -> Horizontal
 *   Physical Y -> NOT INVERTED
 *
 *   Physical X -> Vertical
 *   Physical X -> INVERTED
 *
 * TILT THRESHOLD:
 *   Approximately 30 degrees.
 *
 * EDGES:
 *   The screen wraps around.
 *
 * BUTTON:
 *   Press to restart after game over.
 *
 * Hardware:
 *   Matrix DATA = GPIO 7
 *   Button      = GPIO 17
 *   Buzzer      = GPIO 16
 *   IMU CS      = GPIO 10
 *   IMU SCK     = GPIO 12
 *   IMU MOSI    = GPIO 11
 *   IMU MISO    = GPIO 13
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define NUM_PIXELS 25

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

#define TILT_THRESHOLD 4240

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Snake Variables
//=============================================================================

int snakeX[25];
int snakeY[25];

int snakeLength;

int appleX;
int appleY;

int dirX = 1;
int dirY = 0;

unsigned long lastMove = 0;

int moveDelay = 350;

bool gameOver = false;

//=============================================================================
// Matrix
//=============================================================================

int xyToIndex(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void pixel(
  int x,
  int y,
  uint32_t color
)
{
  if (x < 0 || x > 4 ||
      y < 0 || y > 4)
    return;

  matrix.setPixelColor(
    xyToIndex(x, y),
    color
  );
}

//=============================================================================
// Draw Game
//=============================================================================

void drawGame()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Apple
  //------------------------------------------------------------------------

  pixel(
    appleX,
    appleY,
    matrix.Color(255, 0, 0)
  );

  //------------------------------------------------------------------------
  // Snake
  //------------------------------------------------------------------------

  for (int i = 0;
       i < snakeLength;
       i++)
  {
    if (i == 0)
    {
      // Head.
      pixel(
        snakeX[i],
        snakeY[i],
        matrix.Color(
          237,
          187,
          69
        )
      );
    }
    else
    {
      // Body.
      pixel(
        snakeX[i],
        snakeY[i],
        matrix.Color(
          0,
          255,
          0
        )
      );
    }
  }

  matrix.show();
}

//=============================================================================
// Startup Animation
//=============================================================================

void startupFlash()
{
  // First flash.
  drawGame();

  tone(
    BUZZER_PIN,
    1200,
    60
  );

  delay(80);

  matrix.clear();

  matrix.show();

  delay(170);

  // Second flash.
  drawGame();

  tone(
    BUZZER_PIN,
    1200,
    60
  );

  delay(80);

  matrix.clear();

  matrix.show();

  delay(170);
}

//=============================================================================
// IMU
//=============================================================================

void imuWrite(
  byte reg,
  byte value
)
{
  digitalWrite(
    IMU_CS,
    LOW
  );

  SPI.transfer(
    reg & 0x7F
  );

  SPI.transfer(value);

  digitalWrite(
    IMU_CS,
    HIGH
  );
}

int16_t imuRead16(byte reg)
{
  digitalWrite(
    IMU_CS,
    LOW
  );

  SPI.transfer(
    reg | 0x80
  );

  byte low =
    SPI.transfer(0);

  byte high =
    SPI.transfer(0);

  digitalWrite(
    IMU_CS,
    HIGH
  );

  return (int16_t)(
    (high << 8) | low
  );
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
  // 104 Hz, ±2g.
  imuWrite(
    0x10,
    0x40
  );
}

void readAcceleration(
  int16_t &x,
  int16_t &y,
  int16_t &z
)
{
  x = imuRead16(0x28);
  y = imuRead16(0x2A);
  z = imuRead16(0x2C);
}

//=============================================================================
// Tilt Control
//=============================================================================

void readTilt()
{
  int16_t ax;
  int16_t ay;
  int16_t az;

  readAcceleration(
    ax,
    ay,
    az
  );

  //------------------------------------------------------------------------
  // Physical Y -> Horizontal
  // NOT inverted.
  //------------------------------------------------------------------------

  if (abs(ay) >= TILT_THRESHOLD &&
      abs(ay) > abs(ax))
  {
    dirY = 0;

    if (ay > 0 &&
        dirX != -1)
    {
      dirX = 1;
    }

    if (ay < 0 &&
        dirX != 1)
    {
      dirX = -1;
    }
  }

  //------------------------------------------------------------------------
  // Physical X -> Vertical
  // INVERTED.
  //------------------------------------------------------------------------

  else if (abs(ax) >= TILT_THRESHOLD)
  {
    dirX = 0;

    if (ax > 0 &&
        dirY != 1)
    {
      dirY = -1;
    }

    if (ax < 0 &&
        dirY != -1)
    {
      dirY = 1;
    }
  }
}

//=============================================================================
// Snake Collision
//=============================================================================

bool snakeContains(
  int x,
  int y
)
{
  for (int i = 0;
       i < snakeLength;
       i++)
  {
    if (snakeX[i] == x &&
        snakeY[i] == y)
    {
      return true;
    }
  }

  return false;
}

//=============================================================================
// Apple
//=============================================================================

void placeApple()
{
  do
  {
    appleX = random(0, 5);
    appleY = random(0, 5);

  } while (
    snakeContains(
      appleX,
      appleY
    )
  );
}

//=============================================================================
// Reset
//=============================================================================

void resetGame()
{
  snakeLength = 3;

  snakeX[0] = 2;
  snakeY[0] = 2;

  snakeX[1] = 1;
  snakeY[1] = 2;

  snakeX[2] = 0;
  snakeY[2] = 2;

  dirX = 1;
  dirY = 0;

  placeApple();

  gameOver = false;

  startupFlash();

  lastMove = millis();

  drawGame();
}

//=============================================================================
// Game Over
//=============================================================================

void showGameOver()
{
  matrix.clear();

  for (int i = 0;
       i < NUM_PIXELS;
       i++)
  {
    matrix.setPixelColor(
      i,
      matrix.Color(
        255,
        0,
        0
      )
    );
  }

  matrix.show();

  tone(
    BUZZER_PIN,
    150,
    400
  );
}

//=============================================================================
// Move Snake
//=============================================================================

void moveSnake()
{
  int newX =
    snakeX[0] + dirX;

  int newY =
    snakeY[0] + dirY;

  //------------------------------------------------------------------------
  // Wrap horizontally.
  //------------------------------------------------------------------------

  if (newX < 0)
    newX = 4;

  if (newX > 4)
    newX = 0;

  //------------------------------------------------------------------------
  // Wrap vertically.
  //------------------------------------------------------------------------

  if (newY < 0)
    newY = 4;

  if (newY > 4)
    newY = 0;

  //------------------------------------------------------------------------
  // Self collision.
  //------------------------------------------------------------------------

  if (snakeContains(
        newX,
        newY))
  {
    gameOver = true;

    showGameOver();

    return;
  }

  //------------------------------------------------------------------------
  // Apple.
  //------------------------------------------------------------------------

  bool ateApple =
    (newX == appleX &&
     newY == appleY);

  if (ateApple)
  {
    snakeLength++;

    tone(
      BUZZER_PIN,
      1200,
      80
    );

    if (snakeLength >= 25)
    {
      gameOver = true;

      return;
    }
  }

  //------------------------------------------------------------------------
  // Shift body.
  //------------------------------------------------------------------------

  for (int i = snakeLength - 1;
       i > 0;
       i--)
  {
    snakeX[i] =
      snakeX[i - 1];

    snakeY[i] =
      snakeY[i - 1];
  }

  snakeX[0] = newX;
  snakeY[0] = newY;

  if (ateApple)
    placeApple();

  drawGame();
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

  matrix.begin();

  matrix.setBrightness(10);

  setupIMU();

  randomSeed(
    (uint32_t)esp_random()
  );

  resetGame();

  Serial.println(
    "Snake started."
  );
}

//=============================================================================
// Loop
//=============================================================================

void loop()
{
  if (gameOver)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(200);

      resetGame();
    }

    return;
  }

  readTilt();

  if (millis() - lastMove >= moveDelay)
  {
    lastMove = millis();

    moveSnake();
  }

  delay(10);
}
