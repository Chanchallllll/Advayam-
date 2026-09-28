/*
 * =============================================================================
 * TOUCHLESS PROXIMITY SWITCH
 * =============================================================================
 * Wave a hand near the RPR-0521RS to toggle the onboard User LED.
 *
 * The RPR-0521RS does not directly report distance in centimeters, so
 * PROXIMITY_THRESHOLD must be calibrated experimentally.
 *
 * User LED:
 *   GPIO 45
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6
#define USER_LED 45

// Increase/decrease this value after testing your sensor.
#define PROXIMITY_THRESHOLD 500

RPR0521RS sensor;

bool ledState = false;
bool handWasNear = false;

void setup() {
  Serial.begin(115200);

  pinMode(USER_LED, OUTPUT);
  digitalWrite(USER_LED, LOW);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  Serial.println("Touchless Proximity Switch started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    bool handIsNear = proximity > PROXIMITY_THRESHOLD;

    // Toggle only when the hand ENTERS the detection area.
    // This prevents repeated toggling while the hand remains there.
    if (handIsNear && !handWasNear) {
      ledState = !ledState;

      digitalWrite(USER_LED, ledState ? HIGH : LOW);

      Serial.print("LED: ");
      Serial.println(ledState ? "ON" : "OFF");
    }

    handWasNear = handIsNear;
  }

  delay(50);
}
