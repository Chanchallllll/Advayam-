/*
 * =============================================================================
 * PROXIMITY-BASED GREETING MACHINE
 * =============================================================================
 * When someone approaches the sensor:
 *
 *   1. A smiley face appears on the RGB matrix.
 *   2. The buzzer plays a short greeting chime.
 *
 * Matrix:
 *   GPIO 7
 *
 * Buzzer:
 *   GPIO 16
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define BUZZER_PIN 16

#define NUM_PIXELS 25
#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

bool personDetected = false;

void setPixelXY(int x, int y, uint32_t color) {
  int index;

  if (y % 2 == 0)
    index = y * 5 + x;
  else
    index = y * 5 + (4 - x);

  matrix.setPixelColor(index, color);
}

void showSmiley() {
  matrix.clear();

  uint32_t yellow = matrix.Color(255, 180, 0);

  // Eyes.
  setPixelXY(1, 1, yellow);
  setPixelXY(3, 1, yellow);

  // Smile.
  setPixelXY(1, 3, yellow);
  setPixelXY(2, 4, yellow);
  setPixelXY(3, 3, yellow);

  matrix.show();
}

void greetingChime() {
  tone(BUZZER_PIN, 784, 150);
  delay(170);

  tone(BUZZER_PIN, 988, 150);
  delay(170);

  tone(BUZZER_PIN, 1175, 250);
  delay(270);

  noTone(BUZZER_PIN);
}

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

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    bool detected = proximity > PROXIMITY_THRESHOLD;

    // Trigger only when a new person approaches.
    if (detected && !personDetected) {

      Serial.println("Hello!");

      showSmiley();
      greetingChime();
    }

    personDetected = detected;
  }

  delay(50);
}
