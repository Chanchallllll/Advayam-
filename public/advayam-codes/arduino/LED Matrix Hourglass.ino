/*
  5x5 Falling Sand Timer

  - Sand starts at the TOP row, CENTER pixel
  - Each grain falls downward
  - Grains pile up naturally
  - Matrix gradually fills with sand
  - All 25 pixels are filled in approximately 60 seconds

  Matrix coordinates:

      0 1 2 3 4
    0 . . S . .
    1 . . ↓ . .
    2 . . ↓ . .
    3 . . ↓ . .
    4 . . ↓ . .

  S = starting point
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7

#define WIDTH 5
#define HEIGHT 5
#define NUM_LEDS 25

// Total time for a complete fill (or a complete drain)
#define TOTAL_TIME 60000UL

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13
#define IMU_INT  38

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// --------------------------------------------------
// Convert X/Y to LED number
// --------------------------------------------------

int pixelXY(int x, int y) {

  if (y % 2 == 0)
    return y * WIDTH + x;

  return y * WIDTH + (WIDTH - 1 - x);
}


// --------------------------------------------------
// Orientation state
// --------------------------------------------------
//
// The board has no single "correct" side up. Whatever way it is
// held when it powers on is treated as the starting orientation,
// and every later flip is judged relative to the orientation the
// board was last held in - not a fixed axis.

bool orientationFlipped = false;
int fillDirection = 1; // 1 = filling, -1 = draining

float refDown[3] = {0, 0, 0};

bool flipCandidate = false;
unsigned long flipHoldStart = 0;
unsigned long lastFlipTime = 0;

const float FLIP_DOT_THRESHOLD = -0.5f; // allows some incline, not just a perfect flip
const unsigned long FLIP_HOLD_MS = 300;
const unsigned long FLIP_COOLDOWN_MS = 800;


// --------------------------------------------------
// Set pixel
// --------------------------------------------------

void setPixelXY(int x, int y, uint32_t color) {

  if (x < 0 || x >= WIDTH ||
      y < 0 || y >= HEIGHT)
    return;

  // Map logical coordinates onto the physical LED layout.
  int hx = (HEIGHT-1) - y;
  int hy = x;

  // The active fall direction can be swapped at runtime.
  if (orientationFlipped) {
    hx = (WIDTH - 1) - hx;
    hy = (HEIGHT - 1) - hy;
  }

  matrix.setPixelColor(
    pixelXY(hx, hy),
    color
  );
}


// --------------------------------------------------
// Sand array + grain history (needed to undo grains
// in the order they were placed)
// --------------------------------------------------

bool sand[HEIGHT][WIDTH];

int grainOrder[NUM_LEDS][2];
int grainCount = 0;


// --------------------------------------------------
// Draw all settled sand
// --------------------------------------------------

void drawSand() {

  matrix.clear();

  uint32_t sandColor =
    matrix.Color(255, 120, 0);

  for (int y = 0; y < HEIGHT; y++) {

    for (int x = 0; x < WIDTH; x++) {

      if (sand[y][x]) {

        setPixelXY(
          x,
          y,
          sandColor
        );
      }
    }
  }

  matrix.show();
}


// --------------------------------------------------
// Read a normalized gravity vector from the IMU
// --------------------------------------------------

void readGravity(float &x, float &y, float &z) {

  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;

  imu.getEvent(&accel, &gyro, &temp);

  x = accel.acceleration.x;
  y = accel.acceleration.y;
  z = accel.acceleration.z;

  float mag = sqrt(x * x + y * y + z * z);

  if (mag > 0.0001f) {
    x /= mag;
    y /= mag;
    z /= mag;
  }
}


// --------------------------------------------------
// Check whether the board has been turned upside down
// relative to the orientation it was last held in.
//
// Returns true the moment a flip is confirmed.
// --------------------------------------------------

bool checkFlip() {

  float gx, gy, gz;
  readGravity(gx, gy, gz);

  unsigned long now = millis();

  if (now - lastFlipTime < FLIP_COOLDOWN_MS)
    return false;

  float dot = gx * refDown[0] +
              gy * refDown[1] +
              gz * refDown[2];

  if (dot < FLIP_DOT_THRESHOLD) {

    if (!flipCandidate) {
      flipCandidate = true;
      flipHoldStart = now;
    }

    if (now - flipHoldStart >= FLIP_HOLD_MS) {

      orientationFlipped = !orientationFlipped;
      fillDirection = -fillDirection;

      // The orientation just held becomes the new reference,
      // so the next flip is judged against it, not the original.
      refDown[0] = gx;
      refDown[1] = gy;
      refDown[2] = gz;

      flipCandidate = false;
      lastFlipTime = now;

      return true;
    }

  } else {

    flipCandidate = false;
  }

  return false;
}


// --------------------------------------------------
// Animate one falling grain. Returns false if the
// grain never settled (e.g. a flip interrupted it).
// --------------------------------------------------

bool addGrain() {

  int x = 2;
  int y = 0;

  // If starting position is occupied,
  // find another empty position on the top row.
  if (sand[y][x]) {

    bool found = false;

    for (int offset = 1; offset <= 2; offset++) {

      int left = 2 - offset;
      int right = 2 + offset;

      if (left >= 0 && !sand[0][left]) {
        x = left;
        found = true;
        break;
      }

      if (right < WIDTH && !sand[0][right]) {
        x = right;
        found = true;
        break;
      }
    }

    if (!found)
      return false;
  }


  while (true) {

    // A flip mid-fall hands control back to the main loop,
    // which will start draining instead.
    if (checkFlip())
      return false;

    if (y + 1 < HEIGHT &&
        !sand[y + 1][x]) {

      y++;

      drawSand();

      setPixelXY(x, y, matrix.Color(255, 150, 20));

      matrix.show();

      delay(80);

      continue;
    }


    bool moved = false;

    if (random(2) == 0) {

      if (x > 0 &&
          y + 1 < HEIGHT &&
          !sand[y + 1][x - 1]) {

        x--;
        y++;
        moved = true;

      } else if (x < WIDTH - 1 &&
                 y + 1 < HEIGHT &&
                 !sand[y + 1][x + 1]) {

        x++;
        y++;
        moved = true;
      }

    } else {

      if (x < WIDTH - 1 &&
          y + 1 < HEIGHT &&
          !sand[y + 1][x + 1]) {

        x++;
        y++;
        moved = true;

      } else if (x > 0 &&
                 y + 1 < HEIGHT &&
                 !sand[y + 1][x - 1]) {

        x--;
        y++;
        moved = true;
      }
    }


    if (moved) {

      drawSand();

      setPixelXY(x, y, matrix.Color(255, 150, 20));

      matrix.show();

      delay(80);

      continue;
    }


    // Grain has reached its resting position
    sand[y][x] = true;

    grainOrder[grainCount][0] = x;
    grainOrder[grainCount][1] = y;
    grainCount++;

    drawSand();

    return true;
  }
}


// --------------------------------------------------
// Undo the most recently placed grain
// --------------------------------------------------

void removeGrain() {

  if (grainCount <= 0)
    return;

  grainCount--;

  int x = grainOrder[grainCount][0];
  int y = grainOrder[grainCount][1];

  drawSand();

  setPixelXY(x, y, matrix.Color(255, 220, 120));

  matrix.show();

  delay(80);

  sand[y][x] = false;

  drawSand();
}


// --------------------------------------------------
// Setup
// --------------------------------------------------

void setup() {

  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  matrix.clear();
  matrix.show();

  randomSeed(analogRead(A0));

  memset(sand, false, sizeof(sand));
  grainCount = 0;

  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );

  if (!imu.begin_SPI(IMU_CS, &SPI)) {

    Serial.println("IMU initialization failed!");

    while (1) {
      delay(100);
    }
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  // Whatever way the board happens to be held right now becomes
  // the "upright" starting orientation - there's no fixed side.
  float sx = 0, sy = 0, sz = 0;

  for (int i = 0; i < 20; i++) {

    float gx, gy, gz;
    readGravity(gx, gy, gz);

    sx += gx;
    sy += gy;
    sz += gz;

    delay(10);
  }

  refDown[0] = sx / 20.0f;
  refDown[1] = sy / 20.0f;
  refDown[2] = sz / 20.0f;

  float mag = sqrt(
    refDown[0] * refDown[0] +
    refDown[1] * refDown[1] +
    refDown[2] * refDown[2]
  );

  if (mag > 0.0001f) {
    refDown[0] /= mag;
    refDown[1] /= mag;
    refDown[2] /= mag;
  }

  orientationFlipped = false;
  fillDirection = 1;

  Serial.println("Falling Sand Timer");
}


// --------------------------------------------------
// Main loop
// --------------------------------------------------

const unsigned long grainInterval = TOTAL_TIME / NUM_LEDS;
unsigned long lastGrainTime = 0;

void loop() {

  checkFlip();

  unsigned long now = millis();

  if (now - lastGrainTime >= grainInterval) {

    if (fillDirection == 1 && grainCount < NUM_LEDS) {

      addGrain();
      lastGrainTime = now;

    } else if (fillDirection == -1 && grainCount > 0) {

      removeGrain();
      lastGrainTime = now;
    }

    // If neither branch applies, the timer has either finished
    // filling or finished draining, and simply waits for a flip.
  }

  delay(20);
}
