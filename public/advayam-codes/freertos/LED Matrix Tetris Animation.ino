/*
 * This program animates simple falling tetromino blocks on the
 * 5x5 RGB matrix using a dedicated FreeRTOS task.
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

bool board[5][5];

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void drawBoard(
  int blockX,
  int blockY,
  const int shape[][2],
  int size,
  uint32_t color
)
{
  matrix.clear();

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      if (board[y][x])
      {
        matrix.setPixelColor(
          pixelXY(x, y),
          matrix.Color(80, 80, 80)
        );
      }
    }
  }

  for (int i = 0; i < size; i++)
  {
    int x = blockX + shape[i][0];
    int y = blockY + shape[i][1];

    if (x >= 0 && x < 5 && y >= 0 && y < 5)
    {
      matrix.setPixelColor(pixelXY(x, y), color);
    }
  }

  matrix.show();
}

bool canMoveRight(
  int blockX,
  int blockY,
  const int shape[][2],
  int size
)
{
  for (int i = 0; i < size; i++)
  {
    int x = blockX + shape[i][0];
    int y = blockY + shape[i][1];

    if (x >= 5)
      return false;

    if (x >= 0 && y >= 0 && y < 5 && board[y][x])
    {
      return false;
    }
  }

  return true;
}

void placeBlock(
  int blockX,
  int blockY,
  const int shape[][2],
  int size
)
{
  for (int i = 0; i < size; i++)
  {
    int x = blockX + shape[i][0];
    int y = blockY + shape[i][1];

    if (x >= 0 && x < 5 && y >= 0 && y < 5)
    {
      board[y][x] = true;
    }
  }
}

// FreeRTOS Task for Tetris animations
void tetrisTask(void *pvParameters)
{
  while (true)
  {
    const int I[4][2] = {{0,0}, {1,0}, {2,0}, {3,0}};
    const int T[4][2] = {{1,0}, {0,1}, {1,1}, {2,1}};
    const int L[4][2] = {{0,0}, {0,1}, {0,2}, {1,2}};
    const int O[4][2] = {{0,0}, {1,0}, {0,1}, {1,1}};

    int choice = random(0, 4);
    const int (*shape)[2];

    if (choice == 0) shape = I;
    else if (choice == 1) shape = T;
    else if (choice == 2) shape = L;
    else shape = O;

    uint32_t colors[] =
    {
      matrix.Color(0, 200, 255),
      matrix.Color(180, 0, 255),
      matrix.Color(255, 100, 0),
      matrix.Color(255, 255, 0)
    };

    int blockX = -3;
    int blockY = random(0, 4);

    while (canMoveRight(blockX + 1, blockY, shape, 4))
    {
      blockX++;
      drawBoard(blockX, blockY, shape, 4, colors[choice]);
      vTaskDelay(pdMS_TO_TICKS(250));
    }

    placeBlock(blockX, blockY, shape, 4);
    drawBoard(blockX, blockY, shape, 4, colors[choice]);
    vTaskDelay(pdMS_TO_TICKS(300));

    bool crowded = true;
    for (int x = 0; x < 5; x++)
    {
      if (!board[0][x])
        crowded = false;
    }

    if (crowded)
    {
      for (int y = 0; y < 5; y++)
      {
        for (int x = 0; x < 5; x++)
        {
          board[y][x] = false;
        }
      }

      matrix.clear();
      matrix.show();
      vTaskDelay(pdMS_TO_TICKS(500));
    }
  }
}

void setup()
{
  matrix.begin();
  matrix.setBrightness(25);
  randomSeed(analogRead(1));

  for (int y = 0; y < 5; y++)
  {
    for (int x = 0; x < 5; x++)
    {
      board[y][x] = false;
    }
  }

  xTaskCreate(
    tetrisTask,
    "TetrisTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  vTaskDelete(NULL);
}