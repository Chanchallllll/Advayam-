/*
 * =============================================================================
 * CODE 3 - CONWAY'S GAME OF LIFE
 * =============================================================================
 *
 * Runs Conway's Game of Life on a 5x5 RGB NeoPixel matrix.
 *
 * Matrix:
 *   GPIO 7
 *
 * Living cells:
 *   Green
 *
 * The starting pattern is randomized each time the board starts.
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

//====================================================
// Matrix Configuration
//====================================================

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define MATRIX_WIDTH 5
#define MATRIX_HEIGHT 5

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//====================================================
// Game Grid
//====================================================

bool currentGrid[5][5];
bool nextGrid[5][5];

//====================================================
// Convert X/Y to LED Index
//====================================================
//
// The matrix is wired in a serpentine pattern.
//
// Even rows:
//   left -> right
//
// Odd rows:
//   right -> left
//====================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

//====================================================
// Draw Current Grid
//====================================================

void drawGrid()
{
    matrix.clear();

    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
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

//====================================================
// Count Living Neighbors
//====================================================

int countNeighbors(int x, int y)
{
    int count = 0;

    for (int dy = -1; dy <= 1; dy++)
    {
        for (int dx = -1; dx <= 1; dx++)
        {
            // Do not count the cell itself.
            if (dx == 0 && dy == 0)
            {
                continue;
            }

            int nx = x + dx;
            int ny = y + dy;

            // Ignore cells outside the 5x5 grid.
            if (nx >= 0 &&
                nx < MATRIX_WIDTH &&
                ny >= 0 &&
                ny < MATRIX_HEIGHT)
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

//====================================================
// Calculate Next Generation
//====================================================

void updateGrid()
{
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            int neighbors = countNeighbors(x, y);

            // Living cell:
            // survives with 2 or 3 neighbors.
            if (currentGrid[y][x])
            {
                nextGrid[y][x] =
                    (neighbors == 2 ||
                     neighbors == 3);
            }

            // Dead cell:
            // becomes alive with exactly 3 neighbors.
            else
            {
                nextGrid[y][x] =
                    (neighbors == 3);
            }
        }
    }

    // Copy next generation into current grid.
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            currentGrid[y][x] =
                nextGrid[y][x];
        }
    }
}

//====================================================
// Create Random Starting Pattern
//====================================================

void randomizeGrid()
{
    for (int y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (int x = 0; x < MATRIX_WIDTH; x++)
        {
            currentGrid[y][x] =
                random(100) < 45;
        }
    }
}

//====================================================
// Setup
//====================================================

void setup()
{
    Serial.begin(115200);

    matrix.begin();

    matrix.setBrightness(20);

    // Use the ESP32 hardware timer as a changing
    // source for the pseudo-random seed.
    randomSeed(
        micros()
    );

    randomizeGrid();

    Serial.println("Conway's Game of Life");
}

//====================================================
// Main Loop
//====================================================

void loop()
{
    // Display current generation.
    drawGrid();

    // Calculate next generation.
    updateGrid();

    // Wait before displaying it.
    delay(500);
}
