/*
 * =============================================================================
 * CATCH THE FALLING FRUIT
 * =============================================================================
 * Green fruit falls from the top of the matrix.
 *
 * Tilt left/right controls the basket along the bottom row.
 *
 * IMU CONFIGURATION:
 *
 *   Physical Y axis controls horizontal movement.
 *
 *   +Y -> LEFT
 *   -Y -> RIGHT
 *
 *   Tilt below approximately 15 degrees is ignored.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Matrix = GPIO 7
 * Buzzer = GPIO 16
 * IMU    = LSM6DSM SPI
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

#define NUM_PIXELS 25

// Approximately 15 degrees with the ±2g accelerometer setting.
#define TILT_THRESHOLD 4240

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game Variables
//=============================================================================

int basketX = 2;

int fruitX;
int fruitY;

int score = 0;
int lives = 3;

unsigned long lastFall = 0;

// Controls how quickly the basket repeats movement while held tilted.
unsigned long lastTiltMove = 0;
const unsigned long TILT_MOVE_DELAY = 180;

//=============================================================================
// Matrix
//=============================================================================

/*
 * Rotate the entire display 180 degrees.
 */

int xy(int x, int y)
{
  int displayX = 4 - x;
  int displayY = 4 - y;

  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 + (4 - displayX);
}

void pixel(int x, int y, uint32_t color)
{
  if (x < 0 || x > 4 ||
      y < 0 || y > 4)
    return;

  matrix.setPixelColor(
    xy(x, y),
    color
  );
}

void draw()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Basket.
  //------------------------------------------------------------------------

  pixel(
    basketX,
    4,
    matrix.Color(0, 0, 255)
  );

  //------------------------------------------------------------------------
  // Falling fruit.
  //------------------------------------------------------------------------

  if (fruitY >= 0 && fruitY <= 4) {

    pixel(
      fruitX,
      fruitY,
      matrix.Color(0, 255, 0)
    );
  }

  matrix.show();
}

//=============================================================================
// IMU
//=============================================================================

void imuWrite(byte r, byte v)
{
  digitalWrite(
    IMU_CS,
    LOW
  );

  SPI.transfer(
    r & 0x7F
  );

  SPI.transfer(v);

  digitalWrite(
    IMU_CS,
    HIGH
  );
}

int16_t imu16(byte r)
{
  digitalWrite(
    IMU_CS,
    LOW
  );

  SPI.transfer(
    r | 0x80
  );

  byte l = SPI.transfer(0);
  byte h = SPI.transfer(0);

  digitalWrite(
    IMU_CS,
    HIGH
  );

  return (int16_t)(
    (h << 8) | l
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
  // Normal operating mode.
  imuWrite(
    0x10,
    0x40
  );
}

//=============================================================================
// Basket Controls
//=============================================================================

void readTilt()
{
  int16_t ay = imu16(0x2A);

  // Ignore small tilts.
  if (abs(ay) < TILT_THRESHOLD)
    return;

  // Only move the basket at controlled intervals.
  if (millis() - lastTiltMove < TILT_MOVE_DELAY)
    return;

  lastTiltMove = millis();

  //===========================================================================
  // Y AXIS IS INVERTED
  //
  // +Y -> LEFT
  // -Y -> RIGHT
  //===========================================================================

  if (ay > TILT_THRESHOLD) {

    if (basketX > 0)
      basketX--;
  }

  else if (ay < -TILT_THRESHOLD) {

    if (basketX < 4)
      basketX++;
  }
}

//=============================================================================
// Fruit
//=============================================================================

void newFruit()
{
  fruitX = random(0, 5);
  fruitY = 0;
}

//=============================================================================
// Game Over
//=============================================================================

void gameOver()
{
  Serial.println("GAME OVER");

  for (int i = 0; i < NUM_PIXELS; i++) {

    matrix.setPixelColor(
      i,
      matrix.Color(255, 0, 0)
    );
  }

  matrix.show();

  tone(
    BUZZER_PIN,
    150,
    500
  );

  delay(1000);

  score = 0;
  lives = 3;

  basketX = 2;

  newFruit();

  lastFall = millis();
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  matrix.begin();

  matrix.setBrightness(10);

  matrix.clear();
  matrix.show();

  setupIMU();

  randomSeed(
    (uint32_t)esp_random()
  );

  newFruit();

  lastFall = millis();

  Serial.println(
    "Catch the Falling Fruit started."
  );
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //------------------------------------------------------------------------
  // Read tilt.
  //------------------------------------------------------------------------

  readTilt();

  //------------------------------------------------------------------------
  // Move fruit.
  //------------------------------------------------------------------------

  if (millis() - lastFall >= 500) {

    lastFall = millis();

    fruitY++;

    // Fruit has reached the basket row.
    if (fruitY >= 4) {

      if (fruitX == basketX) {

        // Successfully caught.
        score++;

        tone(
          BUZZER_PIN,
          1200,
          80
        );

        Serial.print("Score: ");
        Serial.println(score);
      }

      else {

        // Missed.
        lives--;

        tone(
          BUZZER_PIN,
          200,
          150
        );

        Serial.print("Lives: ");
        Serial.println(lives);
      }

      // Start a new fruit.
      newFruit();

      // Check for game over.
      if (lives <= 0) {
        gameOver();
      }
    }
  }

  //------------------------------------------------------------------------
  // Draw.
  //------------------------------------------------------------------------

  draw();

  delay(20);
}
