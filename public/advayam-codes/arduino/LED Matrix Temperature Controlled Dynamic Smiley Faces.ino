 /*
  * This program displays happy, neutral, or sad faces
  * on the RGB matrix depending on the LMT87 temperature.
  *
  * The face is displayed upside down.
  * As temperature increases:
  *
  *   Comfortable  -> HAPPY
  *   Warm         -> NEUTRAL
  *   Hot          -> FROWNING
  *
  * Emoji-style yellow face.
  */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LMT87.h>

#define MATRIX_PIN 7
#define TEMP_PIN   1
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

LMT87 lmt87 = LMT87();

int ADCres = 10;
double voltageRef = 3.3;

//====================================================
// Temperature Thresholds
//====================================================

// Comfortable human temperature range
#define HAPPY_MIN_TEMP     15.0
#define HAPPY_MAX_TEMP     28.0

// Above this temperature = frowning
#define FROWN_TEMP         35.0

//====================================================
// Face Patterns
//====================================================

const uint8_t faces[3][5] =
{
  //==================================================
  // HAPPY
  //==================================================
  {
    0b01010,
    0b00000,
    0b00000,
    0b10001,
    0b01110
  },

  //==================================================
  // NEUTRAL
  //==================================================
  {
    0b01010,
    0b00000,
    0b00000,
    0b00000,
    0b11111
  },

  //==================================================
  // FROWNING
  //==================================================
  {
    0b01010,
    0b00000,
    0b00000,
    0b01110,
    0b10001
  }
};

//====================================================
// Convert X/Y coordinates to LED index
//====================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//====================================================
// Draw Face Upside Down
//====================================================

void drawFace(int face)
{
  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      if (faces[face][y] & (1 << (4 - x)))
      {
        // Rotate face 180 degrees
        int flippedX = 4 - x;
        int flippedY = 4 - y;

        matrix.setPixelColor(
          pixelXY(flippedX, flippedY),
          matrix.Color(255, 190, 0)
        );
      }
    }
  }

  matrix.show();
}

//====================================================
// Setup
//====================================================

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();
}

//====================================================
// Main Loop
//====================================================

void loop()
{
  // Read LMT87 temperature
  double temperatureC =
    -lmt87.getCDegree(
      TEMP_PIN,
      ADCres,
      voltageRef,
      100
    );

  Serial.print("Temperature: ");
  Serial.print(temperatureC, 1);
  Serial.println(" C");

  int face;

  //==================================================
  // Temperature-based mood
  //==================================================

  if (temperatureC < HAPPY_MIN_TEMP)
  {
    // Too cold
    face = 1;  // Neutral
  }
  else if (temperatureC < HAPPY_MAX_TEMP)
  {
    // Comfortable human temperature
    face = 0;  // Happy
  }
  else if (temperatureC < FROWN_TEMP)
  {
    // Getting uncomfortably warm
    face = 1;  // Neutral
  }
  else
  {
    // Hot
    face = 2;  // Frowning
  }

  drawFace(face);

  delay(1000);
}
