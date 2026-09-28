/*
 * ============================================================
 * 5x5 RGB Matrix Starfield (FreeRTOS Version)
 * ============================================================
 *
 * White pixels = normal stars
 * Warm yellow  = rare sunset stars
 *
 * Stars fade smoothly in and out via background FreeRTOS tasks.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25
#define MAX_STARS  6
#define FRAME_DELAY 20

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

struct Star
{
  int x;
  int y;
  float brightness;
  float fadeAmount;
  bool fadingIn;
  bool active;
  bool sunset;
};

Star stars[MAX_STARS];

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

bool positionAvailable(int x, int y)
{
  for (int i = 0; i < MAX_STARS; i++)
  {
    if (!stars[i].active)
      continue;

    int dx = abs(x - stars[i].x);
    int dy = abs(y - stars[i].y);

    if (dx <= 1 && dy <= 1)
      return false;
  }
  return true;
}

bool findPosition(int &x, int &y)
{
  for (int attempt = 0; attempt < 50; attempt++)
  {
    int testX = random(0, 5);
    int testY = random(0, 5);

    if (positionAvailable(testX, testY))
    {
      x = testX;
      y = testY;
      return true;
    }
  }
  return false;
}

void createStar(int index)
{
  int x, y;
  if (!findPosition(x, y))
    return;

  stars[index].x = x;
  stars[index].y = y;
  stars[index].brightness = 0;
  stars[index].fadeAmount = random(8, 16) / 10.0;
  stars[index].fadingIn = true;
  stars[index].active = true;
  stars[index].sunset = (random(100) < 10);
}

float smoothFade(float x)
{
  return x * x * (3.0 - 2.0 * x);
}

void updateStars()
{
  for (int i = 0; i < MAX_STARS; i++)
  {
    if (!stars[i].active)
    {
      if (random(100) < 9)
      {
        createStar(i);
      }
      continue;
    }

    if (stars[i].fadingIn)
    {
      stars[i].brightness += stars[i].fadeAmount;
      if (stars[i].brightness >= 255.0)
      {
        stars[i].brightness = 255.0;
        stars[i].fadingIn = false;
      }
    }
    else
    {
      stars[i].brightness -= stars[i].fadeAmount;
      if (stars[i].brightness <= 0.0)
      {
        stars[i].brightness = 0.0;
        stars[i].active = false;
      }
    }
  }
}

void drawStars()
{
  matrix.clear();

  for (int i = 0; i < MAX_STARS; i++)
  {
    if (!stars[i].active)
      continue;

    float normalized = stars[i].brightness / 255.0;
    float smooth = smoothFade(normalized);
    int b = (int)(smooth * 255.0);
    uint32_t color;

    if (!stars[i].sunset)
    {
      color = matrix.Color(b, b, b);
    }
    else
    {
      color = matrix.Color(b, (int)(b * 0.55), 0);
    }

    matrix.setPixelColor(pixelXY(stars[i].x, stars[i].y), color);
  }

  matrix.show();
}

// FreeRTOS Starfield Task
void starfieldTask(void *pvParameters)
{
  while (true)
  {
    updateStars();
    drawStars();
    vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY));
  }
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  randomSeed(analogRead(1));

  for (int i = 0; i < MAX_STARS; i++)
  {
    stars[i].active = false;
  }

  for (int i = 0; i < 3; i++)
  {
    createStar(i);
  }

  xTaskCreate(
    starfieldTask,
    "StarTask",
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