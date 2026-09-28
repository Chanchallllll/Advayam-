/*
 * =============================================================================
 * TOUCHLESS GESTURE MODE SWITCH
 * =============================================================================
 * Attempts to detect touchless hand gestures and cycle through RGB matrix
 * display modes.
 *
 * IMPORTANT:
 * The RPR-0521RS is a single-point sensor and cannot truly determine whether
 * a hand moved LEFT or RIGHT. This implementation uses changes in proximity
 * as an approach/recede gesture.
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

#define GESTURE_THRESHOLD 500

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

int displayMode = 0;

unsigned short previousProximity = 0;
unsigned long lastGestureTime = 0;

void showMode() {
  matrix.clear();

  uint32_t color;

  if (displayMode == 0)
    color = matrix.Color(255, 0, 0);
  else if (displayMode == 1)
    color = matrix.Color(0, 255, 0);
  else
    color = matrix.Color(0, 0, 255);

  for (int i = 0; i < NUM_PIXELS; i++) {
    matrix.setPixelColor(i, color);
  }

  matrix.show();
}

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();
  matrix.setBrightness(80);

  showMode();

  Serial.println("Touchless Gesture Mode started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    // Detect a substantial increase in proximity.
    if (proximity > GESTURE_THRESHOLD &&
        previousProximity < GESTURE_THRESHOLD &&
        millis() - lastGestureTime > 700) {

      displayMode++;
      if (displayMode > 2)
        displayMode = 0;

      showMode();

      lastGestureTime = millis();

      Serial.print("Display mode: ");
      Serial.println(displayMode);
    }

    previousProximity = proximity;
  }

  delay(40);
}
