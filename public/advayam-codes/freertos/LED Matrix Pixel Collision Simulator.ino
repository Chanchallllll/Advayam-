/*
 * =============================================================================
 * COLLISION + EXPLOSION ANIMATION - FreeRTOS VERSION
 * =============================================================================
 *
 * Two white pixels move toward the centre of the matrix.
 * They collide and produce a colourful expanding explosion.
 *
 * FreeRTOS:
 *   Animation task -> Controls the complete collision sequence.
 *
 * vTaskDelay() is used instead of delay(), allowing FreeRTOS to schedule
 * other tasks while the animation is waiting between frames.
 *
 * Matrix = GPIO 7
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

SemaphoreHandle_t matrixMutex;


//=============================================================================
// Matrix Coordinate Mapping
//=============================================================================

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}


//=============================================================================
// Set Pixel
//=============================================================================

void setPixelXY(
  int x,
  int y,
  uint32_t color
)
{
  if (x >= 0 && x < 5 &&
      y >= 0 && y < 5)
  {
    matrix.setPixelColor(
      pixelXY(x, y),
      color
    );
  }
}


//=============================================================================
// Collision Animation
//=============================================================================

void collision()
{
  //==================================================
  // Two white pixels moving toward the centre.
  //==================================================

  for (int step = 0;
       step < 3;
       step++)
  {
    if (xSemaphoreTake(
          matrixMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      matrix.clear();

      int leftX = step;
      int rightX = 4 - step;

      setPixelXY(
        leftX,
        2,
        matrix.Color(
          255,
          255,
          255
        )
      );

      setPixelXY(
        rightX,
        2,
        matrix.Color(
          255,
          255,
          255
        )
      );

      matrix.show();

      xSemaphoreGive(matrixMutex);
    }

    vTaskDelay(
      pdMS_TO_TICKS(120)
    );
  }

  //==================================================
  // Collision flash.
  //==================================================

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    setPixelXY(
      2,
      2,
      matrix.Color(
        255,
        255,
        255
      )
    );

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }

  vTaskDelay(
    pdMS_TO_TICKS(30)
  );

  explosion();
}


//=============================================================================
// Colourful Explosion
//=============================================================================

void explosion()
{
  uint32_t colors[] =
  {
    matrix.Color(255, 0, 0),
    matrix.Color(255, 80, 0),
    matrix.Color(255, 255, 0),
    matrix.Color(0, 255, 0),
    matrix.Color(0, 150, 255),
    matrix.Color(80, 0, 255),
    matrix.Color(255, 0, 150)
  };

  //==================================================
  // Explosion frame 1 - centre.
  //==================================================

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    setPixelXY(
      2,
      2,
      matrix.Color(
        255,
        255,
        255
      )
    );

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }

  vTaskDelay(
    pdMS_TO_TICKS(30)
  );


  //==================================================
  // Explosion frame 2.
  //==================================================

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    setPixelXY(2, 2, colors[0]);

    setPixelXY(1, 2, colors[1]);
    setPixelXY(3, 2, colors[2]);
    setPixelXY(2, 1, colors[3]);
    setPixelXY(2, 3, colors[4]);

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }

  vTaskDelay(
    pdMS_TO_TICKS(50)
  );


  //==================================================
  // Explosion frame 3.
  //==================================================

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    setPixelXY(2, 2, colors[0]);

    setPixelXY(1, 2, colors[1]);
    setPixelXY(3, 2, colors[2]);
    setPixelXY(2, 1, colors[3]);
    setPixelXY(2, 3, colors[4]);

    setPixelXY(1, 1, colors[5]);
    setPixelXY(3, 1, colors[6]);
    setPixelXY(1, 3, colors[2]);
    setPixelXY(3, 3, colors[3]);

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }

  vTaskDelay(
    pdMS_TO_TICKS(60)
  );


  //==================================================
  // Explosion frame 4 - maximum expansion.
  //==================================================

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    setPixelXY(2, 2, colors[0]);

    setPixelXY(1, 2, colors[1]);
    setPixelXY(3, 2, colors[2]);
    setPixelXY(2, 1, colors[3]);
    setPixelXY(2, 3, colors[4]);

    setPixelXY(1, 1, colors[5]);
    setPixelXY(3, 1, colors[6]);
    setPixelXY(1, 3, colors[0]);
    setPixelXY(3, 3, colors[1]);

    setPixelXY(0, 2, colors[2]);
    setPixelXY(4, 2, colors[3]);
    setPixelXY(2, 0, colors[4]);
    setPixelXY(2, 4, colors[5]);

    setPixelXY(0, 0, colors[6]);
    setPixelXY(4, 0, colors[0]);
    setPixelXY(0, 4, colors[1]);
    setPixelXY(4, 4, colors[2]);

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }

  vTaskDelay(
    pdMS_TO_TICKS(100)
  );


  //==================================================
  // Explosion fades.
  //==================================================

  for (
    int brightness = 100;
    brightness > 0;
    brightness -= 20
  )
  {
    if (xSemaphoreTake(
          matrixMutex,
          portMAX_DELAY
        ) == pdTRUE)
    {
      matrix.setBrightness(
        brightness
      );

      matrix.show();

      xSemaphoreGive(matrixMutex);
    }

    vTaskDelay(
      pdMS_TO_TICKS(40)
    );
  }

  if (xSemaphoreTake(
        matrixMutex,
        portMAX_DELAY
      ) == pdTRUE)
  {
    matrix.clear();

    // Restore normal brightness.
    matrix.setBrightness(25);

    matrix.show();

    xSemaphoreGive(matrixMutex);
  }
}


//=============================================================================
// Animation Task
//=============================================================================

void animationTask(void *parameter)
{
  while (true)
  {
    collision();

    // Pause before repeating.
    vTaskDelay(
      pdMS_TO_TICKS(500)
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

  matrixMutex =
    xSemaphoreCreateMutex();

  xTaskCreatePinnedToCore(
    animationTask,
    "CollisionAnimation",
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
