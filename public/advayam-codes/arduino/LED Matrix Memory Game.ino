/*
 * =============================================================================
 * SIMON SAYS MEMORY GAME
 * =============================================================================
 * The matrix flashes a growing sequence of four colours.
 *
 * PLAYER INPUT:
 *   Touch GPIO 8 -> Cycle through colours
 *   Button GPIO 17 -> Select/confirm current colour
 *
 * COLOUR ORDER:
 *   RED -> GREEN -> BLUE -> YELLOW -> RED...
 *
 * Every touch gives a short beep.
 *
 * The sequence gets longer after every successful round.
 *
 * Matrix = GPIO 7
 * Button = GPIO 17
 * Touch  = GPIO 8
 * Buzzer = GPIO 16
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define TOUCH_PIN 8
#define BUZZER_PIN 16

#define MAX_SEQUENCE 20

Adafruit_NeoPixel matrix(25, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

byte sequence[MAX_SEQUENCE];
int sequenceLength = 1;

// Used for detecting a new touch.
bool lastTouchState = HIGH;

//=============================================================================
// Matrix / Colour Functions
//=============================================================================

void allColor(uint32_t c)
{
  for (int i = 0; i < 25; i++)
    matrix.setPixelColor(i, c);

  matrix.show();
}

void showColor(byte color)
{
  uint32_t c;

  if (color == 0)
    c = matrix.Color(255, 0, 0);       // RED

  if (color == 1)
    c = matrix.Color(0, 255, 0);       // GREEN

  if (color == 2)
    c = matrix.Color(0, 0, 255);       // BLUE

  if (color == 3)
    c = matrix.Color(255, 180, 0);     // YELLOW

  allColor(c);

  tone(BUZZER_PIN, 400 + color * 250, 200);
  delay(300);

  matrix.clear();
  matrix.show();
  delay(150);
}

//=============================================================================
// Touch Sensor
//=============================================================================

bool touchPressed()
{
  bool touchState = digitalRead(TOUCH_PIN);

  bool pressed =
    (lastTouchState == HIGH && touchState == LOW);

  lastTouchState = touchState;

  return pressed;
}

//=============================================================================
// Player Selection
//=============================================================================

byte waitForSelection()
{
  byte selected = 0;

  // Start with RED.
  showSelection(selected);

  while (true) {

    //=========================================================
    // Touch -> cycle to next colour
    //=========================================================

    if (touchPressed()) {

      selected++;

      if (selected > 3)
        selected = 0;

      // Short beep for every touch.
      digitalWrite(BUZZER_PIN, HIGH);
      delay(100);
      digitalWrite(BUZZER_PIN, LOW);

      // Simple debounce.
      delay(200);

      // Show the newly selected colour.
      showSelection(selected);
    }

    //=========================================================
    // Button -> confirm colour
    //=========================================================

    if (digitalRead(BUTTON_PIN) == LOW) {

      // Wait for button release.
      delay(150);

      while (digitalRead(BUTTON_PIN) == LOW)
        delay(10);

      return selected;
    }

    delay(10);
  }
}

//=============================================================================
// Show Currently Selected Colour
//=============================================================================

void showSelection(byte color)
{
  uint32_t c;

  if (color == 0)
    c = matrix.Color(255, 0, 0);       // RED

  if (color == 1)
    c = matrix.Color(0, 255, 0);       // GREEN

  if (color == 2)
    c = matrix.Color(0, 0, 255);       // BLUE

  if (color == 3)
    c = matrix.Color(255, 180, 0);     // YELLOW

  allColor(c);
}

//=============================================================================
// Failure / Success
//=============================================================================

void failure()
{
  for (int i = 0; i < 3; i++) {

    allColor(matrix.Color(255, 0, 0));

    tone(BUZZER_PIN, 150, 150);
    delay(200);

    matrix.clear();
    matrix.show();
    delay(100);
  }
}

void success()
{
  allColor(matrix.Color(0, 255, 0));

  tone(BUZZER_PIN, 1200, 200);
  delay(300);

  matrix.clear();
  matrix.show();
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
  Serial.begin(115200);

  // Button.
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // Touch sensor.
  pinMode(TOUCH_PIN, INPUT_PULLUP);

  // Buzzer.
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Matrix.
  matrix.begin();
  matrix.setBrightness(7);
  matrix.clear();
  matrix.show();

  randomSeed((uint32_t)esp_random());

  // Generate sequence.
  for (int i = 0; i < MAX_SEQUENCE; i++)
    sequence[i] = random(0, 4);

  Serial.println("Simon Says started.");
}

//=============================================================================
// Main Game Loop
//=============================================================================

void loop()
{
  Serial.print("Round: ");
  Serial.println(sequenceLength);

  //===========================================================
  // Show the entire sequence.
  //===========================================================

  for (int i = 0; i < sequenceLength; i++) {

    showColor(sequence[i]);

    delay(150);
  }

  //===========================================================
  // Player reproduces the sequence.
  //===========================================================

  for (int i = 0; i < sequenceLength; i++) {

    byte answer = waitForSelection();

    Serial.print("Selected: ");
    Serial.println(answer);

    if (answer != sequence[i]) {

      Serial.println("Wrong!");

      failure();

      sequenceLength = 1;

      delay(1000);

      return;
    }
  }

  //===========================================================
  // Round completed successfully.
  //===========================================================

  Serial.println("Correct!");

  success();

  sequenceLength++;

  if (sequenceLength > MAX_SEQUENCE)
    sequenceLength = MAX_SEQUENCE;

  delay(800);
}
