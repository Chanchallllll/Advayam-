/*
 * =============================================================================
 * NTP CLOCK - FreeRTOS VERSION
 * =============================================================================
 *
 * ESP32-S3
 *
 * Features:
 *
 *   - Connects to WiFi
 *   - Synchronizes time using NTP
 *   - Uses Indian Standard Time (UTC +5:30)
 *   - Displays HH:MM:SS
 *   - Scrolls the time from right to left
 *   - Text is upside-down but NOT mirrored
 *
 * Matrix wiring:
 *
 *    20 21 22 23 24
 *    19 18 17 16 15
 *    10 11 12 13 14
 *     9  8  7  6  5
 *     0  1  2  3  4
 *
 *
 * FreeRTOS tasks:
 *
 *   1. WiFiTask
 *        Connects to WiFi and maintains the connection.
 *
 *   2. TimeTask
 *        Reads the current NTP time and updates the shared time string.
 *
 *   3. DisplayTask
 *        Continuously scrolls the latest time across the matrix.
 *
 *
 * Synchronization:
 *
 *   timeMutex protects the shared time string.
 *   displayMutex protects the NeoPixel matrix.
 * =============================================================================
 */

#include <Arduino.h>
#include <WiFi.h>
#include <time.h>
#include <Adafruit_NeoPixel.h>


// =============================================================================
// Pins
// =============================================================================

#define MATRIX_PIN 7
#define NUM_LEDS   25


// =============================================================================
// NeoPixel matrix
// =============================================================================

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);


// =============================================================================
// WiFi credentials
// =============================================================================

const char* WIFI_SSID =
  "YOUR_WIFI_SSID";

const char* WIFI_PASSWORD =
  "YOUR_WIFI_PASSWORD";


// =============================================================================
// IST timezone
// =============================================================================

const long GMT_OFFSET_SECONDS =
  5 * 3600 + 30 * 60;

const int DAYLIGHT_OFFSET_SECONDS =
  0;


// =============================================================================
// Shared time string
// =============================================================================
//
// The TimeTask updates this.
// The DisplayTask reads this.
//

String currentTime =
  "00:00:00";


// =============================================================================
// FreeRTOS synchronization
// =============================================================================

SemaphoreHandle_t timeMutex;

SemaphoreHandle_t displayMutex;


// =============================================================================
// 5x5 number font
// =============================================================================

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


// =============================================================================
// Matrix coordinate conversion
// =============================================================================

int pixelXY(
  int x,
  int y
)
{
  // Even rows run left to right.
  if (y % 2 == 0)
  {
    return y * 5 + x;
  }

  // Odd rows run right to left.
  return y * 5 + (4 - x);
}


// =============================================================================
// Set pixel using x/y coordinates
// =============================================================================

void setPixelXY(
  int x,
  int y,
  uint32_t color
)
{
  if (
    x < 0 ||
    x >= 5 ||
    y < 0 ||
    y >= 5
  )
  {
    return;
  }


  matrix.setPixelColor(
    pixelXY(x, y),
    color
  );
}


// =============================================================================
// Draw one number
// =============================================================================
//
// The font is flipped vertically,
// but NOT horizontally.
//
// This produces upside-down text
// without making it mirrored.
//

void drawNumber(
  int number,
  int xOffset,
  uint32_t color
)
{
  for (int y = 0; y < 5; y++)
  {
    // Reverse row order.
    uint8_t row =
      numbers[number][4 - y];


    for (int x = 0; x < 5; x++)
    {
      if (
        row & (1 << (4 - x))
      )
      {
        setPixelXY(
          x + xOffset,
          y,
          color
        );
      }
    }
  }
}


// =============================================================================
// Draw colon
// =============================================================================

void drawColon(
  int xOffset,
  uint32_t color
)
{
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


// =============================================================================
// Draw one scrolling frame
// =============================================================================

void drawScrollFrame(
  const String &text,
  int offset
)
{
  matrix.clear();


  uint32_t color =
    matrix.Color(
      0,
      150,
      255
    );


  // ---------------------------------------------------------------------------
  // Draw every character.
  // ---------------------------------------------------------------------------

  for (
    int i = 0;
    i < text.length();
    i++
  )
  {
    char c =
      text[i];


    int xPosition =
      offset + i * 6;


    // Number.
    if (
      c >= '0' &&
      c <= '9'
    )
    {
      drawNumber(
        c - '0',
        xPosition,
        color
      );
    }


    // Colon.
    else if (c == ':')
    {
      drawColon(
        xPosition,
        color
      );
    }
  }


  matrix.show();
}


// =============================================================================
// Get a safe copy of the current time
// =============================================================================

String getCurrentTime()
{
  String result;


  if (
    xSemaphoreTake(
      timeMutex,
      portMAX_DELAY
    ) == pdTRUE
  )
  {
    result =
      currentTime;


    xSemaphoreGive(
      timeMutex
    );
  }


  return result;
}


// =============================================================================
// WiFi Task
// =============================================================================

void WiFiTask(
  void *parameter
)
{
  while (true)
  {
    // -------------------------------------------------------------------------
    // Check WiFi connection.
    // -------------------------------------------------------------------------

    if (
      WiFi.status() != WL_CONNECTED
    )
    {
      Serial.println(
        "WiFi disconnected."
      );


      Serial.println(
        "Attempting reconnection..."
      );


      WiFi.disconnect();


      WiFi.begin(
        WIFI_SSID,
        WIFI_PASSWORD
      );


      // -----------------------------------------------------------------------
      // Wait for connection.
      // -----------------------------------------------------------------------

      unsigned long start =
        millis();


      while (
        WiFi.status() != WL_CONNECTED &&
        millis() - start < 15000
      )
      {
        vTaskDelay(
          pdMS_TO_TICKS(500)
        );


        Serial.print(".");
      }


      Serial.println();


      if (
        WiFi.status() == WL_CONNECTED
      )
      {
        Serial.println(
          "WiFi connected."
        );


        Serial.print(
          "IP: "
        );


        Serial.println(
          WiFi.localIP()
        );
      }
      else
      {
        Serial.println(
          "WiFi connection failed."
        );
      }
    }


    // -------------------------------------------------------------------------
    // Check every 5 seconds.
    // -------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(5000)
    );
  }
}


// =============================================================================
// Time Task
// =============================================================================

void TimeTask(
  void *parameter
)
{
  while (true)
  {
    // -------------------------------------------------------------------------
    // Only attempt to read time when WiFi is available.
    // -------------------------------------------------------------------------

    if (
      WiFi.status() == WL_CONNECTED
    )
    {
      struct tm timeinfo;


      if (
        getLocalTime(
          &timeinfo
        )
      )
      {
        char timeString[20];


        // ---------------------------------------------------------------------
        // HH:MM:SS
        // ---------------------------------------------------------------------

        strftime(
          timeString,
          sizeof(timeString),
          "%H:%M:%S",
          &timeinfo
        );


        // ---------------------------------------------------------------------
        // Update shared time string.
        // ---------------------------------------------------------------------

        if (
          xSemaphoreTake(
            timeMutex,
            pdMS_TO_TICKS(100)
          ) == pdTRUE
        )
        {
          currentTime =
            String(timeString);


          xSemaphoreGive(
            timeMutex
          );
        }


        Serial.print(
          "Time: "
        );


        Serial.println(
          timeString
        );
      }
      else
      {
        Serial.println(
          "Failed to obtain time."
        );
      }
    }


    // -------------------------------------------------------------------------
    // Update once per second.
    // -------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(1000)
    );
  }
}


// =============================================================================
// Display Task
// =============================================================================

void DisplayTask(
  void *parameter
)
{
  while (true)
  {
    // -------------------------------------------------------------------------
    // Get a snapshot of the current time.
    //
    // Once copied, the mutex is released and the display
    // can scroll without blocking the TimeTask.
    // -------------------------------------------------------------------------

    String text =
      getCurrentTime();


    int textWidth =
      text.length() * 6;


    // -------------------------------------------------------------------------
    // Scroll from right to left.
    // -------------------------------------------------------------------------

    for (
      int offset = 5;
      offset > -textWidth;
      offset--
    )
    {
      // -----------------------------------------------------------------------
      // Protect the NeoPixel matrix.
      // -----------------------------------------------------------------------

      if (
        xSemaphoreTake(
          displayMutex,
          portMAX_DELAY
        ) == pdTRUE
      )
      {
        drawScrollFrame(
          text,
          offset
        );


        xSemaphoreGive(
          displayMutex
        );
      }


      // -----------------------------------------------------------------------
      // Scroll speed.
      // -----------------------------------------------------------------------

      vTaskDelay(
        pdMS_TO_TICKS(150)
      );
    }


    // -------------------------------------------------------------------------
    // Small pause before fetching the newest time.
    // -------------------------------------------------------------------------

    vTaskDelay(
      pdMS_TO_TICKS(100)
    );
  }
}


// =============================================================================
// Setup
// =============================================================================

void setup()
{
  Serial.begin(115200);


  // ===========================================================================
  // Initialize matrix.
  // ===========================================================================

  matrix.begin();


  matrix.setBrightness(
    25
  );


  matrix.clear();

  matrix.show();


  // ===========================================================================
  // Create mutexes.
  // ===========================================================================

  timeMutex =
    xSemaphoreCreateMutex();


  displayMutex =
    xSemaphoreCreateMutex();


  if (
    timeMutex == NULL ||
    displayMutex == NULL
  )
  {
    Serial.println(
      "ERROR: Failed to create mutex."
    );


    while (true)
    {
      delay(1000);
    }
  }


  // ===========================================================================
  // Start WiFi.
  // ===========================================================================

  WiFi.mode(
    WIFI_STA
  );


  WiFi.begin(
    WIFI_SSID,
    WIFI_PASSWORD
  );


  Serial.print(
    "Connecting to WiFi"
  );


  // ===========================================================================
  // Wait for initial WiFi connection.
  //
  // This happens only once during startup.
  // After that, WiFiTask takes over.
  // ===========================================================================

  while (
    WiFi.status() != WL_CONNECTED
  )
  {
    delay(500);

    Serial.print(".");
  }


  Serial.println();


  Serial.println(
    "WiFi connected."
  );


  Serial.print(
    "IP: "
  );


  Serial.println(
    WiFi.localIP()
  );


  // ===========================================================================
  // Configure NTP.
  // ===========================================================================

  configTime(
    GMT_OFFSET_SECONDS,
    DAYLIGHT_OFFSET_SECONDS,
    "pool.ntp.org",
    "time.nist.gov"
  );


  // ===========================================================================
  // Wait for initial NTP synchronization.
  // ===========================================================================

  Serial.println(
    "Waiting for NTP time..."
  );


  struct tm timeinfo;


  while (
    !getLocalTime(
      &timeinfo
    )
  )
  {
    Serial.println(
      "Waiting..."
    );


    delay(1000);
  }


  Serial.println(
    "NTP synchronized."
  );


  // ===========================================================================
  // Get initial time.
  // ===========================================================================

  char timeString[20];


  strftime(
    timeString,
    sizeof(timeString),
    "%H:%M:%S",
    &timeinfo
  );


  currentTime =
    String(timeString);


  // ===========================================================================
  // Create FreeRTOS tasks.
  // ===========================================================================

  xTaskCreatePinnedToCore(
    WiFiTask,
    "WiFiTask",
    4096,
    NULL,
    1,
    NULL,
    0
  );


  xTaskCreatePinnedToCore(
    TimeTask,
    "TimeTask",
    4096,
    NULL,
    2,
    NULL,
    0
  );


  xTaskCreatePinnedToCore(
    DisplayTask,
    "DisplayTask",
    4096,
    NULL,
    1,
    NULL,
    1
  );


  Serial.println(
    "NTP Clock FreeRTOS started."
  );
}


// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // ===========================================================================
  // Everything is handled by FreeRTOS tasks.
  // ===========================================================================

  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
