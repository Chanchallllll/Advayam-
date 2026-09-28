/*
 * =============================================================================
 * HAND PROXIMITY REACTION GAME
 * =============================================================================
 * A simple reaction-time game using the proximity sensor.
 *
 * Game sequence:
 *
 *   1. Player places hand near sensor.
 *   2. System waits a random amount of time.
 *   3. Buzzer sounds.
 *   4. Player pulls hand away.
 *   5. Reaction time is displayed over UART.
 *
 * Buzzer:
 *   GPIO 16
 *
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define BUZZER_PIN 16

#define HAND_NEAR_THRESHOLD 500
#define HAND_AWAY_THRESHOLD 200

RPR0521RS sensor;

enum GameState {
  WAITING_FOR_HAND,
  COUNTDOWN,
  REACTION
};

GameState state = WAITING_FOR_HAND;

unsigned long countdownStart = 0;
unsigned long reactionStart = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  randomSeed(analogRead(0));

  Serial.println("=== PROXIMITY REACTION GAME ===");
  Serial.println("Place your hand near the sensor.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) != 0)
    return;

  switch (state) {

    case WAITING_FOR_HAND:

      // Player has positioned their hand.
      if (proximity > HAND_NEAR_THRESHOLD) {

        Serial.println("Hand detected.");
        Serial.println("Get ready...");

        // Random delay prevents the player from predicting the buzzer.
        countdownStart =
          millis() + random(1500, 4000);

        state = COUNTDOWN;
      }

      break;

    case COUNTDOWN:

      // Wait until the random countdown expires.
      if (millis() >= countdownStart) {

        tone(BUZZER_PIN, 1500, 200);

        reactionStart = millis();

        Serial.println("GO!");

        state = REACTION;
      }

      break;

    case REACTION:

      // The reaction ends when the hand leaves the detection zone.
      if (proximity < HAND_AWAY_THRESHOLD) {

        unsigned long reactionTime =
          millis() - reactionStart;

        noTone(BUZZER_PIN);

        Serial.print("Reaction time: ");
        Serial.print(reactionTime);
        Serial.println(" ms");

        if (reactionTime < 250)
          Serial.println("Amazing!");
        else if (reactionTime < 400)
          Serial.println("Great!");
        else if (reactionTime < 600)
          Serial.println("Good!");
        else
          Serial.println("Try again!");

        Serial.println();
        Serial.println("Place your hand near the sensor.");

        state = WAITING_FOR_HAND;
      }

      break;
  }

  delay(20);
}
