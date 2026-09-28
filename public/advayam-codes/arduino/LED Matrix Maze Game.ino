/*
 * =============================================================================
 * PIXEL MAZE ESCAPE
 * =============================================================================
 * Generates a randomized 5x5 maze.
 *
 * START:
 *   (0,0)
 *
 * EXIT:
 *   (4,4)
 *
 * TILT CONTROLS:
 *
 *   +Y -> LEFT
 *   -Y -> RIGHT
 *
 *   +X -> DOWN
 *   -X -> UP
 *
 * Tilt below approximately 15 degrees is ignored.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Walls are displayed as red pixels.
 * Player is green.
 * Exit is blue.
 *
 * Matrix = GPIO 7
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
#define BUZZER_PIN 16

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

#define NUM_PIXELS 25

// Approximately 15 degrees with ±2g configuration.
#define TILT_THRESHOLD 4240

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Maze
//=============================================================================

bool wall[5][5];

int playerX = 0;
int playerY = 0;

//=============================================================================
// Tilt timing
//=============================================================================

// Prevents the player from moving multiple cells instantly.
unsigned long lastMove = 0;

const unsigned long MOVE_DELAY = 180;

//=============================================================================
// Matrix
//=============================================================================

/*
 * Rotate the display 180 degrees.
 *
 * Game coordinates are transformed before being sent
 * to the physical NeoPixel matrix.
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

//=============================================================================
// Maze Generation
//=============================================================================
void generateMaze()
{
  while (true) {

    //=======================================================================
    // Generate random walls.
    //=======================================================================

    for (int x = 0; x < 5; x++) {
      for (int y = 0; y < 5; y++) {

        // Start and exit must always remain open.
        if ((x == 0 && y == 0) ||
            (x == 4 && y == 4)) {

          wall[x][y] = false;
        }
        else {

          // About 30% chance of being a wall.
          wall[x][y] = (random(0, 100) < 30);
        }
      }
    }

    //=======================================================================
    // Check whether the maze is actually solvable.
    //=======================================================================

    bool visited[5][5];

    memset(visited, false, sizeof(visited));

    int queueX[25];
    int queueY[25];

    int head = 0;
    int tail = 0;

    // Start at (0,0).
    queueX[tail] = 0;
    queueY[tail] = 0;
    tail++;

    visited[0][0] = true;

    while (head < tail) {

      int x = queueX[head];
      int y = queueY[head];
      head++;

      // Reached the exit.
      if (x == 4 && y == 4)
        break;

      // Four possible directions.
      const int dx[4] = { 1, -1, 0, 0 };
      const int dy[4] = { 0, 0, 1, -1 };

      for (int d = 0; d < 4; d++) {

        int nx = x + dx[d];
        int ny = y + dy[d];

        if (nx < 0 || nx >= 5 ||
            ny < 0 || ny >= 5)
          continue;

        if (wall[nx][ny])
          continue;

        if (visited[nx][ny])
          continue;

        visited[nx][ny] = true;

        queueX[tail] = nx;
        queueY[tail] = ny;
        tail++;
      }
    }

    //=======================================================================
    // If the exit was reached, keep this maze.
    // Otherwise generate another one.
    //=======================================================================

    if (visited[4][4])
      break;
  }
}

void draw()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Walls.
  //------------------------------------------------------------------------

  for (int x = 0; x < 5; x++) {

    for (int y = 0; y < 5; y++) {

      if (wall[x][y]) {

        pixel(
          x,
          y,
          matrix.Color(255, 0, 0)
        );
      }
    }
  }

  //------------------------------------------------------------------------
  // Exit.
  //------------------------------------------------------------------------

  pixel(
    4,
    4,
    matrix.Color(0, 0, 255)
  );

  //------------------------------------------------------------------------
  // Player.
  //------------------------------------------------------------------------

  pixel(
    playerX,
    playerY,
    matrix.Color(0, 255, 0)
  );

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
// Player Movement
//=============================================================================

void movePlayer()
{
  int16_t ax = imu16(0x28);
  int16_t ay = imu16(0x2A);

  //------------------------------------------------------------------------
  // Ignore small tilts.
  //------------------------------------------------------------------------

  if (abs(ax) < TILT_THRESHOLD &&
      abs(ay) < TILT_THRESHOLD)
    return;

  //------------------------------------------------------------------------
  // Prevent movement from happening too quickly.
  //------------------------------------------------------------------------

  if (millis() - lastMove < MOVE_DELAY)
    return;

  int newX = playerX;
  int newY = playerY;

  //------------------------------------------------------------------------
  // Determine which axis is being tilted more strongly.
  //------------------------------------------------------------------------

  if (abs(ay) > abs(ax)) {

    //=======================================================================
    // Y AXIS
    //
    // Inverted:
    //
    // +Y -> LEFT
    // -Y -> RIGHT
    //=======================================================================

    if (ay > TILT_THRESHOLD)
      newX--;

    else if (ay < -TILT_THRESHOLD)
      newX++;
  }

  else {

    //=======================================================================
    // X AXIS
    //
    // +X -> DOWN
    // -X -> UP
    //=======================================================================

    if (ax > TILT_THRESHOLD)
      newY++;

    else if (ax < -TILT_THRESHOLD)
      newY--;
  }

  //------------------------------------------------------------------------
  // Only move if the destination is inside the maze and isn't a wall.
  //------------------------------------------------------------------------

  if (newX >= 0 &&
      newX < 5 &&
      newY >= 0 &&
      newY < 5 &&
      !wall[newX][newY]) {

    playerX = newX;
    playerY = newY;

    lastMove = millis();

    tone(
      BUZZER_PIN,
      700,
      30
    );
  }
}

//=============================================================================
// Victory
//=============================================================================

void mazeEscaped()
{
  Serial.println(
    "MAZE ESCAPED!"
  );

  //------------------------------------------------------------------------
  // Green victory animation.
  //------------------------------------------------------------------------

  for (int i = 0; i < 3; i++) {

    for (int j = 0; j < NUM_PIXELS; j++) {

      matrix.setPixelColor(
        j,
        matrix.Color(0, 255, 0)
      );
    }

    matrix.show();

    tone(
      BUZZER_PIN,
      1000 + i * 300,
      150
    );

    delay(200);

    matrix.clear();
    matrix.show();

    delay(100);
  }

  delay(500);

  //------------------------------------------------------------------------
  // Generate a new maze.
  //------------------------------------------------------------------------

  playerX = 0;
  playerY = 0;

  generateMaze();

  lastMove = millis();
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

  generateMaze();

  draw();

  Serial.println(
    "Pixel Maze Escape started."
  );
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  movePlayer();

  //------------------------------------------------------------------------
  // Check for exit.
  //------------------------------------------------------------------------

  if (playerX == 4 &&
      playerY == 4) {

    mazeEscaped();
  }

  draw();

  delay(20);
}
