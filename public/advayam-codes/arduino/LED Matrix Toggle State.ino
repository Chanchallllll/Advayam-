/*
  Button Toggle with Audible Feedback

  Each button press toggles the LED matrix ON or OFF.

  A short buzzer click provides audible feedback
  for each button press.
*/

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define BUTTON_PIN 17
#define BUZZER_PIN 16

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

bool lastButton = HIGH;
bool ledState = false;


// Set the entire matrix
void setMatrix(uint32_t color) {

  for (int i = 0; i < NUM_LEDS; i++) {

    matrix.setPixelColor(
      i,
      color
    );
  }

  matrix.show();
}


// Play a short click
void clickSound() {

  tone(
    BUZZER_PIN,
    1200,
    50
  );
}


void setup() {

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  matrix.begin();
  matrix.setBrightness(25);

  setMatrix(
    matrix.Color(0, 0, 0)
  );

  Serial.println("Button Toggle");
  Serial.println("Press button to toggle LED.");
}


void loop() {

  bool button = digitalRead(BUTTON_PIN);

  // Detect button press
  if (lastButton == HIGH && button == LOW) {

    ledState = !ledState;

    if (ledState) {

      setMatrix(
        matrix.Color(0, 255, 0)
      );

      Serial.println("LED: ON");
    }

    else {

      setMatrix(
        matrix.Color(0, 0, 0)
      );

      Serial.println("LED: OFF");
    }

    // Audible confirmation
    clickSound();

    delay(150);
  }

  lastButton = button;
}
