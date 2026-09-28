/*
 * ============================================================
 * 5x5 RGB Matrix Wi-Fi Analog Clock - UPSIDE DOWN
 * ============================================================
 *
 * Red pixel    = Second hand
 * Green pixels = Minute hand
 * Blue pixel   = Hour hand
 *
 * Clock gets the current time from an NTP server over Wi-Fi.
 *
 * The entire clock face is rotated 180 degrees.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>

//====================================================
// Hardware
//====================================================

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

//====================================================
// Wi-Fi
//====================================================

const char* WIFI_SSID =
  "therearefedsoutside";

const char* WIFI_PASSWORD =
  "niggersinparis999";

//====================================================
// India Standard Time
//====================================================

const long GMT_OFFSET_SEC = 19800;
const int DAYLIGHT_OFFSET_SEC = 0;

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
// Set Pixel
//====================================================

void setPixelXY(int x, int y, uint32_t color)
{
  if (x >= 0 && x < 5 &&
      y >= 0 && y < 5)
  {
    // Rotate the entire clock 180 degrees
    int flippedX = 4 - x;
    int flippedY = 4 - y;

    matrix.setPixelColor(
      pixelXY(flippedX, flippedY),
      color
    );
  }
}

//====================================================
// Clock perimeter
//====================================================

const int clockX[16] =
{
  2, 3, 4, 4,
  4, 4, 4, 3,
  2, 1, 0, 0,
  0, 0, 0, 1
};

const int clockY[16] =
{
  0, 0, 0, 1,
  2, 3, 4, 4,
  4, 4, 4, 3,
  2, 1, 0, 0
};

//====================================================
// Draw Hand
//====================================================

void drawHand(
  int perimeterIndex,
  int length,
  uint32_t color
)
{
  int x = clockX[perimeterIndex];
  int y = clockY[perimeterIndex];

  int dx = 0;
  int dy = 0;

  // Direction toward center
  if (x < 2)
    dx = 1;
  else if (x > 2)
    dx = -1;

  if (y < 2)
    dy = 1;
  else if (y > 2)
    dy = -1;

  for (int i = 0; i < length; i++)
  {
    int px = x + dx * i;
    int py = y + dy * i;

    setPixelXY(
      px,
      py,
      color
    );
  }
}

//====================================================
// Draw Clock
//====================================================

void drawClock(
  int hour,
  int minute,
  int second
)
{
  matrix.clear();

  uint32_t secondColor =
    matrix.Color(255, 0, 0);

  uint32_t minuteColor =
    matrix.Color(0, 255, 0);

  uint32_t hourColor =
    matrix.Color(0, 0, 255);

  uint32_t centerColor =
    matrix.Color(255, 255, 255);

  //==================================================
  // SECOND HAND
  //==================================================
  //
  // Reverse the perimeter index so that after the
  // 180-degree visual rotation, the second hand
  // still appears to move CLOCKWISE.
  //

  float secondPosition =
    second * 16.0 / 60.0;

  int secondIndex =
    (int)round(secondPosition) % 16;

  secondIndex =
    (16 - secondIndex) % 16;

  setPixelXY(
    clockX[secondIndex],
    clockY[secondIndex],
    secondColor
  );

  //==================================================
  // MINUTE HAND
  //==================================================

  float minutePosition =
    (minute + second / 60.0) * 16.0 / 60.0;

  int minuteIndex =
    (int)round(minutePosition) % 16;

  minuteIndex =
    (16 - minuteIndex) % 16;

  drawHand(
    minuteIndex,
    2,
    minuteColor
  );

  //==================================================
  // HOUR HAND
  //==================================================

  float hour12 =
    (hour % 12) +
    minute / 60.0;

  float hourPosition =
    hour12 * 16.0 / 12.0;

  int hourIndex =
    (int)round(hourPosition) % 16;

  hourIndex =
    (16 - hourIndex) % 16;

  drawHand(
    hourIndex,
    1,
    hourColor
  );

  //==================================================
  // CENTER
  //==================================================

  setPixelXY(
    2,
    2,
    centerColor
  );

  matrix.show();
}

//====================================================
// Connect Wi-Fi
//====================================================

void connectWiFi()
{
  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print("Connecting to Wi-Fi");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi connected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

//====================================================
// NTP Time Setup
//====================================================

void setupTime()
{
  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
  );

  Serial.println("Waiting for NTP time...");

  struct tm timeinfo;

  while (!getLocalTime(&timeinfo))
  {
    Serial.println("Waiting...");
    delay(500);
  }

  Serial.println("Time synchronized");
}

//====================================================
// Setup
//====================================================

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  connectWiFi();

  setupTime();
}

//====================================================
// Main Loop
//====================================================

void loop()
{
  struct tm timeinfo;

  if (!getLocalTime(&timeinfo))
  {
    Serial.println("Failed to get time");
    delay(1000);
    return;
  }

  int hour =
    timeinfo.tm_hour;

  int minute =
    timeinfo.tm_min;

  int second =
    timeinfo.tm_sec;

  Serial.printf(
    "%02d:%02d:%02d\n",
    hour,
    minute,
    second
  );

  drawClock(
    hour,
    minute,
    second
  );

  delay(1000);
}
