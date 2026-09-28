/*
 * =============================================================================
 * WHACK-A-MOLE
 * =============================================================================
 *
 * A random pixel appears on the 5x5 matrix.
 *
 * PLAYER:
 *   Tilt -> move the cursor.
 *   Button -> whack the target.
 *
 * IMU CONFIGURATION:
 *   Physical Y axis -> horizontal movement
 *   Physical X axis -> vertical movement
 *
 *   Because the device/display is upside down:
 *
 *     Positive Y -> RIGHT
 *     Negative Y -> LEFT
 *
 *     Positive X -> DOWN
 *     Negative X -> UP
 *
 *   Tilt threshold = approximately 15 degrees.
 *
 *   Holding a tilt continuously moves the cursor.
 *
 * DISPLAY:
 *   Entire matrix is rotated 180 degrees.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Buzzer = GPIO 16
 * IMU    = LSM6DSM SPI
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

// Cursor movement speed while holding a tilt.
#define CURSOR_MOVE_DELAY 180

// Target timeout.
#define TARGET_TIME 2500

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

int cursorX = 2;
int cursorY = 2;

int targetX = 0;
int targetY = 0;

unsigned long targetStart = 0;
unsigned long lastCursorMove = 0;

int score = 0;

//=============================================================================
// Matrix
//=============================================================================

/*
 * Rotate the entire display 180 degrees.
 *
 * Game coordinates:
 *
 *   (0,0) -------- (4,0)
 *     |
 *     |
 *   (0,4) -------- (4,4)
 *
 * Physical display is rotated:
 *
 *   displayX = 4 - gameX
 *   displayY = 4 - gameY
 */

int xyToIndex(int x, int y)
{
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

//=============================================================================
// Draw
//=============================================================================

void draw()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Target
  //------------------------------------------------------------------------

  setPixel(
    targetX,
    targetY,
    matrix.Color(255, 0, 0)
  );

  //------------------------------------------------------------------------
  // Cursor
  //------------------------------------------------------------------------

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

  draw();
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
  // Normal operating mode.
  imuWrite(0x10, 0x40);
}

//=============================================================================
// Tilt Control
//=============================================================================

void readTilt()
{
  int16_t ax = imuRead16(0x28);
  int16_t ay = imuRead16(0x2A);

  //------------------------------------------------------------------------
  // Require approximately 15 degrees of tilt.
  //------------------------------------------------------------------------

  if (abs(ax) < TILT_THRESHOLD &&
      abs(ay) < TILT_THRESHOLD) {

    return;
  }

  //------------------------------------------------------------------------
  // Only move at the configured rate.
  //------------------------------------------------------------------------

  if (millis() - lastCursorMove < CURSOR_MOVE_DELAY)
    return;

  lastCursorMove = millis();

  //------------------------------------------------------------------------
  // Choose whichever axis has the stronger tilt.
  // This prevents diagonal tilts from moving both axes at once.
  //------------------------------------------------------------------------

  if (abs(ay) > abs(ax)) {

    //===============================================================
    // Y AXIS = HORIZONTAL
    //===============================================================

    // Positive Y -> RIGHT.
    if (ay < TILT_THRESHOLD) {

      if (cursorX < 4) {
        cursorX++;

        tone(BUZZER_PIN, 700, 20);
      }
    }

    // Negative Y -> LEFT.
    else if (ay > -TILT_THRESHOLD) {

      if (cursorX > 0) {
        cursorX--;

        tone(BUZZER_PIN, 700, 20);
      }
    }
  }

  else {

    //===============================================================
    // X AXIS = VERTICAL
    //===============================================================

    // Positive X -> DOWN.
    if (ax > TILT_THRESHOLD) {

      if (cursorY < 4) {
        cursorY++;

        tone(BUZZER_PIN, 700, 20);
      }
    }

    // Negative X -> UP.
    else if (ax < -TILT_THRESHOLD) {

      if (cursorY > 0) {
        cursorY--;

        tone(BUZZER_PIN, 700, 20);
      }
    }
  }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  // Button.
  pinMode(
    BUTTON_PIN,
    INPUT_PULLUP
  );

  // Buzzer.
  pinMode(
    BUZZER_PIN,
    OUTPUT
  );

  digitalWrite(
    BUZZER_PIN,
    LOW
  );

  // Matrix.
  matrix.begin();

  matrix.setBrightness(10);

  matrix.clear();
  matrix.show();

  // IMU.
  setupIMU();

  // Random target positions.
  randomSeed(
    (uint32_t)esp_random()
  );

  newTarget();

  Serial.println("Whack-A-Mole started.");
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //===========================================================================
  // Read tilt
  //===========================================================================

  readTilt();

  //===========================================================================
  // Button
  //===========================================================================

  static bool oldButton = HIGH;

  bool button = digitalRead(BUTTON_PIN);

  // Detect new button press.
  if (button == LOW &&
      oldButton == HIGH) {

    // Player hit the target.
    if (cursorX == targetX &&
        cursorY == targetY) {

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

    // Player missed.
    else {

      tone(
        BUZZER_PIN,
        150,
        150
      );
    }
  }

  oldButton = button;

  //===========================================================================
  // Target timeout
  //===========================================================================

  if (millis() - targetStart >= TARGET_TIME) {

    Serial.println("Miss!");

    tone(
      BUZZER_PIN,
      200,
      150
    );

    newTarget();
  }

  //===========================================================================
  // Display
  //===========================================================================

  draw();

  delay(10);
}
