/*
 * =============================================================================
 * CATCH THE FALLING FRUIT
 * =============================================================================
 *
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
 * DIFFICULTY:
 *
 *   Every 5 successfully caught fruits:
 *
 *   - The falling fruit becomes slightly faster.
 *   - The basket becomes slightly faster to control.
 *
 *   The maximum speed is limited so the game remains
 *   physically possible to play.
 *
 * AUDIO:
 *
 *   - Short beep whenever the basket moves LEFT or RIGHT.
 *   - Higher-pitched beep when a fruit is caught.
 *   - Low-pitched beep when a fruit is missed.
 *
 * Hardware:
 *   Matrix -> GPIO 7
 *   Buzzer -> GPIO 16
 *   IMU    -> LSM6DSM SPI
 *
 * The complete game runs inside a dedicated FreeRTOS task.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define MATRIX_PIN 7
#define BUZZER_PIN 16

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

#define NUM_PIXELS 25

// Approximately 15 degrees with the ±2g accelerometer setting.
#define TILT_THRESHOLD 4240

// =============================================================================
// NeoPixel matrix
// =============================================================================

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Game variables
// =============================================================================

int basketX = 2;

int fruitX;
int fruitY;

int score = 0;
int lives = 3;

unsigned long lastFall = 0;

// =============================================================================
// Difficulty settings
// =============================================================================
//
// The fruit initially moves every 500 ms.
//
// Every 5 catches, the interval is reduced by 35 ms.
//
// The interval will never become shorter than 325 ms.
//
// This gives 5 difficulty increases before the maximum speed
// is reached.
//
// The basket movement delay starts at 180 ms and becomes faster
// along with the fruit.
//
// It will never become shorter than 110 ms.
//

const unsigned long START_FALL_INTERVAL = 500;
const unsigned long FALL_SPEED_STEP = 35;
const unsigned long MIN_FALL_INTERVAL = 325;

const unsigned long START_TILT_MOVE_DELAY = 180;
const unsigned long TILT_SPEED_STEP = 12;
const unsigned long MIN_TILT_MOVE_DELAY = 110;

unsigned long fallInterval =
  START_FALL_INTERVAL;

unsigned long tiltMoveDelay =
  START_TILT_MOVE_DELAY;

unsigned long lastTiltMove = 0;

// =============================================================================
// Difficulty update
// =============================================================================

void updateDifficulty()
{
  // Increase difficulty every 5 successfully caught fruits.
  int difficultyLevel =
    score / 5;

  // Calculate falling speed.
  unsigned long calculatedFallInterval =
    START_FALL_INTERVAL -
    (difficultyLevel * FALL_SPEED_STEP);

  // Limit the maximum falling speed.
  if (
    calculatedFallInterval <
    MIN_FALL_INTERVAL
  )
  {
    calculatedFallInterval =
      MIN_FALL_INTERVAL;
  }

  fallInterval =
    calculatedFallInterval;

  // ---------------------------------------------------------------------------
  // Make basket movement faster as the fruit becomes faster.
  // ---------------------------------------------------------------------------

  unsigned long calculatedTiltDelay =
    START_TILT_MOVE_DELAY -
    (difficultyLevel * TILT_SPEED_STEP);

  // Limit the maximum basket speed.
  if (
    calculatedTiltDelay <
    MIN_TILT_MOVE_DELAY
  )
  {
    calculatedTiltDelay =
      MIN_TILT_MOVE_DELAY;
  }

  tiltMoveDelay =
    calculatedTiltDelay;
}

// =============================================================================
// Matrix
// =============================================================================

/*
 * Rotate the entire display 180 degrees.
 */

int xy(
  int x,
  int y
)
{
  int displayX = 4 - x;
  int displayY = 4 - y;

  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 + (4 - displayX);
}

// =============================================================================
// Set one matrix pixel
// =============================================================================

void pixel(
  int x,
  int y,
  uint32_t colour
)
{
  if (
    x < 0 ||
    x > 4 ||
    y < 0 ||
    y > 4
  )
    return;

  matrix.setPixelColor(
    xy(x, y),
    colour
  );
}

// =============================================================================
// Draw game
// =============================================================================

void draw()
{
  matrix.clear();

  // ---------------------------------------------------------------------------
  // Basket
  // ---------------------------------------------------------------------------

  pixel(
    basketX,
    4,
    matrix.Color(0, 0, 255)
  );

  // ---------------------------------------------------------------------------
  // Falling fruit
  // ---------------------------------------------------------------------------

  if (
    fruitY >= 0 &&
    fruitY <= 4
  )
  {
    pixel(
      fruitX,
      fruitY,
      matrix.Color(0, 255, 0)
    );
  }

  matrix.show();
}

// =============================================================================
// IMU communication
// =============================================================================

void imuWrite(
  byte r,
  byte v
)
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

// =============================================================================
// Read 16-bit IMU register
// =============================================================================

int16_t imu16(
  byte r
)
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

// =============================================================================
// Configure IMU
// =============================================================================

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
  //   104 Hz
  //   ±2g
  //   Normal operating mode.
  imuWrite(
    0x10,
    0x40
  );
}

// =============================================================================
// Basket controls
// =============================================================================

void readTilt()
{
  int16_t ay =
    imu16(0x2A);

  // Ignore small tilts.
  if (abs(ay) < TILT_THRESHOLD)
    return;

  // Only move the basket at controlled intervals.
  if (
    millis() - lastTiltMove <
    tiltMoveDelay
  )
    return;

  // ---------------------------------------------------------------------------
  // +Y -> LEFT
  // -Y -> RIGHT
  // ---------------------------------------------------------------------------

  if (ay > TILT_THRESHOLD)
  {
    if (basketX > 0)
    {
      basketX--;

      // Short beep for left movement.
      tone(
        BUZZER_PIN,
        900,
        25
      );
    }
  }
  else if (ay < -TILT_THRESHOLD)
  {
    if (basketX < 4)
    {
      basketX++;

      // Short beep for right movement.
      tone(
        BUZZER_PIN,
        1100,
        25
      );
    }
  }

  // Start the movement timer even if the basket is
  // already at the edge.
  lastTiltMove = millis();
}

// =============================================================================
// Start a new fruit
// =============================================================================

void newFruit()
{
  fruitX = random(0, 5);
  fruitY = 0;
}

// =============================================================================
// Game-over sequence
// =============================================================================

void gameOver()
{
  Serial.println(
    "GAME OVER"
  );

  // Fill the matrix red.
  for (int i = 0; i < NUM_PIXELS; i++)
  {
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

  // Wait without blocking the FreeRTOS scheduler.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );

  // Reset game.
  score = 0;
  lives = 3;

  basketX = 2;

  // Reset difficulty.
  updateDifficulty();

  newFruit();

  lastFall = millis();
  lastTiltMove = millis();

  Serial.println(
    "Game restarted."
  );
}

// =============================================================================
// Game FreeRTOS task
// =============================================================================

void fruitGameTask(void *parameter)
{
  while (true)
  {
    // -------------------------------------------------------------------------
    // Read tilt
    // -------------------------------------------------------------------------

    readTilt();

    // -------------------------------------------------------------------------
    // Move fruit
    // -------------------------------------------------------------------------

    if (
      millis() - lastFall >=
      fallInterval
    )
    {
      lastFall = millis();

      fruitY++;

      // -----------------------------------------------------------------------
      // Fruit has reached the basket row.
      // -----------------------------------------------------------------------

      if (fruitY >= 4)
      {
        if (fruitX == basketX)
        {
          // -------------------------------------------------------------------
          // Successfully caught.
          // -------------------------------------------------------------------

          score++;

          tone(
            BUZZER_PIN,
            1200,
            80
          );

          Serial.print(
            "Caught! Score: "
          );

          Serial.println(score);

          // -------------------------------------------------------------------
          // Increase difficulty after every 5 catches.
          // -------------------------------------------------------------------

          if (score % 5 == 0)
          {
            updateDifficulty();

            Serial.println(
              "Difficulty increased!"
            );

            Serial.print(
              "Fruit interval: "
            );

            Serial.print(
              fallInterval
            );

            Serial.println(
              " ms"
            );

            Serial.print(
              "Basket movement delay: "
            );

            Serial.print(
              tiltMoveDelay
            );

            Serial.println(
              " ms"
            );
          }
        }
        else
        {
          // -------------------------------------------------------------------
          // Missed.
          // -------------------------------------------------------------------

          lives--;

          tone(
            BUZZER_PIN,
            200,
            150
          );

          Serial.print(
            "Missed! Lives: "
          );

          Serial.println(lives);
        }

        // Start a new fruit.
        newFruit();

        // Check for game over.
        if (lives <= 0)
        {
          gameOver();
        }
      }
    }

    // -------------------------------------------------------------------------
    // Draw
    // -------------------------------------------------------------------------

    draw();

    // Run the game approximately every 20 ms.
    vTaskDelay(
      pdMS_TO_TICKS(20)
    );
  }
}

// =============================================================================
// Setup
// =============================================================================

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

  // Initialize difficulty.
  updateDifficulty();

  newFruit();

  lastFall = millis();
  lastTiltMove = millis();

  Serial.println(
    "Catch the Falling Fruit started."
  );

  Serial.print(
    "Initial fruit interval: "
  );

  Serial.print(
    fallInterval
  );

  Serial.println(
    " ms"
  );

  // Create the game task.
  xTaskCreatePinnedToCore(
    fruitGameTask,
    "Fruit Game Task",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}

// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // The game is handled by fruitGameTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
