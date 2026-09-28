/*
 * =============================================================================
 * MINESWEEPER MINI
 * =============================================================================
 *
 * Simplified 5x5 Minesweeper game.
 *
 * CONTROLS:
 *   Tilt          -> move cursor
 *   Short press   -> uncover
 *   Long press    -> flag / unflag
 *
 * IMU CONFIGURATION:
 *   Physical Y axis -> horizontal movement
 *   Physical X axis -> vertical movement
 *
 *   +Y -> RIGHT
 *   -Y -> LEFT
 *
 *   +X -> DOWN
 *   -X -> UP
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

// Long press duration.
#define LONG_PRESS_TIME 700

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

bool mine[5][5];
bool revealed[5][5];
bool flagged[5][5];

int cursorX = 0;
int cursorY = 0;

//=============================================================================
// Timing
//=============================================================================

unsigned long lastCursorMove = 0;

//=============================================================================
// Matrix
//=============================================================================

/*
 * Rotate the entire display 180 degrees.
 *
 * Game coordinate:
 *
 *   (0,0) -------- (4,0)
 *     |
 *     |
 *   (0,4) -------- (4,4)
 *
 * Display is rotated:
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
// Count Nearby Mines
//=============================================================================

int countMines(int x, int y)
{
  int count = 0;

  for (int dx = -1; dx <= 1; dx++) {

    for (int dy = -1; dy <= 1; dy++) {

      int nx = x + dx;
      int ny = y + dy;

      if (nx >= 0 && nx < 5 &&
          ny >= 0 && ny < 5 &&
          mine[nx][ny]) {

        count++;
      }
    }
  }

  return count;
}

//=============================================================================
// Draw
//=============================================================================

void draw()
{
  matrix.clear();

  for (int x = 0; x < 5; x++) {

    for (int y = 0; y < 5; y++) {

      uint32_t color =
        matrix.Color(0, 0, 0);

      // Flagged cell.
      if (flagged[x][y]) {

        color = matrix.Color(
          255,
          150,
          0
        );
      }

      // Revealed safe cell.
      else if (revealed[x][y]) {

        color = matrix.Color(
          0,
          150,
          0
        );
      }

      // Cursor always appears white.
      if (x == cursorX &&
          y == cursorY) {

        color = matrix.Color(
          255,
          255,
          255
        );
      }

      setPixel(
        x,
        y,
        color
      );
    }
  }

  matrix.show();
}

//=============================================================================
// Reveal Everything
//=============================================================================

void revealAll()
{
  for (int x = 0; x < 5; x++) {

    for (int y = 0; y < 5; y++) {

      revealed[x][y] = true;
    }
  }

  draw();
}

//=============================================================================
// Setup New Game
//=============================================================================

void setupGame()
{
  memset(
    mine,
    false,
    sizeof(mine)
  );

  memset(
    revealed,
    false,
    sizeof(revealed)
  );

  memset(
    flagged,
    false,
    sizeof(flagged)
  );

  //------------------------------------------------------------------------
  // Place five mines.
  //------------------------------------------------------------------------

  int placed = 0;

  while (placed < 5) {

    int x = random(0, 5);
    int y = random(0, 5);

    // Keep starting cell safe.
    if (x == 0 && y == 0)
      continue;

    if (!mine[x][y]) {

      mine[x][y] = true;
      placed++;
    }
  }

  //------------------------------------------------------------------------
  // Reset cursor.
  //------------------------------------------------------------------------

  cursorX = 0;
  cursorY = 0;

  lastCursorMove = millis();

  draw();
}

//=============================================================================
// IMU
//=============================================================================

void imuWrite(byte reg, byte value)
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
    (highByte << 8) |
    lowByte
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
// Cursor Movement
//=============================================================================

void moveCursor()
{
  int16_t ax =
    imuRead16(0x28);

  int16_t ay =
    imuRead16(0x2A);

  //------------------------------------------------------------------------
  // Ignore tilt below approximately 15 degrees.
  //------------------------------------------------------------------------

  if (abs(ax) < TILT_THRESHOLD &&
      abs(ay) < TILT_THRESHOLD) {

    return;
  }

  //------------------------------------------------------------------------
  // Continuous movement.
  //------------------------------------------------------------------------

  if (millis() - lastCursorMove <
      CURSOR_MOVE_DELAY) {

    return;
  }

  lastCursorMove = millis();

  //------------------------------------------------------------------------
  // Use the strongest axis.
  // This prevents diagonal tilt from moving both axes simultaneously.
  //------------------------------------------------------------------------

  if (abs(ay) > abs(ax)) {

    //=======================================================================
    // Y AXIS = HORIZONTAL
    //=======================================================================

    // +Y -> LEFT
    if (ay > TILT_THRESHOLD) {

      if (cursorX < 4) {

        cursorX++;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }

    // -Y -> RIGHT
    else if (ay < -TILT_THRESHOLD) {

      if (cursorX > 0) {

        cursorX--;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }
  }

  else {

    //=======================================================================
    // X AXIS = VERTICAL
    //=======================================================================

    // +X -> DOWN
    if (ax > TILT_THRESHOLD) {

      if (cursorY < 4) {

        cursorY++;

        tone(
          BUZZER_PIN,
          700,
          20
        );
      }
    }

    // -X -> UP
    else if (ax < -TILT_THRESHOLD) {

      if (cursorY > 0) {

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
// Uncover
//=============================================================================

void uncover()
{
  // Cannot uncover a flagged square.
  if (flagged[cursorX][cursorY])
    return;

  // Already revealed.
  if (revealed[cursorX][cursorY])
    return;

  //------------------------------------------------------------------------
  // Mine!
  //------------------------------------------------------------------------

  if (mine[cursorX][cursorY]) {

    Serial.println("BOOM!");

    revealAll();

    tone(
      BUZZER_PIN,
      120,
      500
    );

    delay(1000);

    setupGame();

    return;
  }

  //------------------------------------------------------------------------
  // Safe square.
  //------------------------------------------------------------------------

  revealed[cursorX][cursorY] = true;

  int nearby =
    countMines(
      cursorX,
      cursorY
    );

  tone(
    BUZZER_PIN,
    500 + nearby * 150,
    80
  );

  Serial.print(
    "Nearby mines: "
  );

  Serial.println(
    nearby
  );
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

  matrix.setBrightness(80);

  matrix.clear();
  matrix.show();

  setupIMU();

  randomSeed(
    (uint32_t)esp_random()
  );

  setupGame();

  Serial.println(
    "Minesweeper started."
  );
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //===========================================================================
  // Cursor
  //===========================================================================

  moveCursor();

  //===========================================================================
  // Button
  //===========================================================================

  static bool previousButton = HIGH;

  static unsigned long buttonStart = 0;

  bool button =
    digitalRead(BUTTON_PIN);

  //------------------------------------------------------------------------
  // Button just pressed.
  //------------------------------------------------------------------------

  if (button == LOW &&
      previousButton == HIGH) {

    buttonStart = millis();
  }

  //------------------------------------------------------------------------
  // Button just released.
  //------------------------------------------------------------------------

  if (button == HIGH &&
      previousButton == LOW) {

    unsigned long duration =
      millis() - buttonStart;

    // Long press = flag/unflag.
    if (duration >= LONG_PRESS_TIME) {

      // Don't flag an already revealed square.
      if (!revealed[cursorX][cursorY]) {

        flagged[cursorX][cursorY] =
          !flagged[cursorX][cursorY];

        tone(
          BUZZER_PIN,
          700,
          60
        );
      }
    }

    // Short press = uncover.
    else {

      uncover();
    }
  }

  previousButton = button;

  //===========================================================================
  // Display
  //===========================================================================

  draw();

  delay(10);
}
