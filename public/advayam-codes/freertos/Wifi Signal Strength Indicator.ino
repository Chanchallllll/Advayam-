/*
 * =============================================================================
 * WI-FI SIGNAL STRENGTH DISPLAY
 * =============================================================================
 *
 * This program connects to Wi-Fi and displays the signal strength
 * as 1 to 4 bars on the 5x5 RGB matrix based on Wi-Fi RSSI.
 *
 * The Wi-Fi signal monitoring and matrix updates run inside
 * a dedicated FreeRTOS task.
 *
 * Hardware:
 *   NeoPixel matrix -> GPIO 7
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

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
// Wi-Fi credentials
// =============================================================================

const char* WIFI_SSID =
  "YOUR_WiFi_SSID";

const char* WIFI_PASSWORD =
  "YOUR_WiFi_PASSWORD";

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
// Set one matrix pixel
// =============================================================================

void setPixel(
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
    matrix.setPixelColor(
      pixelXY(x, y),
      colour
    );
  }
}

// =============================================================================
// Draw Wi-Fi signal bars
// =============================================================================

void drawSignal(
  int bars
)
{
  matrix.clear();

  uint32_t colour =
    matrix.Color(0, 200, 50);

  // Four ascending signal bars.
  for (int bar = 0; bar < 4; bar++)
  {
    int height = bar + 1;

    if (bar < bars)
    {
      for (int h = 0; h < height; h++)
      {
        setPixel(
          bar + 1,
          h,
          colour
        );
      }
    }
  }

  matrix.show();
}

// =============================================================================
// Wi-Fi signal FreeRTOS task
// =============================================================================

void wifiSignalTask(void *parameter)
{
  while (true)
  {
    // Reconnect automatically if Wi-Fi has been lost.
    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println(
        "Wi-Fi connection lost. Reconnecting..."
      );

      WiFi.reconnect();

      vTaskDelay(
        pdMS_TO_TICKS(1000)
      );

      continue;
    }

    // Read the current received signal strength.
    int rssi = WiFi.RSSI();

    int bars;

    // Convert RSSI into four signal levels.
    if (rssi >= -55)
      bars = 4;
    else if (rssi >= -67)
      bars = 3;
    else if (rssi >= -80)
      bars = 2;
    else
      bars = 1;

    Serial.print("RSSI: ");
    Serial.print(rssi);
    Serial.print(" dBm | Bars: ");
    Serial.println(bars);

    drawSignal(bars);

    // Update the display every two seconds.
    vTaskDelay(
      pdMS_TO_TICKS(2000)
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

  // Connect to Wi-Fi before starting the monitoring task.
  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print(
    "Connecting"
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

  // Create the FreeRTOS Wi-Fi monitoring task.
  xTaskCreatePinnedToCore(
    wifiSignalTask,
    "WiFi Signal Task",
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
  // Wi-Fi monitoring is handled by wifiSignalTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
