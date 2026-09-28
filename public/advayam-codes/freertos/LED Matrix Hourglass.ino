/*
  5x5 Falling Sand Timer - FreeRTOS

  - Sand starts at the TOP row, CENTER pixel
  - Each grain falls downward
  - Grains pile up naturally
  - Matrix gradually fills with sand
  - All 25 pixels are filled in 60 seconds
  - Board flips reverse the direction of the sand
*/

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7

#define WIDTH 5
#define HEIGHT 5
#define NUM_LEDS 25

#define TOTAL_TIME 60000UL

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13
#define IMU_INT  38


// ==================================================
// FREERTOS
// ==================================================

SemaphoreHandle_t sandMutex;
SemaphoreHandle_t displayMutex;

TaskHandle_t imuTaskHandle;
TaskHandle_t sandTaskHandle;


// Flip notification.

volatile bool flipRequested = false;


// ==================================================
// HARDWARE
// ==================================================

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// ==================================================
// ORIENTATION STATE
// ==================================================

bool orientationFlipped = false;

int fillDirection = 1;
//  1 = filling
// -1 = draining


float refDown[3] = {
  0,
  0,
  0
};


bool flipCandidate = false;

unsigned long flipHoldStart = 0;

unsigned long lastFlipTime = 0;


const float FLIP_DOT_THRESHOLD =
  -0.5f;

const unsigned long FLIP_HOLD_MS =
  300;

const unsigned long FLIP_COOLDOWN_MS =
  800;


// ==================================================
// SAND STATE
// ==================================================

bool sand[HEIGHT][WIDTH];

int grainOrder[NUM_LEDS][2];

int grainCount = 0;


// ==================================================
// MATRIX COORDINATES
// ==================================================

int pixelXY(
  int x,
  int y
)
{
  if (x < 0 ||
      x >= WIDTH ||
      y < 0 ||
      y >= HEIGHT)
    return -1;


  if (y % 2 == 0)
    return y * WIDTH + x;


  return y * WIDTH +
         (WIDTH - 1 - x);
}


// ==================================================
// SET PIXEL
// ==================================================

void setPixelXY(
  int x,
  int y,
  uint32_t color
)
{
  if (x < 0 ||
      x >= WIDTH ||
      y < 0 ||
      y >= HEIGHT)
    return;


  // 90 degree clockwise display rotation.

  int hx =
    (HEIGHT - 1) - y;

  int hy =
    x;


  // Reverse physical coordinates when the board
  // has been flipped.

  if (orientationFlipped)
  {
    hx =
      (WIDTH - 1) - hx;

    hy =
      (HEIGHT - 1) - hy;
  }


  int index =
    pixelXY(
      hx,
      hy
    );


  if (index >= 0)
  {
    matrix.setPixelColor(
      index,
      color
    );
  }
}


// ==================================================
// DRAW SAND
// ==================================================

void drawSand()
{
  uint32_t sandColor =
    matrix.Color(
      255,
      120,
      0
    );


  matrix.clear();


  for (int y = 0;
       y < HEIGHT;
       y++)
  {
    for (int x = 0;
         x < WIDTH;
         x++)
    {
      if (sand[y][x])
      {
        setPixelXY(
          x,
          y,
          sandColor
        );
      }
    }
  }


  matrix.show();
}


// ==================================================
// SAFE DRAW
// ==================================================

void safeDrawSand()
{
  if (
    xSemaphoreTake(
      displayMutex,
      pdMS_TO_TICKS(100)
    ) == pdTRUE
  )
  {
    drawSand();

    xSemaphoreGive(
      displayMutex
    );
  }
}


// ==================================================
// READ GRAVITY
// ==================================================

void readGravity(
  float &x,
  float &y,
  float &z
)
{
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;


  imu.getEvent(
    &accel,
    &gyro,
    &temp
  );


  x =
    accel.acceleration.x;

  y =
    accel.acceleration.y;

  z =
    accel.acceleration.z;


  float mag =
    sqrt(
      x * x +
      y * y +
      z * z
    );


  if (mag > 0.0001f)
  {
    x /= mag;
    y /= mag;
    z /= mag;
  }
}


// ==================================================
// CHECK FLIP
// ==================================================

bool checkFlip()
{
  float gx;
  float gy;
  float gz;


  readGravity(
    gx,
    gy,
    gz
  );


  unsigned long now =
    millis();


  if (
    now - lastFlipTime <
    FLIP_COOLDOWN_MS
  )
  {
    return false;
  }


  float dot =
    gx * refDown[0] +
    gy * refDown[1] +
    gz * refDown[2];


  if (
    dot <
    FLIP_DOT_THRESHOLD
  )
  {
    if (!flipCandidate)
    {
      flipCandidate = true;

      flipHoldStart =
        now;
    }


    if (
      now - flipHoldStart >=
      FLIP_HOLD_MS
    )
    {
      orientationFlipped =
        !orientationFlipped;


      fillDirection =
        -fillDirection;


      // Current gravity becomes the
      // reference for the next flip.

      refDown[0] = gx;
      refDown[1] = gy;
      refDown[2] = gz;


      flipCandidate =
        false;


      lastFlipTime =
        now;


      return true;
    }
  }
  else
  {
    flipCandidate =
      false;
  }


  return false;
}


// ==================================================
// FIND STARTING GRAIN
// ==================================================

bool findStartingGrain(
  int &x,
  int &y
)
{
  x = 2;
  y = 0;


  if (!sand[y][x])
    return true;


  for (
    int offset = 1;
    offset <= 2;
    offset++
  )
  {
    int left =
      2 - offset;

    int right =
      2 + offset;


    if (
      left >= 0 &&
      !sand[0][left]
    )
    {
      x = left;

      return true;
    }


    if (
      right < WIDTH &&
      !sand[0][right]
    )
    {
      x = right;

      return true;
    }
  }


  return false;
}


// ==================================================
// ADD ONE GRAIN
// ==================================================

bool addGrain()
{
  int x;
  int y;


  if (
    !findStartingGrain(
      x,
      y
    )
  )
  {
    return false;
  }


  while (true)
  {
    // Check for a flip.

    if (flipRequested)
      return false;


    // Straight down.

    if (
      y + 1 < HEIGHT &&
      !sand[y + 1][x]
    )
    {
      y++;


      safeDrawSand();


      if (
        xSemaphoreTake(
          displayMutex,
          pdMS_TO_TICKS(100)
        ) == pdTRUE
      )
      {
        setPixelXY(
          x,
          y,
          matrix.Color(
            255,
            150,
            20
          )
        );

        matrix.show();

        xSemaphoreGive(
          displayMutex
        );
      }


      vTaskDelay(
        pdMS_TO_TICKS(80)
      );


      continue;
    }


    // Try diagonal movement.

    bool moved =
      false;


    if (
      random(2) == 0
    )
    {
      if (
        x > 0 &&
        y + 1 < HEIGHT &&
        !sand[y + 1][x - 1]
      )
      {
        x--;
        y++;

        moved = true;
      }
      else if (
        x < WIDTH - 1 &&
        y + 1 < HEIGHT &&
        !sand[y + 1][x + 1]
      )
      {
        x++;
        y++;

        moved = true;
      }
    }
    else
    {
      if (
        x < WIDTH - 1 &&
        y + 1 < HEIGHT &&
        !sand[y + 1][x + 1]
      )
      {
        x++;
        y++;

        moved = true;
      }
      else if (
        x > 0 &&
        y + 1 < HEIGHT &&
        !sand[y + 1][x - 1]
      )
      {
        x--;
        y++;

        moved = true;
      }
    }


    if (moved)
    {
      safeDrawSand();


      if (
        xSemaphoreTake(
          displayMutex,
          pdMS_TO_TICKS(100)
        ) == pdTRUE
      )
      {
        setPixelXY(
          x,
          y,
          matrix.Color(
            255,
            150,
            20
          )
        );

        matrix.show();

        xSemaphoreGive(
          displayMutex
        );
      }


      vTaskDelay(
        pdMS_TO_TICKS(80)
      );


      continue;
    }


    // Grain has settled.

    if (
      xSemaphoreTake(
        sandMutex,
        pdMS_TO_TICKS(100)
      ) == pdTRUE
    )
    {
      sand[y][x] =
        true;


      grainOrder[grainCount][0] =
        x;

      grainOrder[grainCount][1] =
        y;


      grainCount++;


      xSemaphoreGive(
        sandMutex
      );
    }


    safeDrawSand();


    return true;
  }
}


// ==================================================
// REMOVE ONE GRAIN
// ==================================================

void removeGrain()
{
  if (
    grainCount <= 0
  )
  {
    return;
  }


  int x;
  int y;


  if (
    xSemaphoreTake(
      sandMutex,
      pdMS_TO_TICKS(100)
    ) != pdTRUE
  )
  {
    return;
  }


  grainCount--;


  x =
    grainOrder[grainCount][0];

  y =
    grainOrder[grainCount][1];


  xSemaphoreGive(
    sandMutex
  );


  // Highlight the grain
  // before removing it.

  if (
    xSemaphoreTake(
      displayMutex,
      pdMS_TO_TICKS(100)
    ) == pdTRUE
  )
  {
    matrix.clear();


    drawSand();


    setPixelXY(
      x,
      y,
      matrix.Color(
        255,
        220,
        120
      )
    );


    matrix.show();


    xSemaphoreGive(
      displayMutex
    );
  }


  vTaskDelay(
    pdMS_TO_TICKS(80)
  );


  if (
    xSemaphoreTake(
      sandMutex,
      pdMS_TO_TICKS(100)
    ) == pdTRUE
  )
  {
    sand[y][x] =
      false;


    xSemaphoreGive(
      sandMutex
    );
  }


  safeDrawSand();
}


// ==================================================
// IMU TASK
// ==================================================

void imuTask(
  void *parameter
)
{
  for (;;)
  {
    if (
      checkFlip()
    )
    {
      flipRequested =
        true;


      Serial.println(
        "Flip detected"
      );
    }


    vTaskDelay(
      pdMS_TO_TICKS(20)
    );
  }
}


// ==================================================
// SAND TASK
// ==================================================

void sandTask(
  void *parameter
)
{
  unsigned long cycleStart =
    millis();


  unsigned long lastGrainTime =
    cycleStart;


  while (true)
  {
    // ------------------------------------------------
    // Handle flip.
    // ------------------------------------------------

    if (flipRequested)
    {
      flipRequested =
        false;


      Serial.print(
        "Direction: "
      );


      if (fillDirection == 1)
      {
        Serial.println(
          "FILLING"
        );
      }
      else
      {
        Serial.println(
          "DRAINING"
        );
      }
    }


    unsigned long now =
      millis();


    // ------------------------------------------------
    // Filling.
    // ------------------------------------------------

    if (
      fillDirection == 1 &&
      grainCount < NUM_LEDS
    )
    {
      unsigned long elapsed =
        now - cycleStart;


      unsigned long interval =
        TOTAL_TIME /
        NUM_LEDS;


      if (
        now - lastGrainTime >=
        interval
      )
      {
        addGrain();

        lastGrainTime =
          now;
      }
    }


    // ------------------------------------------------
    // Draining.
    // ------------------------------------------------

    else if (
      fillDirection == -1 &&
      grainCount > 0
    )
    {
      unsigned long interval =
        TOTAL_TIME /
        NUM_LEDS;


      if (
        now - lastGrainTime >=
        interval
      )
      {
        removeGrain();

        lastGrainTime =
          now;
      }
    }


    // ------------------------------------------------
    // Once completely full, wait for a flip.
    // ------------------------------------------------

    if (
      fillDirection == 1 &&
      grainCount >= NUM_LEDS
    )
    {
      // Stay full.
    }


    // ------------------------------------------------
    // Once completely empty, wait for a flip.
    // ------------------------------------------------

    if (
      fillDirection == -1 &&
      grainCount <= 0
    )
    {
      // Stay empty.
    }


    vTaskDelay(
      pdMS_TO_TICKS(10)
    );
  }
}


// ==================================================
// SETUP
// ==================================================

void setup()
{
  Serial.begin(
    115200
  );


  // ------------------------------------------------
  // Matrix
  // ------------------------------------------------

  matrix.begin();

  matrix.setBrightness(
    25
  );

  pinMode(
    16,
    OUTPUT
  );

  digitalWrite(
    16,
    LOW
  );


  matrix.clear();

  matrix.show();


  // ------------------------------------------------
  // Sand state
  // ------------------------------------------------

  memset(
    sand,
    false,
    sizeof(sand)
  );


  grainCount =
    0;


  randomSeed(
    analogRead(A0)
  );


  // ------------------------------------------------
  // SPI
  // ------------------------------------------------

  SPI.begin(
    IMU_SCK,
    IMU_MISO,
    IMU_MOSI,
    IMU_CS
  );


  // ------------------------------------------------
  // IMU
  // ------------------------------------------------

  if (
    !imu.begin_SPI(
      IMU_CS,
      &SPI
    )
  )
  {
    Serial.println(
      "IMU initialization failed!"
    );


    while (1)
    {
      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );
    }
  }


  imu.setAccelRange(
    LSM6DS_ACCEL_RANGE_4_G
  );


  imu.setAccelDataRate(
    LSM6DS_RATE_104_HZ
  );


  // ------------------------------------------------
  // Establish startup orientation.
  // ------------------------------------------------

  float sx = 0;
  float sy = 0;
  float sz = 0;


  for (
    int i = 0;
    i < 20;
    i++
  )
  {
    float gx;
    float gy;
    float gz;


    readGravity(
      gx,
      gy,
      gz
    );


    sx += gx;
    sy += gy;
    sz += gz;


    delay(10);
  }


  refDown[0] =
    sx / 20.0f;

  refDown[1] =
    sy / 20.0f;

  refDown[2] =
    sz / 20.0f;


  float mag =
    sqrt(
      refDown[0] *
      refDown[0] +

      refDown[1] *
      refDown[1] +

      refDown[2] *
      refDown[2]
    );


  if (mag > 0.0001f)
  {
    refDown[0] /=
      mag;

    refDown[1] /=
      mag;

    refDown[2] /=
      mag;
  }


  orientationFlipped =
    false;


  fillDirection =
    1;


  // ------------------------------------------------
  // FreeRTOS synchronization objects.
  // ------------------------------------------------

  sandMutex =
    xSemaphoreCreateMutex();


  displayMutex =
    xSemaphoreCreateMutex();


  if (
    sandMutex == NULL ||
    displayMutex == NULL
  )
  {
    Serial.println(
      "Mutex creation failed!"
    );


    while (1)
    {
      delay(1000);
    }
  }


  Serial.println(
    "Falling Sand Timer - FreeRTOS"
  );


  // ------------------------------------------------
  // Create tasks.
  // ------------------------------------------------

  xTaskCreate(
    imuTask,
    "IMU Task",
    4096,
    NULL,
    3,
    &imuTaskHandle
  );


  xTaskCreate(
    sandTask,
    "Sand Task",
    4096,
    NULL,
    2,
    &sandTaskHandle
  );
}


// ==================================================
// LOOP
// ==================================================

void loop()
{
  // Arduino loop is intentionally unused.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
