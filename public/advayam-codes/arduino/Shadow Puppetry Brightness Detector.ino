/*
 * =============================================================================
 * SHADOW PUPPETRY BRIGHTNESS DETECTOR
 * =============================================================================
 * Detects rapid changes in ambient brightness using the RPR-0521RS.
 *
 * A sudden reduction in light produces a low sound effect.
 * A sudden increase in light produces a higher sound effect.
 *
 * This can be used for simple "shadow puppetry" experiments.
 *
 * Buzzer:
 *   GPIO 16
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define BUZZER_PIN 16

RPR0521RS sensor;

float previousLux = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Shadow Puppetry detector started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    float change = lux - previousLux;

    // A sufficiently large negative change indicates a shadow.
    if (change < -20) {
      tone(BUZZER_PIN, 300, 150);
      Serial.println("Shadow detected!");
    }

    // A sufficiently large positive change indicates the shadow disappeared.
    else if (change > 20) {
      tone(BUZZER_PIN, 1200, 150);
      Serial.println("Light restored!");
    }

    previousLux = lux;
  }

  delay(80);
}
