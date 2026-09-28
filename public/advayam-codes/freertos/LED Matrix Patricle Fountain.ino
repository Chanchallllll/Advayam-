/*
 * =============================================================================
 * PARTICLE FOUNTAIN - FreeRTOS VERSION
 * =============================================================================
 *
 * Creates a particle fountain from the bottom-centre of the 5x5 RGB matrix.
 *
 * Particles:
 *   - Start at the bottom centre.
 *   - Move upward.
 *   - Drift left or right.
 *   - Slow down due to gravity.
 *   - Gradually fade.
 *   - Restart when they leave the matrix.
 *
 * FreeRTOS:
 *   Particle task -> Updates particle physics and displays each frame.
 *
 * Matrix = GPIO 7
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define PARTICLES 8

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

SemaphoreHandle_t matrixMutex;


//=============================================================================
// Particle Structure
//=============================================================================

struct Particle
{
  float x;
  float y;

  float dx;
  float dy;

  uint8_t brightness;
};

Particle particles[PARTICLES];


//=============================================================================
// Matrix Mapping
//=============================================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


//=============================================================================
// Reset Particle
//=============================================================================

void resetParticle(int i)
{
  // Start at bottom centre.
  particles[i].x = 2;
  particles[i].y = 0;

  // Random horizontal drift.
  particles[i].dx =
    random(-10, 11) / 20.0;

  // Initial upward velocity.
  particles[i].dy =
    random(10, 20) / 10.0;

  particles[i].brightness =
    random(150, 256);
}


//=============================================================================
// Particle Animation Task
//=============================================================================

void particleTask(void *parameter)
{
  while (true)
  {
    if (xSemaphoreTake(
          matrixMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      matrix.clear();

      for (int i = 0;
           i < PARTICLES;
           i++)
      {
        //=================================================
        // Update position.
        //=================================================

        particles[i].x +=
          particles[i].dx;

        particles[i].y +=
          particles[i].dy;

        //=================================================
        // Gravity.
        //=================================================

        particles[i].dy -= 0.08;

        //=================================================
        // Fade particle.
        //=================================================

        if (particles[i].brightness > 10)
        {
          particles[i].brightness -= 8;
        }
        else
        {
          particles[i].brightness = 0;
        }

        //=================================================
        // Convert floating-point position to matrix cell.
        //=================================================

        int x =
          round(particles[i].x);

        int y =
          round(particles[i].y);

        //=================================================
        // Draw particle if it is inside the matrix.
        //=================================================

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

        //=================================================
        // Restart finished particles.
        //=================================================

        if (
          particles[i].y > 5 ||
          particles[i].x < -1 ||
          particles[i].x > 5 ||
          particles[i].brightness == 0
        )
        {
          resetParticle(i);
        }
      }

      matrix.show();

      xSemaphoreGive(matrixMutex);
    }

    // Update the fountain approximately every 80 ms.
    vTaskDelay(
      pdMS_TO_TICKS(80)
    );
  }
}


//=============================================================================
// Setup
//=============================================================================

void setup()
{
  matrix.begin();

  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  randomSeed(
    (uint32_t)esp_random()
  );

  for (int i = 0;
       i < PARTICLES;
       i++)
  {
    resetParticle(i);
  }

  matrixMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    particleTask,
    "ParticleFountain",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}


//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
