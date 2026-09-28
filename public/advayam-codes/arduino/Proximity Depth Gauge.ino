/*
 * =============================================================================
 * PROXIMITY DEPTH GAUGE
 * =============================================================================
 * Displays 1 to 5 illuminated rows on the 5x5 RGB matrix as a hand gets
 * closer to the RPR-0521RS.
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

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

void setRow(int row, uint32_t color) {
  for (int col = 0; col < 5; col++) {

    // Convert row/column coordinates to the serpentine matrix index.
    int index;

    if (row % 2 == 0)
      index = row * 5 + col;
    else
      index = row * 5 + (4 - col);

    matrix.setPixelColor(index, color);
  }
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

    // These thresholds should be calibrated for your board.
    int rows = 0;

    if (proximity > 100) rows = 1;
    if (proximity > 300) rows = 2;
    if (proximity > 600) rows = 3;
    if (proximity > 1000) rows = 4;
    if (proximity > 1500) rows = 5;

    matrix.clear();

    for (int row = 0; row < rows; row++) {
      setRow(row, matrix.Color(0, 150, 255));
    }

    matrix.show();
  }

  delay(80);
}
