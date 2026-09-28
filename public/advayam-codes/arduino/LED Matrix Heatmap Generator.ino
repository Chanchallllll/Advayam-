/*
 * This program displays the LMT87 temperature as a vertical
 * heatmap on the 5x5 RGB matrix.
 *
 * Lower temperatures produce blue colors while higher
 * temperatures progress through green and yellow to red.
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

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);
}

void loop()
{
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

  // Convert temperature to heat level
  int heat =
    constrain(
      (int)((temperatureC - 15.0) * 10),
      0,
      255
    );

  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    // Bottom is hottest
    int rowHeat =
      heat - y * 35;

    rowHeat =
      constrain(
        rowHeat,
        0,
        255
      );

    uint32_t color;

    if (rowHeat < 85)
    {
      color = matrix.Color(
        0,
        rowHeat * 3,
        255
      );
    }
    else if (rowHeat < 170)
    {
      color = matrix.Color(
        (rowHeat - 85) * 3,
        255,
        255 - (rowHeat - 85) * 3
      );
    }
    else
    {
      color = matrix.Color(
        255,
        255 - (rowHeat - 170) * 3,
        0
      );
    }

    for (int x = 0; x < 5; x++)
    {
      matrix.setPixelColor(
        pixelXY(x, y),
        color
      );
    }
  }

  matrix.show();

  delay(500);
}
