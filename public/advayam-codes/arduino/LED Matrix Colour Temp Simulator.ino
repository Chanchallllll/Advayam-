/*
 * =============================================================================
 * COLOR TEMPERATURE SIMULATOR FROM AMBIENT LUX
 * =============================================================================
 * Adjusts the RGB matrix color according to ambient light:
 *
 * Bright room -> cool white
 * Dim room    -> warm amber
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

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();
  matrix.setBrightness(100);
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    // Convert 0-500 lux into a warm-to-cool color transition.
    float ratio = constrain(lux, 0.0, 500.0) / 500.0;

    int red = 255;
    int green = 80 + (int)(175 * ratio);
    int blue = (int)(255 * ratio);

    uint32_t color = matrix.Color(red, green, blue);

    for (int i = 0; i < NUM_PIXELS; i++)
      matrix.setPixelColor(i, color);

    matrix.show();

    Serial.print("Lux: ");
    Serial.print(lux);
    Serial.print(" | RGB: ");
    Serial.print(red);
    Serial.print(",");
    Serial.print(green);
    Serial.print(",");
    Serial.println(blue);
  }

  delay(300);
}
