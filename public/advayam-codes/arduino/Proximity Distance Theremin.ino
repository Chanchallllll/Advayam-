/*
 * =============================================================================
 * PROXIMITY DISTANCE THEREMIN
 * =============================================================================
 * Uses the RPR-0521RS proximity reading to control the pitch of a buzzer.
 *
 * Hand closer  -> higher pitch
 * Hand farther  -> lower pitch
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

// Adjust these values for your particular sensor/environment.
#define MIN_PROXIMITY 50
#define MAX_PROXIMITY 2000

#define MIN_FREQUENCY 150
#define MAX_FREQUENCY 2500

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Proximity Theremin started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    // Limit the sensor reading to our calibrated range.
    int p = constrain(proximity, MIN_PROXIMITY, MAX_PROXIMITY);

    // Convert proximity into frequency.
    int frequency = map(p,
                        MIN_PROXIMITY,
                        MAX_PROXIMITY,
                        MIN_FREQUENCY,
                        MAX_FREQUENCY);

    // Only make a sound when an object is sufficiently close.
    if (proximity > MIN_PROXIMITY) {
      tone(BUZZER_PIN, frequency);
    } else {
      noTone(BUZZER_PIN);
    }

    Serial.print("Proximity: ");
    Serial.print(proximity);
    Serial.print(" | Frequency: ");
    Serial.println(frequency);
  }

  delay(30);
}
