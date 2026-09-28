/*
 * =============================================================================
 * 5x5 PONG AGAINST AI
 * =============================================================================
 * A miniature single-player Pong game.
 *
 * PLAYER:
 *   Tilt left/right to move the paddle on the bottom row.
 *
 * IMU CONFIGURATION:
 *   Physical Y axis -> horizontal paddle movement
 *   Y axis is NOT inverted
 *   Tilt threshold = approximately 15 degrees
 *
 * CPU:
 *   A simple AI follows the ball horizontally.
 *
 * BUTTON:
 *   Restart the game after losing.
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

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

#define TILT_THRESHOLD 4240

#define NUM_PIXELS 25

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game Variables
//=============================================================================

int ballX = 2;
int ballY = 3;

int ballDX = 1;
int ballDY = -1;

int playerX = 2;
int cpuX = 2;

unsigned long lastMove = 0;

bool gameOver = false;

// Prevents one continuous tilt from moving the paddle repeatedly.
bool tiltReady = true;

//=============================================================================
// Matrix
//=============================================================================

int xy(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void pixel(int x, int y, uint32_t color)
{
  if (x >= 0 && x < 5 &&
      y >= 0 && y < 5)
  {
    matrix.setPixelColor(
      xy(x, y),
      color
    );
  }
}

void draw()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Ball
  //------------------------------------------------------------------------

  pixel(
    ballX,
    ballY,
    matrix.Color(255, 255, 255)
  );

  //------------------------------------------------------------------------
  // CPU paddle
  //------------------------------------------------------------------------

  pixel(
    cpuX,
    0,
    matrix.Color(255, 0, 0)
  );

  if (cpuX > 0)
  {
    pixel(
      cpuX - 1,
      0,
      matrix.Color(255, 0, 0)
    );
  }

  if (cpuX < 4)
  {
    pixel(
      cpuX + 1,
      0,
      matrix.Color(255, 0, 0)
    );
  }

  //------------------------------------------------------------------------
  // Player paddle
  //------------------------------------------------------------------------

  pixel(
    playerX,
    4,
    matrix.Color(0, 255, 0)
  );

  if (playerX > 0)
  {
    pixel(
      playerX - 1,
      4,
      matrix.Color(0, 255, 0)
    );
  }

  if (playerX < 4)
  {
    pixel(
      playerX + 1,
      4,
      matrix.Color(0, 255, 0)
    );
  }

  matrix.show();
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

  byte low = SPI.transfer(0);
  byte high = SPI.transfer(0);

  digitalWrite(IMU_CS, HIGH);

  return (int16_t)((high << 8) | low);
}

void setupIMU()
{
  pinMode(IMU_CS, OUTPUT);

  digitalWrite(IMU_CS, HIGH);

  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );

  // LSM6DSM accelerometer:
  // 104 Hz, ±2g
  imuWrite(0x10, 0x40);
}

//=============================================================================
// Tilt Control
//=============================================================================

void readTilt()
{
  int16_t ay = imuRead16(0x2A);

  // Below threshold = neutral.
  if (abs(ay) < TILT_THRESHOLD)
  {
    tiltReady = true;
    return;
  }

  if (!tiltReady)
    return;

  // Positive Y -> right.
  if (ay > 0 && playerX < 4)
  {
    playerX++;
  }

  // Negative Y -> left.
  else if (ay < 0 && playerX > 0)
  {
    playerX--;
  }

  tiltReady = false;
}

//=============================================================================
// Reset
//=============================================================================

void resetGame()
{
  ballX = 2;
  ballY = 3;

  ballDX = random(0, 2) ? 1 : -1;
  ballDY = -1;

  playerX = 2;
  cpuX = 2;

  gameOver = false;

  tiltReady = true;

  draw();
}

//=============================================================================
// Game Over
//=============================================================================

void lose()
{
  gameOver = true;

  for (int i = 0; i < 3; i++)
  {
    matrix.clear();
    matrix.show();

    delay(100);

    for (int j = 0; j < NUM_PIXELS; j++)
    {
      matrix.setPixelColor(
        j,
        matrix.Color(255, 0, 0)
      );
    }

    matrix.show();

    delay(100);
  }

  tone(
    BUZZER_PIN,
    150,
    500
  );
}

//=============================================================================
// Ball Update
//=============================================================================

void updateBall()
{
  int nextX = ballX + ballDX;
  int nextY = ballY + ballDY;

  //------------------------------------------------------------------------
  // Side walls
  //------------------------------------------------------------------------

  if (nextX < 0 || nextX > 4)
  {
    ballDX = -ballDX;

    nextX = ballX + ballDX;
  }

  //------------------------------------------------------------------------
  // CPU paddle
  //------------------------------------------------------------------------

  if (nextY == 0)
  {
    if (abs(nextX - cpuX) <= 1)
    {
      ballDY = 1;

      tone(
        BUZZER_PIN,
        800,
        40
      );

      nextY = 1;
    }
    else
    {
      lose();
      return;
    }
  }

  //------------------------------------------------------------------------
  // Player paddle
  //------------------------------------------------------------------------

  if (nextY == 4)
  {
    if (abs(nextX - playerX) <= 1)
    {
      ballDY = -1;

      tone(
        BUZZER_PIN,
        1000,
        40
      );

      nextY = 3;
    }
    else
    {
      lose();
      return;
    }
  }

  ballX = nextX;
  ballY = nextY;

  //------------------------------------------------------------------------
  // CPU follows the ball.
  //------------------------------------------------------------------------

  if (cpuX < ballX)
    cpuX++;

  else if (cpuX > ballX)
    cpuX--;
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

  matrix.begin();

  matrix.setBrightness(10);

  setupIMU();

  randomSeed(
    (uint32_t)esp_random()
  );

  resetGame();
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
      delay(250);

      resetGame();
    }

    return;
  }

  readTilt();

  if (millis() - lastMove >= 300)
  {
    lastMove = millis();

    updateBall();

    draw();
  }

  delay(20);
}
