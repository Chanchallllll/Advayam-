/*
 * =============================================================================
 * MATRIX SNAKE GAME
 * =============================================================================
 * Classic Snake running on the 5x5 WS2812B matrix.
 *
 * CONTROLS:
 *   Tilt left/right    -> move horizontally
 *   Tilt forward/back  -> move vertically
 *
 * AXIS MAPPING:
 *   Physical Y -> Horizontal (NOT INVERTED)
 *   Physical X -> Vertical   (INVERTED)
 *
 * TILT THRESHOLD:
 *   Tilt must be approximately 30 degrees or more to register.
 *   Smaller tilts are ignored.
 *
 * STARTUP:
 *   The snake and apple flash twice with a beep before the game starts.
 *
 * EDGES:
 *   The screen wraps around.
 *   Right edge -> snake appears on left.
 *   Left edge  -> snake appears on right.
 *   Top edge   -> snake appears on bottom.
 *   Bottom edge -> snake appears on top.
 *
 * BUTTON:
 *   Press to restart after game over.
 *
 * Hardware:
 *   Matrix DATA = GPIO 7
 *   Button      = GPIO 17
 *   IMU         = LSM6DSM SPI
 *   Buzzer      = GPIO 16
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

// Approximately sin(30°) * 16384 for ±2g.
#define TILT_THRESHOLD 4240


Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

// Snake can contain every cell except the apple.
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
// Matrix Helpers
//=============================================================================

int xyToIndex(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void pixel(int x, int y, uint32_t color)
{
  if (x < 0 || x > 4 || y < 0 || y > 4)
    return;

  matrix.setPixelColor(xyToIndex(x, y), color);
}

void drawGame()
{
  matrix.clear();

  // Apple is red.
  pixel(appleX, appleY, matrix.Color(255, 0, 0));

  // Snake.
  for (int i = 0; i < snakeLength; i++) {

    if (i == 0) {
      // Head: sort of yellow
      pixel(snakeX[i], snakeY[i], matrix.Color(237, 187, 69));
    }
    else {
      // Body: bright green
      pixel(snakeX[i], snakeY[i], matrix.Color(0, 255, 0));
    }
  }

  matrix.show();
}

//=============================================================================
// Startup Animation
//=============================================================================

void startupFlash()
{
  // First flash ON.
  drawGame();

  // Short ON time + longer OFF time.
  tone(BUZZER_PIN, 1200, 60);
  delay(80);

  // OFF.
  matrix.clear();
  matrix.show();
  delay(170);

  // Second flash ON.
  drawGame();

  tone(BUZZER_PIN, 1200, 60);
  delay(80);

  // OFF.
  matrix.clear();
  matrix.show();
  delay(170);
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

byte imuRead(byte reg)
{
  digitalWrite(IMU_CS, LOW);
  SPI.transfer(reg | 0x80);
  byte value = SPI.transfer(0);
  digitalWrite(IMU_CS, HIGH);
  return value;
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

  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  // LSM6DSM accelerometer:
  // 104 Hz, ±2g, normal operating mode.
  imuWrite(0x10, 0x40);
}

void readAcceleration(int16_t &x, int16_t &y, int16_t &z)
{
  x = imuRead16(0x28);
  y = imuRead16(0x2A);
  z = imuRead16(0x2C);
}

//=============================================================================
// Game Logic
//=============================================================================

void readTilt()
{
  int16_t ax, ay, az;
  readAcceleration(ax, ay, az);

  /*
   * AXIS MAPPING
   *
   * Physical Y -> Snake X (horizontal)
   * Physical X -> Snake Y (vertical)
   *
   * Physical Y = NOT inverted
   * Physical X = inverted
   *
   * Tilt must be >= approximately 30 degrees.
   */

  //------------------------------------------------------------------------
  // Physical Y -> Horizontal movement
  // NOT INVERTED
  //------------------------------------------------------------------------

  if (abs(ay) >= TILT_THRESHOLD &&
      abs(ay) > abs(ax)) {

    dirY = 0;

    if (ay > 0 && dirX != -1)
      dirX = 1;

    if (ay < 0 && dirX != 1)
      dirX = -1;
  }

  //------------------------------------------------------------------------
  // Physical X -> Vertical movement
  // INVERTED
  //------------------------------------------------------------------------

  else if (abs(ax) >= TILT_THRESHOLD) {

    dirX = 0;

    if (ax > 0 && dirY != 1)
      dirY = -1;

    if (ax < 0 && dirY != -1)
      dirY = 1;
  }
}

bool snakeContains(int x, int y)
{
  for (int i = 0; i < snakeLength; i++) {
    if (snakeX[i] == x && snakeY[i] == y)
      return true;
  }

  return false;
}

void placeApple()
{
  do {
    appleX = random(0, 5);
    appleY = random(0, 5);
  } while (snakeContains(appleX, appleY));
}

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

  // Show the startup animation before movement begins.
  startupFlash();

  lastMove = millis();

  drawGame();
}

void showGameOver()
{
  matrix.clear();

  for (int i = 0; i < NUM_PIXELS; i++)
    matrix.setPixelColor(i, matrix.Color(255, 0, 0));

  matrix.show();

  tone(BUZZER_PIN, 150, 400);
}

void moveSnake()
{
  int newX = snakeX[0] + dirX;
  int newY = snakeY[0] + dirY;

  //=========================================================================
  // WRAP AROUND EDGES
  //=========================================================================

  if (newX < 0)
    newX = 4;

  if (newX > 4)
    newX = 0;

  if (newY < 0)
    newY = 4;

  if (newY > 4)
    newY = 0;

  //=========================================================================
  // SELF COLLISION
  //=========================================================================

  if (snakeContains(newX, newY)) {
    gameOver = true;
    showGameOver();
    return;
  }

  //=========================================================================
  // APPLE
  //=========================================================================

  bool ateApple =
    (newX == appleX && newY == appleY);

  if (ateApple) {
    snakeLength++;

    tone(BUZZER_PIN, 1200, 80);

    if (snakeLength >= 25) {
      gameOver = true;
      return;
    }
  }

  // Shift the body backwards.
  for (int i = snakeLength - 1; i > 0; i--) {
    snakeX[i] = snakeX[i - 1];
    snakeY[i] = snakeY[i - 1];
  }

  snakeX[0] = newX;
  snakeY[0] = newY;

  if (ateApple)
    placeApple();

  drawGame();
}

void setup()
{
  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);

  matrix.begin();
  matrix.setBrightness(10);

  setupIMU();

  randomSeed((uint32_t)esp_random());

  resetGame();

  Serial.println("Snake started.");
}

void loop()
{
  if (gameOver) {

    // Button restarts the game.
    if (digitalRead(BUTTON_PIN) == LOW) {
      delay(200);
      resetGame();
    }

    return;
  }

  readTilt();

  if (millis() - lastMove >= moveDelay) {
    lastMove = millis();
    moveSnake();
  }

  delay(10);
}
