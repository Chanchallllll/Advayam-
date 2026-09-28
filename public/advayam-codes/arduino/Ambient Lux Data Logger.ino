/*
 * =============================================================================
 * AMBIENT LUX DATA LOGGER
 * =============================================================================
 * Samples ambient light every minute and stores the readings in the ESP32-S3
 * internal flash filesystem using LittleFS.
 *
 * Log file:
 *   /lux_log.csv
 *
 * NOTE:
 * The ESP32 does not have a battery-backed real-time clock in this setup.
 * Therefore the timestamp below is elapsed time since the board started.
 * =============================================================================
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <LittleFS.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define LOG_INTERVAL 60000UL

RPR0521RS sensor;

unsigned long lastLogTime = 0;
unsigned long elapsedMinutes = 0;

void setup() {
  Serial.begin(115200);

  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  // Mount the internal flash filesystem.
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS initialization failed!");
    while (1);
  }

  // Create the CSV header if the file doesn't already exist.
  if (!LittleFS.exists("/lux_log.csv")) {
    File file = LittleFS.open("/lux_log.csv", FILE_WRITE);

    if (file) {
      file.println("Elapsed_Minutes,Lux");
      file.close();
    }
  }

  Serial.println("Ambient Lux Logger started.");
}

void loop() {

  if (millis() - lastLogTime >= LOG_INTERVAL) {

    lastLogTime = millis();
    elapsedMinutes++;

    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(&proximity, &lux) == 0) {

      File file = LittleFS.open("/lux_log.csv", FILE_APPEND);

      if (file) {
        file.print(elapsedMinutes);
        file.print(",");
        file.println(lux, 2);
        file.close();

        Serial.print("Logged: ");
        Serial.print(elapsedMinutes);
        Serial.print(" min, ");
        Serial.print(lux, 2);
        Serial.println(" lux");
      } else {
        Serial.println("Failed to open log file.");
      }
    }
  }

  delay(100);
}
