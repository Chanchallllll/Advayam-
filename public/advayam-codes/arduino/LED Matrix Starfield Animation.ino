/*
 * ============================================================
 * 5x5 RGB Matrix Starfield
 * ============================================================
 *
 * White pixels = normal stars
 * Warm yellow  = rare sunset stars
 *
 * Stars fade smoothly in and out.
 * Stars are spaced apart to avoid clustering.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <math.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define MAX_STARS 6

// Faster update interval
#define FRAME_DELAY 20

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//====================================================
// Star Structure
//====================================================

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

//====================================================
// Pixel Mapping
//====================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//====================================================
// Check Position
//====================================================

bool positionAvailable(int x, int y)
{
  for (int i = 0; i < MAX_STARS; i++)
  {
    if (!stars[i].active)
      continue;

    int dx = abs(x - stars[i].x);
    int dy = abs(y - stars[i].y);

    // Keep stars separated
    if (dx <= 1 && dy <= 1)
      return false;
  }

  return true;
}

//====================================================
// Find Random Position
//====================================================

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

//====================================================
// Create Star
//====================================================

void createStar(int index)
{
  int x;
  int y;

  if (!findPosition(x, y))
    return;

  stars[index].x = x;
  stars[index].y = y;

  // Start completely dark
  stars[index].brightness = 0;

  // Very small brightness increments
  //
  // This produces many more visible fade levels.
  //
  stars[index].fadeAmount =
    random(8, 16) / 10.0;

  stars[index].fadingIn = true;

  stars[index].active = true;

  // Approximately 10% sunset stars
  stars[index].sunset =
    (random(100) < 10);
}

//====================================================
// Remove Star
//====================================================

void removeStar(int index)
{
  stars[index].active = false;
}

//====================================================
// Smooth Fade Function
//====================================================
//
// Smoothstep easing:
//
// Starts slowly
// Speeds up in the middle
// Slows down at the end
//
// This makes the fade appear smoother to the eye.
//

float smoothFade(float x)
{
  return x * x * (3.0 - 2.0 * x);
}

//====================================================
// Update Stars
//====================================================

void updateStars()
{
  for (int i = 0; i < MAX_STARS; i++)
  {
    //================================================
    // Create new star
    //================================================

    if (!stars[i].active)
    {
      if (random(100) < 9)
      {
        createStar(i);
      }

      continue;
    }

    //================================================
    // Fade In
    //================================================

    if (stars[i].fadingIn)
    {
      stars[i].brightness +=
        stars[i].fadeAmount;

      if (stars[i].brightness >= 255.0)
      {
        stars[i].brightness = 255.0;

        stars[i].fadingIn = false;
      }
    }

    //================================================
    // Fade Out
    //================================================

    else
    {
      stars[i].brightness -=
        stars[i].fadeAmount;

      if (stars[i].brightness <= 0.0)
      {
        stars[i].brightness = 0.0;

        removeStar(i);
      }
    }
  }
}

//====================================================
// Draw Stars
//====================================================

void drawStars()
{
  matrix.clear();

  for (int i = 0; i < MAX_STARS; i++)
  {
    if (!stars[i].active)
      continue;

    float normalized =
      stars[i].brightness / 255.0;

    // Apply smooth easing
    float smooth =
      smoothFade(normalized);

    int b =
      (int)(smooth * 255.0);

    uint32_t color;

    //================================================
    // White Star
    //================================================

    if (!stars[i].sunset)
    {
      color = matrix.Color(
        b,
        b,
        b
      );
    }

    //================================================
    // Sunset Yellow
    //================================================

    else
    {
      color = matrix.Color(
        b,
        (int)(b * 0.55),
        0
      );
    }

    matrix.setPixelColor(
      pixelXY(
        stars[i].x,
        stars[i].y
      ),
      color
    );
  }

  matrix.show();
}

//====================================================
// Setup
//====================================================

void setup()
{
  matrix.begin();

  matrix.setBrightness(25);

  matrix.clear();

  matrix.show();

  randomSeed(
    analogRead(1)
  );

  // Initialize all stars
  for (int i = 0; i < MAX_STARS; i++)
  {
    stars[i].active = false;
  }

  // Initial scattered stars
  for (int i = 0; i < 3; i++)
  {
    createStar(i);
  }
}

//====================================================
// Main Loop
//====================================================

void loop()
{
  updateStars();

  drawStars();

  delay(FRAME_DELAY);
}
