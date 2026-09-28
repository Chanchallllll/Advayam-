/*
 * =============================================================================
 * TUG OF WAR - PLAYER VS CPU
 * =============================================================================
 *
 * PLAYER:
 *   Press the button repeatedly to pull the marker toward the player.
 *
 * CPU:
 *   Automatically pulls toward its side.
 *
 * WIN CONDITION:
 *   The player must keep the marker on the player's side
 *   (position 0 or 1) continuously for 3 seconds.
 *
 * DIFFICULTY:
 *   The CPU gradually gets faster as the player makes progress.
 *
 *   The CPU has a minimum delay so the game remains realistically playable.
 *
 * Reach position 0 -> begins the 3-second hold.
 * Reach position 4 -> CPU wins.
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define BUTTON_PIN 17
#define BUZZER_PIN 16

Adafruit_NeoPixel matrix(
  25,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Game Variables
//=============================================================================

int marker = 2;

// CPU starts at 450 ms.
unsigned long cpuInterval = 450;

// CPU can get faster, but never faster than this.
const unsigned long MIN_CPU_INTERVAL = 280;

// How much faster the CPU becomes after each player press.
const unsigned long CPU_SPEEDUP = 8;

unsigned long lastCPU = 0;

//=============================================================================
// Player Hold
//=============================================================================

bool holdingPlayerSide = false;

unsigned long playerHoldStart = 0;

// Player must hold the marker on their side for 3 seconds.
const unsigned long PLAYER_HOLD_TIME = 3000;

//=============================================================================
// Matrix
//=============================================================================

int xy(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void draw()
{
  matrix.clear();

  //------------------------------------------------------------------------
  // Draw the player's side in green.
  //------------------------------------------------------------------------

  for (int x = 0; x < marker; x++) {

    matrix.setPixelColor(
      xy(x, 2),
      matrix.Color(0, 255, 0)
    );
  }

  //------------------------------------------------------------------------
  // Draw the CPU side in red.
  //------------------------------------------------------------------------

  for (int x = marker + 1; x < 5; x++) {

    matrix.setPixelColor(
      xy(x, 2),
      matrix.Color(255, 0, 0)
    );
  }

  //------------------------------------------------------------------------
  // Draw marker.
  //------------------------------------------------------------------------

  matrix.setPixelColor(
    xy(marker, 2),
    matrix.Color(255, 255, 255)
  );

  matrix.show();
}

//=============================================================================
// Reset
//=============================================================================

void resetGame()
{
  marker = 2;

  cpuInterval = 450;

  lastCPU = millis();

  holdingPlayerSide = false;
  playerHoldStart = 0;

  draw();
}

//=============================================================================
// End Game
//=============================================================================

void endGame(bool playerWon)
{
  uint32_t c =
    playerWon
      ? matrix.Color(0, 255, 0)
      : matrix.Color(255, 0, 0);

  //------------------------------------------------------------------------
  // Flash result.
  //------------------------------------------------------------------------

  for (int i = 0; i < 3; i++) {

    for (int j = 0; j < 25; j++)
      matrix.setPixelColor(j, c);

    matrix.show();

    tone(
      BUZZER_PIN,
      playerWon ? 1200 : 150,
      120
    );

    delay(180);

    matrix.clear();
    matrix.show();

    delay(100);
  }

  delay(500);

  resetGame();
}

//=============================================================================
// Player Pull
//=============================================================================

void playerPull()
{
  //------------------------------------------------------------------------
  // Pull toward player.
  //------------------------------------------------------------------------

  if (marker > 0) {

    marker--;

    tone(
      BUZZER_PIN,
      900,
      30
    );

    //------------------------------------------------------------------------
    // Every successful pull makes the CPU slightly faster.
    //------------------------------------------------------------------------

    if (cpuInterval > MIN_CPU_INTERVAL) {

      if (cpuInterval > CPU_SPEEDUP)
        cpuInterval -= CPU_SPEEDUP;
    }
  }

  //------------------------------------------------------------------------
  // Reaching position 0 starts the hold timer.
  //------------------------------------------------------------------------

  if (marker <= 0) {

    marker = 0;

    if (!holdingPlayerSide) {

      holdingPlayerSide = true;

      playerHoldStart = millis();

      Serial.println(
        "PLAYER SIDE! Hold for 3 seconds!"
      );

      tone(
        BUZZER_PIN,
        1100,
        50
      );
    }
  }
}

//=============================================================================
// Check Player Hold
//=============================================================================

bool checkPlayerHold()
{
  if (!holdingPlayerSide)
    return false;

  // If CPU pulls the marker away, the timer is cancelled.
  if (marker > 1) {

    holdingPlayerSide = false;
    playerHoldStart = 0;

    Serial.println(
      "Hold interrupted!"
    );

    return false;
  }

  // Check whether 3 seconds have passed.
  if (millis() - playerHoldStart >= PLAYER_HOLD_TIME) {

    Serial.println(
      "PLAYER WINS!"
    );

    endGame(true);

    return true;
  }

  return false;
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

  resetGame();

  Serial.println(
    "Tug of War started."
  );
}

//=============================================================================
// Main Loop
//=============================================================================

void loop()
{
  //===========================================================================
  // BUTTON
  //===========================================================================

  static bool oldButton = HIGH;

  bool button =
    digitalRead(BUTTON_PIN);

  // Detect a new button press.
  if (button == LOW &&
      oldButton == HIGH) {

    playerPull();
  }

  oldButton = button;

  //===========================================================================
  // CPU
  //===========================================================================

  if (millis() - lastCPU >= cpuInterval) {

    lastCPU = millis();

    // CPU pulls toward position 4.
    if (marker < 4) {

      marker++;

      // If the player was holding their side,
      // the hold is immediately broken.
      if (marker > 1 &&
          holdingPlayerSide) {

        holdingPlayerSide = false;
        playerHoldStart = 0;

        Serial.println(
          "CPU broke the hold!"
        );
      }
    }

    // CPU wins.
    if (marker >= 4) {

      marker = 4;

      Serial.println(
        "CPU WINS!"
      );

      endGame(false);

      return;
    }
  }

  //===========================================================================
  // PLAYER HOLD
  //===========================================================================

  if (checkPlayerHold())
    return;

  //===========================================================================
  // DISPLAY
  //===========================================================================

  draw();

  delay(10);
}
