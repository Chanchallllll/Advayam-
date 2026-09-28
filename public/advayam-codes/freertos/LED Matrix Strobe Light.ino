/*
 * LED Matrix Strobe Light (FreeRTOS Version)
 *
 * This program uses a touch sensor connected to GPIO 8 to control 
 * a strobe effect on a 25-pixel NeoPixel LED matrix.
 *
 * The first sensor touch turns the strobe ON at 1 Hz.
 * Each subsequent touch increases the flashing frequency:
 * 1, 2, 3, 5, 7, 10, 12, and 15 Hz.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7     
#define NUM_LEDS 25      
#define TOUCH_PIN 8      // GPIO pin connected to the touch sensor

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

const int frequencies[] = {
  1, 2, 3, 5, 7, 10, 12, 15
};

int frequencyIndex = 0;  
bool strobeEnabled = false;  
bool lastTouch = HIGH;       

void setMatrix(uint32_t color) {
  for (int i = 0; i < NUM_LEDS; i++) {
    matrix.setPixelColor(i, color);
  }
  matrix.show();
}

// FreeRTOS Task managing touch input detection and strobe execution concurrently
void strobeControlTask(void *pvParameters)
{
  unsigned long lastFlash = 0;
  bool flashState = false;

  while (true)
  {
    // Read the current touch sensor state
    bool touch = digitalRead(TOUCH_PIN);

    // Detect a new touch activation: HIGH -> LOW
    if (lastTouch == HIGH && touch == LOW) {
      if (!strobeEnabled) {
        strobeEnabled = true;
      } 
      else {
        frequencyIndex++;
        if (frequencyIndex >= 8) {
          strobeEnabled = false;
          frequencyIndex = 0;
        }
      }

      Serial.print("Strobe: ");
      if (strobeEnabled) {
        Serial.print("ON, ");
        Serial.print(frequencies[frequencyIndex]);
        Serial.println(" Hz");
      } 
      else {
        Serial.println("OFF");
      }

      vTaskDelay(pdMS_TO_TICKS(100)); // Debounce delay
    }

    lastTouch = touch;

    if (strobeEnabled) {
      unsigned long interval = 1000 / (frequencies[frequencyIndex] * 2);
      unsigned long currentMillis = millis();

      if (currentMillis - lastFlash >= interval) {
        lastFlash = currentMillis;
        flashState = !flashState;

        if (flashState) {
          setMatrix(matrix.Color(255, 255, 255));
        } 
        else {
          setMatrix(matrix.Color(0, 0, 0));
        }
      }
    }

    vTaskDelay(pdMS_TO_TICKS(10)); // Yield to other tasks
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(TOUCH_PIN, INPUT_PULLUP);

  matrix.begin();
  matrix.setBrightness(10);
  matrix.clear();
  matrix.show();

  Serial.println("LED Matrix Strobe (Touch Sensor)");
  Serial.println("Touch GPIO 8 to enable/disable.");

  xTaskCreate(
    strobeControlTask,
    "StrobeTask",
    2048,
    NULL,
    1,
    NULL
  );
}

void loop() {
  vTaskDelete(NULL);
}