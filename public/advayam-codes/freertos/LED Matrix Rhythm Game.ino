/*
 * =============================================================================
 * RHYTHM TAP REVOLUTION
 * =============================================================================
 *
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
 *   Missing a note slows the game down.
 *
 * SPECIAL:
 *   Scores divisible by 5 produce a special buzzer tone.
 *
 * DISPLAY:
 *   Rotated 180 degrees.
 *
 * Hardware:
 *   Matrix DATA = GPIO 7
 *   Button      = GPIO 17
 *   Buzzer      = GPIO 16
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

uint32_t noteColor;

unsigned long lastNoteMove = 0;

int missCount = 0;

int missSlowdown = 0;

const int MAX_SLOWDOWN = 1000;

//=============================================================================
// Matrix
//=============================================================================

int xy(int x, int y)
{
  // Rotate display 180 degrees.
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
  // Higher score = faster notes.
  int speed =
    600 - score * 15;

  // Misses = slower notes.
  speed += missSlowdown;

  return constrain(
    speed,
    100,
    1600
  );
}

//=============================================================================
// New Note
//=============================================================================

void newNote()
{
  noteX = random(0, 5);

  noteY = 0;

  int red =
    random(0, 256);

  int green =
    random(0, 256);

  int blue =
    random(0, 256);

  noteColor =
    matrix.Color(
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

  multiplier =
    min(
      10,
      multiplier + 1
    );

  // Reduce slowdown after successful hit.
  if (missSlowdown > 0)
  {
    missSlowdown -= 50;

    if (missSlowdown < 0)
      missSlowdown = 0;
  }

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

  //------------------------------------------------------------------------
  // Milestone every 5 points.
  //------------------------------------------------------------------------

  if (score % 5 == 0)
  {
    int duration =
      max(
        40,
        180 - score * 3
      );

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
// Miss
//=============================================================================

void missed()
{
  multiplier = 1;

  missCount++;

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
// Loop
//=============================================================================

void loop()
{
  static bool oldButton = HIGH;

  bool button =
    digitalRead(BUTTON_PIN);

  //------------------------------------------------------------------------
  // Button press.
  //------------------------------------------------------------------------

  if (button == LOW &&
      oldButton == HIGH)
  {
    if (noteY >= 3)
    {
      successfulTap();

      newNote();
    }
    else
    {
      missed();

      newNote();
    }
  }

  oldButton = button;

  //------------------------------------------------------------------------
  // Move note.
  //------------------------------------------------------------------------

  if (millis() - lastNoteMove >= getSpeed())
  {
    lastNoteMove = millis();

    noteY++;

    // Note reached bottom without being hit.
    if (noteY > 4)
    {
      missed();

      newNote();
    }
  }

  draw();

  delay(10);
}
