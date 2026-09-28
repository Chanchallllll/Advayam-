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
 *   Entire display is rotated 180 degrees.
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

#define TILT_THRESHOLD 4240

#define PLAYER_MOVE_DELAY 180

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

unsigned long lastPlayerMove = 0;
unsigned long lastBulletMove = 0;
unsigned long lastEnemyMove = 0;

//=============================================================================
// Matrix Coordinate Conversion
//=============================================================================

int xyToIndex(
  int x,
  int y
)
{
  // Rotate game coordinates 180 degrees.
  int displayX = 4 - x;
  int displayY = 4 - y;

  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 +
         (4 - displayX);
}

void setPixel(
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
  // Player
  //------------------------------------------------------------------------

  setPixel(
    playerX,
    4,
    matrix.Color(
      0,
      255,
      0
    )
  );

  //------------------------------------------------------------------------
  // Enemy
  //------------------------------------------------------------------------

  if (enemyY >= 0 &&
      enemyY <= 4)
  {
    setPixel(
      enemyX,
      enemyY,
      matrix.Color(
        255,
        0,
        0
      )
    );
  }

  //------------------------------------------------------------------------
  // Bullet
  //------------------------------------------------------------------------

  if (bulletY >= 0 &&
      bulletY <= 4)
  {
    setPixel(
      bulletX,
      bulletY,
      matrix.Color(
        255,
        255,
        255
      )
    );
  }

  matrix.show();
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

  byte lowByte =
    SPI.transfer(0);

  byte highByte =
    SPI.transfer(0);

  digitalWrite(
    IMU_CS,
    HIGH
  );

  return (int16_t)(
    (highByte << 8) | lowByte
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
  // 104 Hz
  // ±2g
  imuWrite(
    0x10,
    0x40
  );
}

//=============================================================================
// Player Tilt
//=============================================================================

void updatePlayer()
{
  int16_t ay =
    imuRead16(0x2A);

  //------------------------------------------------------------------------
  // Ignore small tilts.
  //------------------------------------------------------------------------

  if (abs(ay) < TILT_THRESHOLD)
    return;

  //------------------------------------------------------------------------
  // Limit movement rate.
  //------------------------------------------------------------------------

  if (millis() - lastPlayerMove <
      PLAYER_MOVE_DELAY)
  {
    return;
  }

  lastPlayerMove = millis();

  /*
   * Positive Y -> RIGHT
   * Negative Y -> LEFT
   */

  if (ay > 0)
  {
    if (playerX < 4)
    {
      playerX++;

      tone(
        BUZZER_PIN,
        700,
        25
      );
    }
  }
  else
  {
    if (playerX > 0)
    {
      playerX--;

      tone(
        BUZZER_PIN,
        700,
        25
      );
    }
  }
}

//=============================================================================
// Fire Bullet
//=============================================================================

void fireBullet()
{
  // Only one bullet at a time.
  if (bulletY != -1)
    return;

  bulletX = playerX;

  bulletY = 3;

  tone(
    BUZZER_PIN,
    1500,
    50
  );
}

//=============================================================================
// Bullet Collision
//=============================================================================

bool bulletHitsEnemy(
  int oldY,
  int newY
)
{
  if (bulletX != enemyX)
    return false;

  // Check whether the bullet crossed the enemy.
  if (oldY >= enemyY &&
      newY <= enemyY)
  {
    return true;
  }

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

  //------------------------------------------------------------------------
  // Collision.
  //------------------------------------------------------------------------

  if (bulletHitsEnemy(
        oldY,
        bulletY))
  {
    tone(
      BUZZER_PIN,
      2000,
      100
    );

    bulletX = -1;
    bulletY = -1;

    // Respawn enemy.
    enemyX =
      random(0, 5);

    enemyY = 0;

    return;
  }

  //------------------------------------------------------------------------
  // Bullet left display.
  //------------------------------------------------------------------------

  if (bulletY < 0)
  {
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

  if (enemyY >= 4)
  {
    if (enemyX == playerX)
    {
      gameOver = true;

      matrix.clear();

      // Flash entire matrix red.
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
        500
      );

      delay(1000);

      return;
    }

    // Enemy missed player.
    enemyX =
      random(0, 5);

    enemyY = 0;
  }
}

//=============================================================================
// Reset Game
//=============================================================================

void resetGame()
{
  playerX = 2;

  enemyX =
    random(0, 5);

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

  Serial.println(
    "Space Invaders started."
  );
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //==========================================================================
  // Game Over
  //==========================================================================

  if (gameOver)
  {
    if (digitalRead(BUTTON_PIN) == LOW)
    {
      delay(200);

      // Wait for button release.
      while (
        digitalRead(BUTTON_PIN) == LOW
      )
      {
        delay(10);
      }

      resetGame();
    }

    return;
  }

  //==========================================================================
  // Player
  //==========================================================================

  updatePlayer();

  //==========================================================================
  // Button / Fire
  //==========================================================================

  static bool previousButton = HIGH;

  bool buttonState =
    digitalRead(BUTTON_PIN);

  if (buttonState == LOW &&
      previousButton == HIGH)
  {
    fireBullet();
  }

  previousButton =
    buttonState;

  //==========================================================================
  // Bullet
  //==========================================================================

  if (millis() - lastBulletMove >=
      BULLET_MOVE_DELAY)
  {
    lastBulletMove = millis();

    updateBullet();
  }

  //==========================================================================
  // Enemy
  //==========================================================================

  if (millis() - lastEnemyMove >=
      ENEMY_MOVE_DELAY)
  {
    lastEnemyMove = millis();

    updateEnemy();
  }

  //==========================================================================
  // Display
  //==========================================================================

  drawGame();

  delay(10);
}
