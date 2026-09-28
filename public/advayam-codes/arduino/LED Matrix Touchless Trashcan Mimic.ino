/*
 * =============================================================================
 * TOUCHLESS TRASHCAN LID MIMIC
 * =============================================================================
 * Hold a hand over the proximity sensor for approximately one second.
 * The matrix performs an animation that mimics a trashcan lid opening.
 *
 * Matrix:
 *   GPIO 7
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

unsigned long hoverStart = 0;
bool animationPlayed = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();
  matrix.setBrightness(20);
  matrix.clear();
  matrix.show();
}

void animateLid() {
  // Gradually light rows from bottom to top.
  for (int row = 0; row < 5; row++) {

    for (int col = 0; col < 5; col++) {
      int index;

      if (row % 2 == 0)
        index = row * 5 + col;
      else
        index = row * 5 + (4 - col);

      matrix.setPixelColor(index, matrix.Color(255, 120, 20));
    }

    matrix.show();
    delay(120);
  }

  delay(500);

  // Close animation.
  for (int row = 4; row >= 0; row--) {

    for (int col = 0; col < 5; col++) {
      int index;

      if (row % 2 == 0)
        index = row * 5 + col;
      else
        index = row * 5 + (4 - col);

      matrix.setPixelColor(index, 0);
    }

    matrix.show();
    delay(120);
  }
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    if (proximity > PROXIMITY_THRESHOLD) {

      // Start the hover timer when the hand first appears.
      if (hoverStart == 0)
        hoverStart = millis();

      // Require a continuous one-second hover.
      if (!animationPlayed &&
          millis() - hoverStart >= 1000) {

        animateLid();
        animationPlayed = true;
      }

    } else {

      // Reset when the hand leaves.
      hoverStart = 0;
      animationPlayed = false;
    }
  }

  delay(50);
}
