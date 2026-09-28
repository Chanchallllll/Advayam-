/*
 * =============================================================================
 * RHYTHM TAP REVOLUTION
 * =============================================================================
 * Falling notes move toward the bottom row.
 *
 * Press the button when a note reaches the bottom.
 *
 * SCORING:
 *   Correct tap       -> score increases
 *   Consecutive hits  -> multiplier increases
 *   Higher score      -> notes fall faster
 *
 * MISSES:
 *   Missing a note slows the falling speed.
 *
 * SPECIAL BEHAVIOUR:
 *   Scores divisible by 5 produce a special buzzer tone.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Matrix:
 *   GPIO 7
 *
 * Button:
 *   GPIO 17
 *
 * Buzzer:
 *   GPIO 16
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

#define NUM_PIXELS 25

Adafruit_NeoPixel matrix(
  NUM_PIXELS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game Variables
//=============================================================================

int noteX;
int noteY;

int score = 0;
int multiplier = 1;

// Random colour of the current note.
uint32_t noteColor;

unsigned long lastNoteMove = 0;

// Number of misses.
int missCount = 0;

// Additional delay caused by misses.
int missSlowdown = 0;

// Maximum slowdown.
const int MAX_SLOWDOWN = 1000;

//=============================================================================
// Matrix
//=============================================================================

/*
 * Convert game coordinates to physical matrix coordinates.
 *
 * The entire display is rotated 180 degrees.
 */

int xy(int x, int y)
{
  int displayX = 4 - x;
  int displayY = 4 - y;

  if (displayY % 2 == 0)
    return displayY * 5 + displayX;

  return displayY * 5 + (4 - displayX);
}

//=============================================================================
// Draw
//=============================================================================

void draw()
{
  matrix.clear();

  matrix.setPixelColor(
    xy(noteX, noteY),
    noteColor
  );

  matrix.show();
}

//=============================================================================
// Falling Speed
//=============================================================================

int getSpeed()
{
  // Higher score makes notes fall faster.
  int speed = 600 - score * 15;

  // Misses make the game slower.
  speed += missSlowdown;

  return constrain(
    speed,
    100,
    1600
  );
}

//=============================================================================
// Random Note
//=============================================================================

void newNote()
{
  noteX = random(0, 5);
  noteY = 0;

  int red   = random(0, 256);
  int green = random(0, 256);
  int blue  = random(0, 256);

  noteColor = matrix.Color(
    red,
    green,
    blue
  );

  lastNoteMove = millis();
}

//=============================================================================
// Successful Tap
//=============================================================================

void successfulTap()
{
  score++;

  // Every consecutive hit increases the multiplier.
  multiplier = min(
    10,
    multiplier + 1
  );

  // A successful hit removes some of the slowdown.
  if (missSlowdown > 0) {

    missSlowdown -= 50;

    if (missSlowdown < 0)
      missSlowdown = 0;
  }

  // Normal multiplier tone.
  int toneFrequency =
    500 + multiplier * 100;

  tone(
    BUZZER_PIN,
    toneFrequency,
    80
  );

  Serial.print("Score: ");
  Serial.print(score);

  Serial.print(" | Multiplier: x");
  Serial.print(multiplier);

  Serial.print(" | Speed: ");
  Serial.println(getSpeed());

  // Every score divisible by 5 gets a special tone.
  if (score % 5 == 0) {

    int duration =
      max(40, 180 - score * 3);

    tone(
      BUZZER_PIN,
      1200 + score * 20,
      duration
    );

    Serial.println(
      "*** SCORE MILESTONE! ***"
    );
  }
}

//=============================================================================
// Missed Note
//=============================================================================

void missed()
{
  multiplier = 1;

  missCount++;

  // Slow the game down after a miss.
  //
  // Each miss adds 100 ms.
  missSlowdown += 100;

  if (missSlowdown > MAX_SLOWDOWN)
    missSlowdown = MAX_SLOWDOWN;

  tone(
    BUZZER_PIN,
    180,
    150
  );

  Serial.print("Miss!  Slowdown: +");
  Serial.print(missSlowdown);
  Serial.println(" ms");
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

  matrix.begin();

  matrix.setBrightness(10);

  randomSeed(
    (uint32_t)esp_random()
  );

  newNote();

  draw();
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  static bool oldButton = HIGH;

  bool button =
    digitalRead(BUTTON_PIN);

  //------------------------------------------------------------------------
  // Button press attempts to hit the note.
  //------------------------------------------------------------------------

  if (button == LOW &&
      oldButton == HIGH) {

    if (noteY >= 3) {

      successfulTap();

      newNote();
    }

    else {

      missed();

      newNote();
    }
  }

  oldButton = button;

  //------------------------------------------------------------------------
  // Move the falling note.
  //------------------------------------------------------------------------

  if (millis() - lastNoteMove >= getSpeed()) {

    lastNoteMove = millis();

    noteY++;

    // Note reached the bottom without being hit.
    if (noteY > 4) {

      missed();

      newNote();
    }
  }

  draw();

  delay(10);
}
