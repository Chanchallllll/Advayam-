/*
 * =============================================================================
 * TOUCHLESS GESTURE MODE SWITCH (FreeRTOS Version)
 * =============================================================================
 * Attempts to detect touchless hand gestures via background FreeRTOS task
 * and cycle through RGB matrix display modes.
 */

#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

#define GESTURE_THRESHOLD 500

RPR0521RS sensor;
Adafruit_NeoPixel matrix(NUM_PIXELS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

int displayMode = 0;
unsigned short previousProximity = 0;
unsigned long lastGestureTime = 0;

void showMode() {
  matrix.clear();
  uint32_t color;

  if (displayMode == 0)
    color = matrix.Color(255, 0, 0);
  else if (displayMode == 1)
    color = matrix.Color(0, 255, 0);
  else
    color = matrix.Color(0, 0, 255);

  for (int i = 0; i < NUM_PIXELS; i++) {
    matrix.setPixelColor(i, color);
  }

  matrix.show();
}

// FreeRTOS Task for sensor polling and gesture evaluation
void gestureTask(void *pvParameters)
{
  while (true)
  {
    unsigned short proximity;
    float lux;

    if (sensor.get_psalsval(&proximity, &lux) == 0) {
      if (proximity > GESTURE_THRESHOLD &&
          previousProximity < GESTURE_THRESHOLD &&
          millis() - lastGestureTime > 700) {

        displayMode++;
        if (displayMode > 2)
          displayMode = 0;

        showMode();
        lastGestureTime = millis();

        Serial.print("Display mode: ");
        Serial.println(displayMode);
      }

      previousProximity = proximity;
    }

    vTaskDelay(pdMS_TO_TICKS(40));
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  if (sensor.init() != 0) {
    Serial.println("Sensor initialization failed!");
    while (1);
  }

  matrix.begin();
  matrix.setBrightness(10);
  showMode();

  Serial.println("Touchless Gesture Mode started.");

  xTaskCreate(
    gestureTask,
    "GestureTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop() {
  vTaskDelete(NULL);
}