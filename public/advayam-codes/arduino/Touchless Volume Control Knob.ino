/*
 * =============================================================================
 * TOUCHLESS VOLUME KNOB
 * =============================================================================
 * Calibrates the current hand position as the "center" position.
 *
 * Move hand closer:
 *   Increase matrix brightness
 *
 * Move hand farther:
 *   Decrease matrix brightness
 *
 * Hold your hand in position for CALIBRATION_TIME to establish the center.
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

#define CALIBRATION_TIME 2000
#define CALIBRATION_THRESHOLD 200

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

unsigned long calibrationStart = 0;
bool calibrated = false;

unsigned short centerProximity = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();

  Serial.println("Hold hand steady to calibrate...");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) != 0)
    return;

  if (!calibrated) {

    if (proximity > CALIBRATION_THRESHOLD) {

      if (calibrationStart == 0)
        calibrationStart = millis();

      if (millis() - calibrationStart >= CALIBRATION_TIME) {

        centerProximity = proximity;
        calibrated = true;

        Serial.print("Calibration complete. Center = ");
        Serial.println(centerProximity);
      }

    } else {
      calibrationStart = 0;
    }

    delay(50);
    return;
  }

  // Difference from the calibrated hand position.
  int difference = proximity - centerProximity;

  // Map movement to brightness.
  int brightness = map(
    constrain(difference, -800, 800),
    -800, 800,
    10, 255
  );

  brightness = constrain(brightness, 10, 255);

  matrix.setBrightness(brightness);

  // Display white so brightness is easy to observe.
  for (int i = 0; i < NUM_PIXELS; i++)
    matrix.setPixelColor(i, matrix.Color(255, 255, 255));

  matrix.show();

  Serial.print("Proximity: ");
  Serial.print(proximity);
  Serial.print(" | Brightness: ");
  Serial.println(brightness);

  delay(50);
}
