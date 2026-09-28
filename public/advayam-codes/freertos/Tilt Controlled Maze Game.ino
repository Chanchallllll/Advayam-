/*
 * =============================================================================
 * TILT-CONTROLLED MAZE - FREERTOS VERSION
 * =============================================================================
 *
 * Tilt the board to move the green player through the maze.
 *
 * LED colors:
 *   Blue  -> wall
 *   Red   -> goal
 *   Green -> player
 *
 * FreeRTOS structure:
 *
 *   MazeTask
 *       - Reads the accelerometer
 *       - Converts tilt into movement
 *       - Checks maze walls
 *       - Updates the NeoPixel matrix
 *
 * The movement is evaluated approximately every 180 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

//====================================================
// Matrix Configuration
//====================================================
#define MATRIX_PIN 7
#define NUM_LEDS   25

//====================================================
// IMU Configuration
//====================================================
#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//====================================================
// Player Position
//====================================================
int playerX = 0;
int playerY = 0;

//====================================================
// Maze Layout
//====================================================
const bool walls[5][5] =
{
    {0, 1, 0, 0, 0},
    {0, 1, 0, 1, 0},
    {0, 0, 0, 1, 0},
    {1, 1, 0, 0, 0},
    {0, 0, 0, 1, 0}
};

//====================================================
// Convert X/Y Coordinates to NeoPixel Index
//====================================================
int pixelXY(int x, int y)
{
    if (y % 2 == 0)
        return y * 5 + x;

    return y * 5 + (4 - x);
}

//====================================================
// Draw Maze
//====================================================
void drawMaze()
{
    matrix.clear();

    // Draw maze walls.
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            if (walls[y][x])
            {
                matrix.setPixelColor(
                    pixelXY(x, y),
                    matrix.Color(0, 0, 100)
                );
            }
        }
    }

    // Draw the goal.
    matrix.setPixelColor(
        pixelXY(4, 4),
        matrix.Color(255, 0, 0)
    );

    // Draw the player.
    matrix.setPixelColor(
        pixelXY(playerX, playerY),
        matrix.Color(0, 255, 0)
    );

    matrix.show();
}

//====================================================
// Maze Task
//====================================================
void MazeTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        imu.getEvent(&accel, &gyro, &temp);

        int newX = playerX;
        int newY = playerY;

        //================================================
        // Horizontal Movement
        //================================================
        // Accelerometer Y controls display X.
        // Direction is intentionally inverted.
        if (accel.acceleration.y > 4)
            newX--;

        if (accel.acceleration.y < -4)
            newX++;

        //================================================
        // Vertical Movement
        //================================================
        // Accelerometer X controls display Y.
        // Direction is intentionally inverted.
        if (accel.acceleration.x > 4)
            newY++;

        if (accel.acceleration.x < -4)
            newY--;

        // Keep the player inside the 5x5 maze.
        newX = constrain(newX, 0, 4);
        newY = constrain(newY, 0, 4);

        //================================================
        // Wall Collision
        //================================================
        if (!walls[newY][newX])
        {
            playerX = newX;
            playerY = newY;
        }

        //================================================
        // Goal Detection
        //================================================
        if (playerX == 4 && playerY == 4)
        {
            matrix.fill(
                matrix.Color(0, 255, 0)
            );

            matrix.show();

            Serial.println("MAZE COMPLETE!");

            // Display success for 500 ms.
            vTaskDelay(pdMS_TO_TICKS(500));

            // Reset player to the start.
            playerX = 0;
            playerY = 0;
        }

        drawMaze();

        // Limit movement speed.
        vTaskDelay(pdMS_TO_TICKS(180));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    matrix.begin();
    matrix.setBrightness(25);

    pinMode(16, OUTPUT);
    digitalWrite(16, LOW);

    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println("IMU initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

    drawMaze();

    xTaskCreate(
        MazeTask,
        "Maze Task",
        4096,
        NULL,
        2,
        NULL
    );
}

//====================================================
// Loop
//====================================================
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
