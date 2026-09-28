/*
 * This program runs Conway's Game of Life on the 5x5 RGB
 * LED matrix connected to GPIO 7.
 *
 * The initial cells are randomized and the simulation
 * continuously evolves on the matrix.
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

bool currentGrid[5][5];
bool nextGrid[5][5];

// Convert x/y coordinates to serpentine LED index
int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

// Draw the current Game of Life state
void drawGrid()
{
    matrix.clear();

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (currentGrid[y][x])
            {
                matrix.setPixelColor(
                    pixelXY(x, y),
                    matrix.Color(0, 255, 0)
                );
            }
        }
    }

    matrix.show();
}

// Count living neighbors
int countNeighbors(int x, int y)
{
    int count = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            if (dx == 0 && dy == 0)
            {
                continue;
            }

            int nx = x + dx;
            int ny = y + dy;

            if (nx >= 0 && nx < 5 &&
                ny >= 0 && ny < 5)
            {
                if (currentGrid[ny][nx])
                {
                    count++;
                }
            }
        }
    }

    return count;
}

// Calculate the next generation
void updateGrid()
{
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            int neighbors =
                countNeighbors(x, y);

            if (currentGrid[y][x])
            {
                nextGrid[y][x] =
                    (neighbors == 2 ||
                     neighbors == 3);
            }
            else
            {
                nextGrid[y][x] =
                    (neighbors == 3);
            }
        }
    }

    // Copy next generation
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            currentGrid[y][x] =
                nextGrid[y][x];
        }
    }
}

// Create a random starting pattern
void randomizeGrid()
{
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            currentGrid[y][x] =
                random(100) < 45;
        }
    }
}

void setup()
{
    matrix.begin();
    matrix.setBrightness(20);

    randomSeed(
        analogRead(1)
    );

    randomizeGrid();
}

void loop()
{
    drawGrid();

    updateGrid();

    delay(500);
}
