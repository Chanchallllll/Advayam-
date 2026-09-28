/*
 * =============================================================================
 * SPACE INVADERS MINI - UPSIDE DOWN DISPLAY
 * =============================================================================
 *
 * 5x5 Space Invaders game.
 *
 * CONTROLS:
 *   Tilt left/right -> move spaceship.
 *   Button          -> fire.
 *
 * IMU:
 *   Physical Y axis controls horizontal movement.
 *   Positive Y -> RIGHT
 *   Negative Y -> LEFT
 *   Tilt threshold = approximately 15 degrees.
 *
 * DISPLAY:
 *   The entire game is rotated 180 degrees.
 *   This makes the matrix appear correctly oriented when the physical
 *   display is mounted upside down.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Buzzer = GPIO 16
 *
 * IMU:
 *   CS   = GPIO 10
 *   SCK  = GPIO 12
 *   MOSI = GPIO 11
 *   MISO = GPIO 13
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// Pins
//=============================================================================

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

// How quickly the player moves while holding a tilt.
#define PLAYER_MOVE_DELAY 180

// Game speeds.
#define BULLET_MOVE_DELAY 100
#define ENEMY_MOVE_DELAY 700

//=============================================================================
// Hardware
//=============================================================================

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game State
//=============================================================================

int playerX = 2;

int enemyX = 2;
int enemyY = 0;

int bulletX = -1;
int bulletY = -1;

bool gameOver = false;

// Timing.
unsigned long lastPlayerMove = 0;
unsigned long lastBulletMove = 0;
unsigned long lastEnemyMove = 0;

//=============================================================================
// Matrix Coordinate System
//=============================================================================

/*
 * The game internally uses the normal coordinate system:
 *
 *       y = 0
 *     0 1 2 3 4
 *
 *       ...
 *
 *       y = 4
 *
 * We rotate the DISPLAY 180 degrees:
 *
 *     display X = 4 - game X
 *     display Y = 4 - game Y
 *
 * This flips the entire game upside down.
 */

int xyToIndex(int x, int y)
{
  // Rotate coordinates 180 degrees.
  int displayX = 4 - x;
  int displayY = 4 - y;

  // Convert rotated coordinates to the physical NeoPixel index.
  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 + (4 - displayX);
}

void setPixel(int x, int y, uint32_t color)
{
  if (x < 0 || x > 4 || y < 0 || y > 4)
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
  // Player
  //------------------------------------------------------------------------

  setPixel(
    playerX,
    4,
    matrix.Color(0, 255, 0)
  );

  //------------------------------------------------------------------------
  // Enemy
  //------------------------------------------------------------------------

  if (enemyY >= 0 && enemyY <= 4) {

    setPixel(
      enemyX,
      enemyY,
      matrix.Color(255, 0, 0)
    );
  }

  //------------------------------------------------------------------------
  // Bullet
  //------------------------------------------------------------------------

  if (bulletY >= 0 && bulletY <= 4) {

    setPixel(
      bulletX,
      bulletY,
      matrix.Color(255, 255, 255)
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

  byte lowByte = SPI.transfer(0);
  byte highByte = SPI.transfer(0);

  digitalWrite(IMU_CS, HIGH);

  return (int16_t)((highByte << 8) | lowByte);
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

  // LSM6DSM:
  // 104 Hz
  // ±2g
  // Normal operating mode
  imuWrite(0x10, 0x40);
}

//=============================================================================
// Player Tilt Control
//=============================================================================

void updatePlayer()
{
  int16_t ay = imuRead16(0x2A);

  //------------------------------------------------------------------------
  // Ignore small tilts.
  //------------------------------------------------------------------------

  if (abs(ay) < TILT_THRESHOLD)
    return;

  //------------------------------------------------------------------------
  // Control movement speed.
  //------------------------------------------------------------------------

  if (millis() - lastPlayerMove < PLAYER_MOVE_DELAY)
    return;

  lastPlayerMove = millis();

  /*
   * Tilt mapping:
   *
   * Positive Y -> LEFT
   * Negative Y -> RIGHT
   *
   * This is independent of the display rotation.
   */

  if (ay < TILT_THRESHOLD) {

    if (playerX < 4) {
      playerX++;

      tone(BUZZER_PIN, 700, 25);
    }
  }

  else if (ay > -TILT_THRESHOLD) {

    if (playerX > 0) {
      playerX--;

      tone(BUZZER_PIN, 700, 25);
    }
  }
}

//=============================================================================
// Fire
//=============================================================================

void fireBullet()
{
  // Only one bullet at a time.
  if (bulletY != -1)
    return;

  bulletX = playerX;
  bulletY = 3;

  tone(BUZZER_PIN, 1500, 50);
}

//=============================================================================
// Bullet Collision
//=============================================================================

bool bulletHitsEnemy(int oldY, int newY)
{
  // X must match.
  if (bulletX != enemyX)
    return false;

  /*
   * Check whether the bullet crossed the enemy during
   * this movement.
   *
   * Example:
   *
   * Bullet:
   *     Y = 3
   *      |
   *      |
   * Enemy Y = 2
   *      |
   *      |
   * Bullet:
   *     Y = 1
   *
   * The bullet crossed Y=2, so it counts as a hit.
   */

  if (oldY >= enemyY &&
      newY <= enemyY)
    return true;

  return false;
}

//=============================================================================
// Bullet Update
//=============================================================================

void updateBullet()
{
  if (bulletY == -1)
    return;

  int oldY = bulletY;

  // Move bullet upward.
  bulletY--;

  // Check collision along the movement path.
  if (bulletHitsEnemy(oldY, bulletY)) {

    tone(BUZZER_PIN, 2000, 100);

    // Remove bullet.
    bulletX = -1;
    bulletY = -1;

    // Respawn enemy.
    enemyX = random(0, 5);
    enemyY = 0;

    return;
  }

  // Bullet left the matrix.
  if (bulletY < 0) {

    bulletX = -1;
    bulletY = -1;
  }
}

//=============================================================================
// Enemy Update
//=============================================================================

void updateEnemy()
{
  enemyY++;

  //------------------------------------------------------------------------
  // Enemy reaches player's row.
  //------------------------------------------------------------------------

  if (enemyY >= 4) {

    if (enemyX == playerX) {

      gameOver = true;

      matrix.clear();

      // Flash the matrix red.
      for (int i = 0; i < NUM_PIXELS; i++) {

        matrix.setPixelColor(
          i,
          matrix.Color(255, 0, 0)
        );
      }

      matrix.show();

      tone(BUZZER_PIN, 150, 500);

      delay(1000);

      return;
    }

    // Enemy missed.
    enemyX = random(0, 5);
    enemyY = 0;
  }
}

//=============================================================================
// Reset
//=============================================================================

void resetGame()
{
  playerX = 2;

  enemyX = random(0, 5);
  enemyY = 0;

  bulletX = -1;
  bulletY = -1;

  gameOver = false;

  lastPlayerMove = millis();
  lastBulletMove = millis();
  lastEnemyMove = millis();

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

  resetGame();

  Serial.println("Space Invaders started.");
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //===========================================================================
  // Game Over
  //===========================================================================

  if (gameOver) {

    if (digitalRead(BUTTON_PIN) == LOW) {

      delay(200);

      while (digitalRead(BUTTON_PIN) == LOW)
        delay(10);

      resetGame();
    }

    return;
  }

  //===========================================================================
  // Player
  //===========================================================================

  updatePlayer();

  //===========================================================================
  // Button
  //===========================================================================

  static bool previousButton = HIGH;

  bool buttonState = digitalRead(BUTTON_PIN);

  if (buttonState == LOW &&
      previousButton == HIGH) {

    fireBullet();
  }

  previousButton = buttonState;

  //===========================================================================
  // Bullet
  //===========================================================================

  if (millis() - lastBulletMove >= BULLET_MOVE_DELAY) {

    lastBulletMove = millis();

    updateBullet();
  }

  //===========================================================================
  // Enemy
  //===========================================================================

  if (millis() - lastEnemyMove >= ENEMY_MOVE_DELAY) {

    lastEnemyMove = millis();

    updateEnemy();
  }

  //===========================================================================
  // Display
  //===========================================================================

  drawGame();

  delay(10);
}
