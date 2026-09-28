/*
 * =============================================================================
 * BLIND SPOT PROXIMITY WARNING
 * =============================================================================
 * Provides an audible warning when an object approaches the RPR-0521RS.
 *
 * Farther obstacle -> slow intermittent beep
 * Closer obstacle  -> faster beep
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

unsigned long lastBeep = 0;
bool buzzerOn = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    if (proximity > 100) {

      // Map proximity to beep interval.
      // High proximity = short interval = faster beeping.
      int interval = map(
        constrain(proximity, 100, 2000),
        100, 2000,
        1000, 100
      );

      if (millis() - lastBeep >= interval) {

        lastBeep = millis();

        if (!buzzerOn) {
          tone(BUZZER_PIN, 1800);
          buzzerOn = true;
        } else {
          noTone(BUZZER_PIN);
          buzzerOn = false;
        }
      }

    } else {
      noTone(BUZZER_PIN);
      buzzerOn = false;
    }
  }

  delay(20);
}
