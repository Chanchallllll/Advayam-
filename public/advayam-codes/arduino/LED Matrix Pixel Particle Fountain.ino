/*
 * This program creates a particle fountain from the bottom-center
 * of the 5x5 RGB matrix. Particles rise upward while drifting
 * left or right and gradually fade.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define PARTICLES  8

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

struct Particle
{
  float x;
  float y;
  float dx;
  float dy;
  uint8_t brightness;
};

Particle particles[PARTICLES];

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void resetParticle(int i)
{
  // Start at bottom center
  particles[i].x = 2;
  particles[i].y = 0;

  // Random horizontal drift
  particles[i].dx =
    random(-10, 11) / 20.0;

  // Shoot upward
  particles[i].dy =
    random(10, 20) / 10.0;

  particles[i].brightness =
    random(150, 256);
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);

  randomSeed(analogRead(1));

  for (int i = 0; i < PARTICLES; i++)
    resetParticle(i);
}

void loop()
{
  matrix.clear();

  for (int i = 0; i < PARTICLES; i++)
  {
    particles[i].x += particles[i].dx;
    particles[i].y += particles[i].dy;

    // Gravity
    particles[i].dy -= 0.08;

    if (particles[i].brightness > 10)
      particles[i].brightness -= 8;
    else
      particles[i].brightness = 0;

    int x =
      round(particles[i].x);

    int y =
      round(particles[i].y);

    if (x >= 0 && x < 5 &&
        y >= 0 && y < 5)
    {
      uint8_t b =
        particles[i].brightness;

      matrix.setPixelColor(
        pixelXY(x, y),
        matrix.Color(
          b,
          b / 3,
          0
        )
      );
    }

    // Restart finished particles
    if (particles[i].y > 5 ||
        particles[i].x < -1 ||
        particles[i].x > 5 ||
        particles[i].brightness == 0)
    {
      resetParticle(i);
    }
  }

  matrix.show();

  delay(80);
}
