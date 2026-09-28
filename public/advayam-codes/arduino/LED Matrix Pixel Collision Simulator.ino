#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//====================================================
// Matrix Coordinate Mapping
//====================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

//====================================================
// Set Pixel Using X/Y Coordinates
//====================================================

void setPixelXY(int x, int y, uint32_t color)
{
  if (x >= 0 && x < 5 && y >= 0 && y < 5)
  {
    matrix.setPixelColor(pixelXY(x, y), color);
  }
}

//====================================================
// Collision Animation
//====================================================

void collision()
{
  //==================================================
  // Two white pixels moving toward the center
  //==================================================

  for (int step = 0; step < 3; step++)
  {
    matrix.clear();

    // Left pixel
    int leftX = step;

    // Right pixel
    int rightX = 4 - step;

    setPixelXY(leftX, 2, matrix.Color(255, 255, 255));
    setPixelXY(rightX, 2, matrix.Color(255, 255, 255));

    matrix.show();

    delay(120);
  }

  //==================================================
  // COLLISION
  //==================================================

  matrix.clear();

  // Bright white flash at the instant of collision
  setPixelXY(2, 2, matrix.Color(255, 255, 255));

  matrix.show();

  // Very short delay so the collision is visible
  delay(30);

  //==================================================
  // INSTANT EXPLOSION
  //==================================================

  explosion();
}

//====================================================
// Colourful Explosion
//====================================================

void explosion()
{
  // Explosion colour palette
  uint32_t colors[] =
  {
    matrix.Color(255, 0, 0),       // Red
    matrix.Color(255, 80, 0),      // Orange
    matrix.Color(255, 255, 0),     // Yellow
    matrix.Color(0, 255, 0),       // Green
    matrix.Color(0, 150, 255),     // Blue
    matrix.Color(80, 0, 255),      // Purple
    matrix.Color(255, 0, 150)      // Pink
  };

  //==================================================
  // EXPLOSION FRAME 1 - Center
  //==================================================

  matrix.clear();

  setPixelXY(2, 2, matrix.Color(255, 255, 255));

  matrix.show();

  delay(30);

  //==================================================
  // EXPLOSION FRAME 2
  //==================================================

  matrix.clear();

  // Center
  setPixelXY(2, 2, colors[0]);

  // Direct neighbours
  setPixelXY(1, 2, colors[1]);
  setPixelXY(3, 2, colors[2]);
  setPixelXY(2, 1, colors[3]);
  setPixelXY(2, 3, colors[4]);

  matrix.show();

  delay(50);

  //==================================================
  // EXPLOSION FRAME 3
  //==================================================

  matrix.clear();

  // Center
  setPixelXY(2, 2, colors[0]);

  // Inner ring
  setPixelXY(1, 2, colors[1]);
  setPixelXY(3, 2, colors[2]);
  setPixelXY(2, 1, colors[3]);
  setPixelXY(2, 3, colors[4]);

  // Diagonal particles
  setPixelXY(1, 1, colors[5]);
  setPixelXY(3, 1, colors[6]);
  setPixelXY(1, 3, colors[2]);
  setPixelXY(3, 3, colors[3]);

  matrix.show();

  delay(60);

  //==================================================
  // EXPLOSION FRAME 4 - Maximum expansion
  //==================================================

  matrix.clear();

  // Center
  setPixelXY(2, 2, colors[0]);

  // Middle pixels
  setPixelXY(1, 2, colors[1]);
  setPixelXY(3, 2, colors[2]);
  setPixelXY(2, 1, colors[3]);
  setPixelXY(2, 3, colors[4]);

  // Diagonals
  setPixelXY(1, 1, colors[5]);
  setPixelXY(3, 1, colors[6]);
  setPixelXY(1, 3, colors[0]);
  setPixelXY(3, 3, colors[1]);

  // Outer explosion
  setPixelXY(0, 2, colors[2]);
  setPixelXY(4, 2, colors[3]);
  setPixelXY(2, 0, colors[4]);
  setPixelXY(2, 4, colors[5]);

  // Corners
  setPixelXY(0, 0, colors[6]);
  setPixelXY(4, 0, colors[0]);
  setPixelXY(0, 4, colors[1]);
  setPixelXY(4, 4, colors[2]);

  matrix.show();

  delay(100);

  //==================================================
  // Explosion fades
  //==================================================

  for (int brightness = 100; brightness > 0; brightness -= 20)
  {
    matrix.setBrightness(brightness);
    delay(40);
  }

  matrix.clear();

  // Restore normal brightness
  matrix.setBrightness(25);

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
}

//====================================================
// Main Loop
//====================================================

void loop()
{
  collision();

  // Pause before repeating
  delay(500);
}
