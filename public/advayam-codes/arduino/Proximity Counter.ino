/*
 * =============================================================================
 * PROXIMITY COUNTER
 * =============================================================================
 * Counts how many times an object enters the configured proximity range.
 *
 * Each object must leave the detection area before another count can occur.
 *
 * UART:
 *   USB Serial at 115200 baud
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;

unsigned long objectCount = 0;
bool objectPresent = false;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Proximity Counter started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    bool detected = proximity > PROXIMITY_THRESHOLD;

    // Count only the transition from "not detected" to "detected".
    if (detected && !objectPresent) {
      objectCount++;

      Serial.print("Object detected! Total count: ");
      Serial.println(objectCount);
    }

    objectPresent = detected;
  }

  delay(50);
}
