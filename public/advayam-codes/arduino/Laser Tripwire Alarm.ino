/*
 * =============================================================================
 * LASER TRIPWIRE / BEAM BREAK ALARM
 * =============================================================================
 * Uses the RPR-0521RS ambient-light sensor as a beam-break detector.
 *
 * A steady light source (e.g, a laser pointer) should illuminate the sensor.
 * When an object blocks the beam, the measured lux drops below the baseline
 * threshold and the buzzer sounds.
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

// Percentage drop from the normal ambient-light level.
// Example: 30 means trigger if light drops by 30%.
#define BEAM_BREAK_PERCENT 30

float baselineLux = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Establishing light baseline...");

  // Average several readings to establish the normal beam level.
  for (int i = 0; i < 20; i++) {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(&proximity, &lux) == 0) {
      baselineLux += lux;
    }

    delay(100);
  }

  baselineLux /= 20.0;

  Serial.print("Baseline lux: ");
  Serial.println(baselineLux);
  Serial.println("Tripwire armed.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    float triggerLevel =
      baselineLux * (1.0 - BEAM_BREAK_PERCENT / 100.0);

    if (lux < triggerLevel) {
      tone(BUZZER_PIN, 2000);

      Serial.print("BEAM BROKEN! Lux = ");
      Serial.println(lux);
    } else {
      noTone(BUZZER_PIN);
    }
  }

  delay(50);
}
