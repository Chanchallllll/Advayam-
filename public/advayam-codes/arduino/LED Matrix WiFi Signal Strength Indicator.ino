/*
 * This program connects to Wi-Fi and displays the signal strength
 * as 1 to 4 bars on the 5x5 RGB matrix based on Wi-Fi RSSI.
 */

#include <Arduino.h>
#include <WiFi.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

const char* WIFI_SSID =
  "YOUR WiFi SSID";

const char* WIFI_PASSWORD =
  "YOUR WiFi PASSWORD";

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

void setPixel(int x, int y, uint32_t color)
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

void drawSignal(int bars)
{
  matrix.clear();

  uint32_t color =
    matrix.Color(0, 200, 50);

  // Four ascending signal bars
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
          color
        );
      }
    }
  }

  matrix.show();
}

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  Serial.print("Connecting");

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
}

void loop()
{
  int rssi = WiFi.RSSI();

  int bars;

  // Convert RSSI into four levels
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

  delay(2000);
}
