/*
 * This program displays happy, neutral, or sad faces
 * on the RGB matrix depending on the LMT87 temperature
 * monitored inside a dedicated FreeRTOS task.
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

#define HAPPY_MIN_TEMP     15.0
#define HAPPY_MAX_TEMP     28.0
#define FROWN_TEMP         35.0

const uint8_t faces[3][5] =
{
  {
    0b01010,
    0b00000,
    0b00000,
    0b10001,
    0b01110
  },
  {
    0b01010,
    0b00000,
    0b00000,
    0b00000,
    0b11111
  },
  {
    0b01010,
    0b00000,
    0b00000,
    0b01110,
    0b10001
  }
};

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void drawFace(int face)
{
  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      if (faces[face][y] & (1 << (4 - x)))
      {
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

// FreeRTOS Task for temperature monitoring and face updating
void tempFaceTask(void *pvParameters)
{
  while (true)
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

    int face;

    if (temperatureC < HAPPY_MIN_TEMP)
    {
      face = 1; 
    }
    else if (temperatureC < HAPPY_MAX_TEMP)
    {
      face = 0; 
    }
    else if (temperatureC < FROWN_TEMP)
    {
      face = 1; 
    }
    else
    {
      face = 2; 
    }

    drawFace(face);

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  xTaskCreate(
    tempFaceTask,
    "TempFaceTask",
    2048,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  vTaskDelete(NULL);
}