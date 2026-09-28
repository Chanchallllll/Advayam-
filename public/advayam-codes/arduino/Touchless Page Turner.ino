/*
 * =============================================================================
 * TOUCHLESS PAGE TURNER
 * =============================================================================
 * Generates UART commands for controlling presentation slides.
 *
 * IMPORTANT:
 * The single RPR-0521RS sensor cannot determine actual left/right movement.
 * This implementation interprets rapid proximity changes as gestures:
 *
 *   Rapid approach -> NEXT
 *   Rapid retreat  -> PREV
 *
 * Example UART commands:
 *   NEXT
 *   PREV
 *
 * A PC-side program can translate these commands into keyboard actions.
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define GESTURE_CHANGE 300
#define GESTURE_TIMEOUT 500

RPR0521RS sensor;

unsigned short previousProximity = 0;
unsigned long lastGesture = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Touchless Page Turner ready.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    int change = (int)proximity - (int)previousProximity;

    if (millis() - lastGesture > GESTURE_TIMEOUT) {

      // Large positive change = hand rapidly approaching.
      if (change > GESTURE_CHANGE) {
        Serial.println("NEXT");
        lastGesture = millis();
      }

      // Large negative change = hand rapidly moving away.
      else if (change < -GESTURE_CHANGE) {
        Serial.println("PREV");
        lastGesture = millis();
      }
    }

    previousProximity = proximity;
  }

  delay(40);
}
