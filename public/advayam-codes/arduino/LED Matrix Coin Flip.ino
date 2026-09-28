/*
  Coin Flip Simulator

  Press the button to flip the coin.
  The matrix shows a spinning coin animation,
  then stops on HEADS or TAILS and plays a chime.
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


// Convert x/y position to LED number
int pixelXY(int x, int y) {

  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


// Set one matrix pixel
void setPixelXY(int x, int y, uint32_t color) {

  if (x < 0 || x >= 5 ||
      y < 0 || y >= 5)
    return;

  matrix.setPixelColor(
    pixelXY(x, y),
    color
  );
}


// Draw the coin
// mode:
// 0 = heads
// 1 = tails
// 2 = thin coin
// 3 = blank
void drawCoin(int mode) {

  matrix.clear();

  uint32_t color =
    matrix.Color(255, 180, 0);


  // -------------------------
  // Normal coin
  // -------------------------
  if (mode == 0 || mode == 1) {

    // Coin outline
    for (int x = 1; x <= 3; x++) {
      setPixelXY(x, 0, color);
      setPixelXY(x, 4, color);
    }

    for (int y = 1; y <= 3; y++) {
      setPixelXY(0, y, color);
      setPixelXY(4, y, color);
    }


    // HEADS
    if (mode == 0) {

      setPixelXY(2, 1, color);
      setPixelXY(1, 2, color);
      setPixelXY(2, 2, color);
      setPixelXY(3, 2, color);
      setPixelXY(2, 3, color);
    }


    // TAILS
    else {

      setPixelXY(1, 1, color);
      setPixelXY(3, 1, color);
      setPixelXY(2, 2, color);
      setPixelXY(1, 3, color);
      setPixelXY(3, 3, color);
    }
  }


  // -------------------------
  // Coin viewed edge-on
  // -------------------------
  else if (mode == 2) {

    for (int y = 0; y < 5; y++) {
      setPixelXY(2, y, color);
    }
  }


  matrix.show();
}


// Coin flipping animation
void flipAnimation(bool result) {

  /*
    The animation starts fast and gradually slows down.
    Heads and tails alternate like a spinning coin.
  */

  int delayTime = 60;

  // Number of flips
  int totalFlips = 12;

  for (int i = 0; i < totalFlips; i++) {

    // Show heads
    drawCoin(0);
    delay(delayTime);

    // Show edge-on
    drawCoin(2);
    delay(delayTime / 2);

    // Show tails
    drawCoin(1);
    delay(delayTime);

    // Show edge-on
    drawCoin(2);
    delay(delayTime / 2);


    // Gradually slow down
    if (i > 5)
      delayTime += 25;
    else
      delayTime += 8;
  }


  // Final result
  drawCoin(result ? 0 : 1);
}


// Play a short chime
void playChime(bool heads) {

  if (heads) {

    tone(BUZZER_PIN, 880, 150);
    delay(170);

    tone(BUZZER_PIN, 1175, 180);
  }

  else {

    tone(BUZZER_PIN, 660, 150);
    delay(170);

    tone(BUZZER_PIN, 440, 180);
  }
}


void setup() {

  Serial.begin(115200);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(BUZZER_PIN, OUTPUT);

  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  randomSeed(micros());

  Serial.println("Coin Flip Simulator");
  Serial.println("Press button to flip.");
}


void loop() {

  bool button = digitalRead(BUTTON_PIN);


  // Detect button press
  if (lastButton == HIGH && button == LOW) {

    // Decide result BEFORE animation
    // but don't reveal it until animation ends
    bool heads = random(0, 2);


    Serial.println("Flipping...");


    // Play animation
    flipAnimation(heads);


    // Reveal result
    if (heads)
      Serial.println("HEADS");
    else
      Serial.println("TAILS");


    // Play result sound
    playChime(heads);


    // Small debounce delay
    delay(300);
  }


  lastButton = button;
}
