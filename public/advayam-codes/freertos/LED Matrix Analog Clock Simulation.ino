/*
 * =============================================================================
 * 5x5 RGB MATRIX WI-FI ANALOG CLOCK
 * =============================================================================
 *
 * Red pixel    = Second hand
 * Green pixels = Minute hand
 * Blue pixel   = Hour hand
 *
 * The clock gets the current time from an NTP server over Wi-Fi.
 *
 * The clock display runs inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *   NeoPixel matrix data -> GPIO 7
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>
#include <time.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// Hardware
// =============================================================================

#define MATRIX_PIN 7
#define NUM_LEDS 25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// =============================================================================
// Wi-Fi
// =============================================================================
//
// Replace these with your actual Wi-Fi credentials.
//
// Do not commit real credentials to a public repository.
//

const char* WIFI_SSID =
  "YOUR_WIFI_SSID";

const char* WIFI_PASSWORD =
  "YOUR_WIFI_PASSWORD";

// =============================================================================
// India Standard Time
// =============================================================================

const long GMT_OFFSET_SEC = 19800;
const int DAYLIGHT_OFFSET_SEC = 0;

// =============================================================================
// Matrix coordinate mapping
// =============================================================================

int pixelXY(
  int x,
  int y
)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

// =============================================================================
// Set a pixel using X/Y coordinates
// =============================================================================

void setPixelXY(
  int x,
  int y,
  uint32_t colour
)
{
  if (
    x >= 0 &&
    x < 5 &&
    y >= 0 &&
    y < 5
  )
  {
    // Rotate the entire clock 180 degrees.
    int flippedX = 4 - x;
    int flippedY = 4 - y;

    matrix.setPixelColor(
      pixelXY(flippedX, flippedY),
      colour
    );
  }
}

// =============================================================================
// Clock perimeter
// =============================================================================

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

// =============================================================================
// Draw a clock hand
// =============================================================================

void drawHand(
  int perimeterIndex,
  int length,
  uint32_t colour
)
{
  int x = clockX[perimeterIndex];
  int y = clockY[perimeterIndex];

  int dx = 0;
  int dy = 0;

  // Direction toward the centre.
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
      colour
    );
  }
}

// =============================================================================
// Draw clock
// =============================================================================

void drawClock(
  int hour,
  int minute,
  int second
)
{
  matrix.clear();

  uint32_t secondColour =
    matrix.Color(255, 0, 0);

  uint32_t minuteColour =
    matrix.Color(0, 255, 0);

  uint32_t hourColour =
    matrix.Color(0, 0, 255);

  uint32_t centreColour =
    matrix.Color(255, 255, 255);

  // ===========================================================================
  // SECOND HAND
  // ===========================================================================
  //
  // Reverse the perimeter index so that after the
  // 180-degree visual rotation, the second hand
  // still appears to move clockwise.
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
    secondColour
  );

  // ===========================================================================
  // MINUTE HAND
  // ===========================================================================

  float minutePosition =
    (minute + second / 60.0) *
    16.0 / 60.0;

  int minuteIndex =
    (int)round(minutePosition) % 16;

  minuteIndex =
    (16 - minuteIndex) % 16;

  drawHand(
    minuteIndex,
    2,
    minuteColour
  );

  // ===========================================================================
  // HOUR HAND
  // ===========================================================================

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
    hourColour
  );

  // ===========================================================================
  // CENTRE
  // ===========================================================================

  setPixelXY(
    2,
    2,
    centreColour
  );

  matrix.show();
}

// =============================================================================
// Connect Wi-Fi
// =============================================================================

void connectWiFi()
{
  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print(
    "Connecting to Wi-Fi"
  );

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(
      pdMS_TO_TICKS(500)
    );

    Serial.print(".");
  }

  Serial.println();
  Serial.println(
    "Wi-Fi connected"
  );

  Serial.print(
    "IP address: "
  );

  Serial.println(
    WiFi.localIP()
  );
}

// =============================================================================
// NTP time setup
// =============================================================================

void setupTime()
{
  configTime(
    GMT_OFFSET_SEC,
    DAYLIGHT_OFFSET_SEC,
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
  );

  Serial.println(
    "Waiting for NTP time..."
  );

  struct tm timeinfo;

  while (!getLocalTime(&timeinfo))
  {
    Serial.println(
      "Waiting..."
    );

    vTaskDelay(
      pdMS_TO_TICKS(500)
    );
  }

  Serial.println(
    "Time synchronized"
  );
}

// =============================================================================
// Clock FreeRTOS task
// =============================================================================

void clockTask(void *parameter)
{
  while (true)
  {
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo))
    {
      Serial.println(
        "Failed to get time"
      );

      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );

      continue;
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

    // Update the display once per second.
    vTaskDelay(
      pdMS_TO_TICKS(1000)
    );
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);

  matrix.clear();
  matrix.show();

  // Connect to Wi-Fi before starting the clock task.
  connectWiFi();

  // Synchronise the clock before starting display updates.
  setupTime();

  // Create the clock task.
  xTaskCreatePinnedToCore(
    clockTask,
    "Clock Task",
    4096,
    NULL,
    1,
    NULL,
    1
  );
}

// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // The clock is handled by clockTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
