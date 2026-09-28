/*
 * This program creates a simple tilt-controlled maze game.
 *
 * Tilt the board to move the green player through the maze.
 * Blue pixels are walls and red is the goal.
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

Adafruit_LSM6DSOX imu;
Adafruit_NeoPixel matrix(NUM_LEDS, MATRIX_PIN, NEO_GRB + NEO_KHZ800);

int playerX = 0;
int playerY = 0;

const bool walls[5][5] = {
  {0, 1, 0, 0, 0},
  {0, 1, 0, 1, 0},
  {0, 0, 0, 1, 0},
  {1, 1, 0, 0, 0},
  {0, 0, 0, 1, 0}
};

int pixelXY(int x, int y) {
  if (y % 2 == 0) return y * 5 + x;
  return y * 5 + (4 - x);
}

void drawMaze() {
  matrix.clear();

  for (int y = 0; y < 5; y++) {
    for (int x = 0; x < 5; x++) {
      if (walls[y][x]) {
        matrix.setPixelColor(
          pixelXY(x, y),
          matrix.Color(0, 0, 100)
        );
      }
    }
  }

  matrix.setPixelColor(
    pixelXY(4, 4),
    matrix.Color(255, 0, 0)
  );

  matrix.setPixelColor(
    pixelXY(playerX, playerY),
    matrix.Color(0, 255, 0)
  );

  matrix.show();
}

void setup() {
  matrix.begin();
  matrix.setBrightness(25);
  pinMode(16,OUTPUT);
  digitalWrite(16,LOW);
  SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

  if (!imu.begin_SPI(IMU_CS, &SPI)) {
    while (1);
  }

  imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
  imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

  drawMaze();
}

void loop() {
  sensors_event_t accel, gyro, temp;
  imu.getEvent(&accel, &gyro, &temp);

  int newX = playerX;
  int newY = playerY;

  // Accelerometer Y controls display X
  // Direction is inverted
  if (accel.acceleration.y > 4) newX--;
  if (accel.acceleration.y < -4) newX++;

  // Accelerometer X controls display Y
  // Direction is inverted
  if (accel.acceleration.x > 4) newY++;
  if (accel.acceleration.x < -4) newY--;

  newX = constrain(newX, 0, 4);
  newY = constrain(newY, 0, 4);

  if (!walls[newY][newX]) {
    playerX = newX;
    playerY = newY;
  }

  if (playerX == 4 && playerY == 4) {
    matrix.fill(matrix.Color(0, 255, 0));
    matrix.show();
    delay(500);
    playerX = 0;
    playerY = 0;
  }

  drawMaze();

  delay(180);
}

