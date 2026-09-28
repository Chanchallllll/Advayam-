/*
 * =============================================================================
 * NIGHT LIGHT CONTROLLER
 * =============================================================================
 * Turns the RGB matrix into a soft warm-white night light when ambient
 * brightness falls below DARK_LUX_THRESHOLD.
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

#define DARK_LUX_THRESHOLD 20

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();

  // Low overall brightness makes the night light comfortable.
  matrix.setBrightness(30);
  matrix.clear();
  matrix.show();
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    matrix.clear();

    if (lux < DARK_LUX_THRESHOLD) {

      // Warm-white is produced using high red, medium green and low blue.
      uint32_t warmWhite = matrix.Color(255, 120, 30);

      for (int i = 0; i < NUM_PIXELS; i++) {
        matrix.setPixelColor(i, warmWhite);
      }
    }

    matrix.show();

    Serial.print("Ambient lux: ");
    Serial.println(lux);
  }

  delay(500);
}
