/*
  NTP Clock - GMT+5:30 - Upside-Down Scrolling Time

  This program connects the ESP32-S3 to Wi-Fi and gets the
  current time from an NTP server.

  The clock uses Indian Standard Time (IST):
  UTC + 5 hours 30 minutes.

  The current time is displayed as HH:MM:SS and scrolls
  from right to left across the 5x5 NeoPixel matrix.

  Matrix wiring:

    20 21 22 23 24
    19 18 17 16 15
    10 11 12 13 14
     9  8  7  6  5
     0  1  2  3  4
*/

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25


// Create the LED matrix
Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// =====================================================
// WiFi credentials- Enter YOUR WiFi details here
// =====================================================

const char* WIFI_SSID = "YOUR WIFI SSID";
const char* WIFI_PASSWORD = "YOUR WIFI PASSWORD";


// =====================================================
// IST timezone
// UTC + 5:30
// =====================================================

const long GMT_OFFSET_SECONDS =
  5 * 3600 + 30 * 60;

const int DAYLIGHT_OFFSET_SECONDS = 0;


// =====================================================
// 5x5 number font
// =====================================================

const uint8_t numbers[10][5] = {

  // 0
  {
    0b01110,
    0b10001,
    0b10001,
    0b10001,
    0b01110
  },

  // 1
  {
    0b00100,
    0b01100,
    0b00100,
    0b00100,
    0b01110
  },

  // 2
  {
    0b01110,
    0b10001,
    0b00010,
    0b00100,
    0b11111
  },

  // 3
  {
    0b11110,
    0b00001,
    0b01110,
    0b00001,
    0b11110
  },

  // 4
  {
    0b10010,
    0b10010,
    0b11111,
    0b00010,
    0b00010
  },

  // 5
  {
    0b11111,
    0b10000,
    0b11110,
    0b00001,
    0b11110
  },

  // 6
  {
    0b01110,
    0b10000,
    0b11110,
    0b10001,
    0b01110
  },

  // 7
  {
    0b11111,
    0b00001,
    0b00010,
    0b00100,
    0b00100
  },

  // 8
  {
    0b01110,
    0b10001,
    0b01110,
    0b10001,
    0b01110
  },

  // 9
  {
    0b01110,
    0b10001,
    0b01111,
    0b00001,
    0b01110
  }
};


// =====================================================
// Convert x/y coordinates to LED number
//
// Handles the serpentine wiring:
//
//  20 21 22 23 24
//  19 18 17 16 15
//  10 11 12 13 14
//   9  8  7  6  5
//   0  1  2  3  4
// =====================================================

int pixelXY(int x, int y) {

  // Even rows run left to right
  if (y % 2 == 0) {
    return y * 5 + x;
  }

  // Odd rows run right to left
  return y * 5 + (4 - x);
}


// Set a pixel using x/y coordinates
void setPixelXY(
  int x,
  int y,
  uint32_t color
) {

  // Ignore pixels outside the matrix
  if (x < 0 || x >= 5 ||
      y < 0 || y >= 5) {
    return;
  }

  matrix.setPixelColor(
    pixelXY(x, y),
    color
  );
}


// =====================================================
// Draw one number
//
// The font is flipped vertically,
// but NOT horizontally.
//
// This makes the text upside down
// without making it mirrored.
// =====================================================

void drawNumber(
  int number,
  int xOffset,
  uint32_t color
) {

  for (int y = 0; y < 5; y++) {

    // Reverse row order for upside-down text
    uint8_t row = numbers[number][4 - y];

    for (int x = 0; x < 5; x++) {

      // Keep the font horizontally normal
      if (row & (1 << (4 - x))) {

        setPixelXY(
          x + xOffset,
          y,
          color
        );
      }
    }
  }
}


// =====================================================
// Draw a colon
// =====================================================

void drawColon(
  int xOffset,
  uint32_t color
) {

  // Draw two vertically separated dots
  setPixelXY(
    xOffset + 2,
    1,
    color
  );

  setPixelXY(
    xOffset + 2,
    3,
    color
  );
}


// =====================================================
// Scroll time from RIGHT to LEFT
// =====================================================

void scrollText(String text) {

  // Each character occupies 5 pixels + 1 space
  int textWidth = text.length() * 6;

  // Start on the right and move toward the left
  for (
    int offset = 5;
    offset > -textWidth;
    offset--
  ) {

    // Clear previous frame
    matrix.clear();

    // Draw each character
    for (
      int i = 0;
      i < text.length();
      i++
    ) {

      char c = text[i];

      int xPosition =
        offset + i * 6;

      // Draw number
      if (c >= '0' && c <= '9') {

        drawNumber(
          c - '0',
          xPosition,
          matrix.Color(0, 150, 255)
        );
      }

      // Draw colon
      else if (c == ':') {

        drawColon(
          xPosition,
          matrix.Color(0, 150, 255)
        );
      }
    }

    // Display the frame
    matrix.show();

    // Scrolling speed
    delay(150);
  }
}


// =====================================================
// Connect to WiFi
// =====================================================

void connectWiFi() {

  Serial.print("Connecting to WiFi");

  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );

  // Wait until connected
  while (WiFi.status() != WL_CONNECTED) {

    delay(500);

    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected.");

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
}


// =====================================================
// Setup
// =====================================================

void setup() {

  // Start Serial Monitor
  Serial.begin(115200);

  // Initialize matrix
  matrix.begin();

  // Set brightness
  matrix.setBrightness(25);

  // Turn LEDs off
  matrix.clear();
  matrix.show();

  // Connect to WiFi
  connectWiFi();

  // Configure NTP
  configTime(
    GMT_OFFSET_SECONDS,
    DAYLIGHT_OFFSET_SECONDS,
    "pool.ntp.org",
    "time.nist.gov"
  );

  Serial.println("Waiting for NTP time...");

  struct tm timeinfo;

  // Wait for NTP synchronization
  while (!getLocalTime(&timeinfo)) {

    Serial.println("Waiting...");
    delay(1000);
  }

  Serial.println("NTP synchronized.");
}


// =====================================================
// Main loop
// =====================================================

void loop() {

  struct tm timeinfo;

  // Get current time
  if (!getLocalTime(&timeinfo)) {

    Serial.println("Failed to obtain time.");
    delay(1000);

    return;
  }

  char timeString[20];

  // Format as HH:MM:SS
  strftime(
    timeString,
    sizeof(timeString),
    "%H:%M:%S",
    &timeinfo
  );

  String message =
    String(timeString);

  // Print time to Serial Monitor
  Serial.print("Time: ");
  Serial.println(message);

  // Scroll time right to left
  scrollText(message);

  delay(500);
}
