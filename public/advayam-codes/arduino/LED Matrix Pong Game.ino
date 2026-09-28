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
 *   Physical X axis is inverted (not used for horizontal paddle control)
 *   Tilt threshold = approximately 15 degrees
 *
 * CPU:
 *   A simple AI follows the ball horizontally.
 *
 * The ball bounces around the matrix and must be returned by the player's
 * paddle.
 *
 * Button:
 *   Restart the game after losing.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

// Approximately sin(15°) * 16384 for ±2g.
#define TILT_THRESHOLD 4240

Adafruit_NeoPixel matrix(25, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

int ballX = 2;
int ballY = 3;

int ballDX = 1;
int ballDY = -1;

int playerX = 2;
int cpuX = 2;

unsigned long lastMove = 0;
bool gameOver = false;

// Used so one tilt only moves the paddle one position.
bool tiltReady = true;

//=============================================================================
// Matrix
//=============================================================================

int xy(int x, int y)
{
  return (y % 2 == 0) ? y * 5 + x : y * 5 + (4 - x);
}

void p(int x, int y, uint32_t c)
{
  if (x >= 0 && x < 5 && y >= 0 && y < 5)
    matrix.setPixelColor(xy(x, y), c);
}

void draw()
{
  matrix.clear();

  // Ball.
  p(ballX, ballY, matrix.Color(255, 255, 255));

  // CPU paddle.
  p(cpuX, 0, matrix.Color(255, 0, 0));

  if (cpuX > 0)
    p(cpuX - 1, 0, matrix.Color(255, 0, 0));

  if (cpuX < 4)
    p(cpuX + 1, 0, matrix.Color(255, 0, 0));

  // Player paddle.
  p(playerX, 4, matrix.Color(0, 255, 0));

  if (playerX > 0)
    p(playerX - 1, 4, matrix.Color(0, 255, 0));

  if (playerX < 4)
    p(playerX + 1, 4, matrix.Color(0, 255, 0));

  matrix.show();
}

//=============================================================================
// IMU
//=============================================================================

void imuWrite(byte r, byte v)
{
  digitalWrite(IMU_CS, LOW);
  SPI.transfer(r & 0x7F);
  SPI.transfer(v);
  digitalWrite(IMU_CS, HIGH);
}

int16_t imu16(byte r)
{
  digitalWrite(IMU_CS, LOW);
  SPI.transfer(r | 0x80);

  byte l = SPI.transfer(0);
  byte h = SPI.transfer(0);

  digitalWrite(IMU_CS, HIGH);

  return (int16_t)((h << 8) | l);
}

void setupIMU()
{
  pinMode(IMU_CS, OUTPUT);
  digitalWrite(IMU_CS, HIGH);

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  // LSM6DSM accelerometer:
  // 104 Hz, ±2g, normal operating mode.
  imuWrite(0x10, 0x40);
}

void readTilt()
{
  int16_t ax = imu16(0x28);
  int16_t ay = imu16(0x2A);

  /*
   * Same configuration as the previous Snake game:
   *
   * Physical Y -> horizontal movement
   * Physical Y is NOT inverted.
   *
   * Physical X -> vertical movement
   * Physical X is inverted.
   *
   * For Pong, only the Y axis is needed because the paddle
   * moves horizontally.
   *
   * Tilt must be approximately 15 degrees or greater.
   */

  //------------------------------------------------------------------------
  // Tilt below 15 degrees: reset the control.
  //------------------------------------------------------------------------

  if (abs(ay) < TILT_THRESHOLD) {
    tiltReady = true;
    return;
  }

  //------------------------------------------------------------------------
  // Tilt is 15 degrees or greater.
  // Only register once until the board returns below the threshold.
  //------------------------------------------------------------------------

  if (tiltReady) {

    // Positive Y -> move right.
    if (ay > 0 && playerX < 4) {
      playerX++;
    }

    // Negative Y -> move left.
    else if (ay < 0 && playerX > 0) {
      playerX--;
    }

    tiltReady = false;
  }
}

//=============================================================================
// Game
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

  // Allow the first tilt immediately.
  tiltReady = true;

  draw();
}

void lose()
{
  gameOver = true;

  for (int i = 0; i < 3; i++) {
    matrix.clear();
    matrix.show();
    delay(100);

    for (int j = 0; j < 25; j++)
      matrix.setPixelColor(j, matrix.Color(255, 0, 0));

    matrix.show();
    delay(100);
  }

  tone(BUZZER_PIN, 150, 500);
}

void updateBall()
{
  int nextX = ballX + ballDX;
  int nextY = ballY + ballDY;

  // Side wall collision.
  if (nextX < 0 || nextX > 4) {
    ballDX = -ballDX;
    nextX = ballX + ballDX;
  }

  // CPU paddle.
  if (nextY == 0) {

    if (abs(nextX - cpuX) <= 1) {
      ballDY = 1;
      tone(BUZZER_PIN, 800, 40);
      nextY = 1;
    } else {
      lose();
      return;
    }
  }

  // Player paddle.
  if (nextY == 4) {

    if (abs(nextX - playerX) <= 1) {
      ballDY = -1;
      tone(BUZZER_PIN, 1000, 40);
      nextY = 3;
    } else {
      lose();
      return;
    }
  }

  ballX = nextX;
  ballY = nextY;

  // Basic CPU tracking.
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

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  matrix.begin();
  matrix.setBrightness(10);

  setupIMU();

  randomSeed((uint32_t)esp_random());

  resetGame();
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  if (gameOver) {

    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(250);
      resetGame();
    }

    return;
  }

  readTilt();

  if (millis() - lastMove >= 300) {
    lastMove = millis();
    updateBall();
    draw();
  }

  delay(20);
}
