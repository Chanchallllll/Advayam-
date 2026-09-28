/*
 * =============================================================================
 * DAYLIGHT EXPOSURE METER
 * =============================================================================
 * Tracks cumulative time spent above a configurable ambient-light threshold.
 *
 * The accumulated exposure time is reported over UART.
 *
 * This is an electronics demonstration and is NOT a medical or UV exposure
 * measurement device. The RPR-0521RS measures visible/ambient light, not UV.
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define BRIGHT_LUX_THRESHOLD 300

RPR0521RS sensor;

unsigned long lastUpdate = 0;
unsigned long brightMilliseconds = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  lastUpdate = millis();

  Serial.println("Daylight Exposure Meter started.");
}

void loop() {
  unsigned short proximity;
  float lux;

  unsigned long now = millis();
  unsigned long elapsed = now - lastUpdate;
  lastUpdate = now;

  if (sensor.get_psalsval(&proximity, &lux) == 0) {

    // Accumulate time only when the ambient light exceeds the threshold.
    if (lux >= BRIGHT_LUX_THRESHOLD) {
      brightMilliseconds += elapsed;
    }

    unsigned long totalSeconds =
      brightMilliseconds / 1000;

    unsigned long minutes =
      totalSeconds / 60;

    unsigned long seconds =
      totalSeconds % 60;

    Serial.print("Lux: ");
    Serial.print(lux, 1);

    Serial.print(" | Bright exposure: ");
    Serial.print(minutes);
    Serial.print(" min ");
    Serial.print(seconds);
    Serial.println(" sec");
  }

  delay(1000);
}
