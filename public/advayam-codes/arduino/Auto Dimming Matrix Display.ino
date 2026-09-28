/*
 * =============================================================================
 * AUTO-DIMMING MATRIX DISPLAY
 * =============================================================================
 * Reads ambient light from the ROHM RPR-0521RS sensor and automatically
 * adjusts the brightness of the 5x5 WS2812B RGB matrix.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * RGB Matrix:
 *   DATA = GPIO 7
 *
 * Sensor:
 *   RPR-0521RS
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

RPR0521RS rpr0521rs;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  Serial.begin(115200);

  // Start the I2C bus using the board's dedicated pins.
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize the proximity/ambient-light sensor.
  if (rpr0521rs.init() != 0) {
    Serial.println("RPR-0521RS initialization failed!");
    while (1);
  }

  matrix.begin();
  matrix.clear();
  matrix.show();

  Serial.println("Auto-Dimming Matrix Display started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  // Read ambient light and proximity data.
  if (rpr0521rs.get_psalsval(&proximity, &lux) == 0) {

    // Convert the lux reading into a useful LED brightness.
    // Very dark = dim display.
    // Very bright = bright display.
    int brightness = map((int)constrain(lux, 0, 500),
                         0, 500,
                         10, 255);

    matrix.setBrightness(brightness);

    // Display a soft blue color.
    for (int i = 0; i < NUM_PIXELS; i++) {
      matrix.setPixelColor(i, matrix.Color(0, 80, 255));
    }

    matrix.show();

    Serial.print("Lux: ");
    Serial.print(lux, 2);
    Serial.print(" | Brightness: ");
    Serial.println(brightness);
  }

  delay(300);
}