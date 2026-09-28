/*
 * =============================================================================
 * 5x5 RGB MATRIX - SCROLLING TEXT
 * =============================================================================
 *
 * FreeRTOS version.
 *
 * A dedicated FreeRTOS task continuously scrolls the message horizontally
 * across the 5x5 RGB LED matrix.
 *
 * Matrix:
 *   GPIO 7
 *
 * Supported characters:
 *   A-Z
 *   0-9
 *   Space / unsupported characters are ignored.
 *
 * The original delay() calls have been replaced with vTaskDelay().
 * This allows the FreeRTOS scheduler to run other tasks while the message
 * is being displayed.
 * =============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

#define SCROLL_DELAY_MS 120
#define MESSAGE_PAUSE_MS 300

Adafruit_NeoPixel matrix(
  NUM_LEDS,
  MATRIX_PIN,
  NEO_GRB + NEO_KHZ800
);

// Change this message as required.
String message = "ADVAYAM";

const uint8_t font[36][5] =
{
  // A
  {0x0E,0x11,0x1F,0x11,0x11},
  // B
  {0x1E,0x11,0x1E,0x11,0x1E},
  // C
  {0x0F,0x10,0x10,0x10,0x0F},
  // D
  {0x1E,0x11,0x11,0x11,0x1E},
  // E
  {0x1F,0x10,0x1E,0x10,0x1F},
  // F
  {0x1F,0x10,0x1E,0x10,0x10},
  // G
  {0x0F,0x10,0x17,0x11,0x0F},
  // H
  {0x11,0x11,0x1F,0x11,0x11},
  // I
  {0x1F,0x04,0x04,0x04,0x1F},
  // J
  {0x01,0x01,0x01,0x11,0x0E},
  // K
  {0x11,0x12,0x1C,0x12,0x11},
  // L
  {0x10,0x10,0x10,0x10,0x1F},
  // M
  {0x11,0x1B,0x15,0x11,0x11},
  // N
  {0x11,0x19,0x15,0x13,0x11},
  // O
  {0x0E,0x11,0x11,0x11,0x0E},
  // P
  {0x1E,0x11,0x1E,0x10,0x10},
  // Q
  {0x0E,0x11,0x15,0x12,0x0D},
  // R
  {0x1E,0x11,0x1E,0x12,0x11},
  // S
  {0x0F,0x10,0x0E,0x01,0x1E},
  // T
  {0x1F,0x04,0x04,0x04,0x04},
  // U
  {0x11,0x11,0x11,0x11,0x0E},
  // V
  {0x11,0x11,0x11,0x0A,0x04},
  // W
  {0x11,0x11,0x15,0x1B,0x11},
  // X
  {0x11,0x0A,0x04,0x0A,0x11},
  // Y
  {0x11,0x0A,0x04,0x04,0x04},
  // Z
  {0x1F,0x02,0x04,0x08,0x1F},

  // 0
  {0x0E,0x11,0x11,0x11,0x0E},
  // 1
  {0x04,0x0C,0x04,0x04,0x0E},
  // 2
  {0x0E,0x01,0x06,0x08,0x1F},
  // 3
  {0x1E,0x01,0x06,0x01,0x1E},
  // 4
  {0x12,0x12,0x1F,0x02,0x02},
  // 5
  {0x1F,0x10,0x1E,0x01,0x1E},
  // 6
  {0x0E,0x10,0x1E,0x11,0x0E},
  // 7
  {0x1F,0x01,0x02,0x04,0x04},
  // 8
  {0x0E,0x11,0x0E,0x11,0x0E},
  // 9
  {0x0E,0x11,0x0F,0x01,0x0E}
};

int pixelXY(int x, int y)
{
  if (y % 2 == 0)
    return y * 5 + x;

  return y * 5 + (4 - x);
}

int fontIndex(char c)
{
  c = toupper(c);

  if (c >= 'A' && c <= 'Z')
    return c - 'A';

  if (c >= '0' && c <= '9')
    return 26 + c - '0';

  return -1;
}

void drawColumn(int column, int xOffset)
{
  if (xOffset < 0 || xOffset >= 5)
    return;

  for (int y = 0; y < 5; y++)
  {
    if (column & (1 << (4 - y)))
    {
      matrix.setPixelColor(
        pixelXY(xOffset, y),
        matrix.Color(0, 150, 255)
      );
    }
  }
}

//------------------------------------------------------------------------------
// FreeRTOS scrolling task.
//------------------------------------------------------------------------------
void scrollTask(void *parameter)
{
  while (true)
  {
    // Each character uses five columns plus one blank column.
    int width = message.length() * 6;

    for (int offset = 5; offset >= -width; offset--)
    {
      matrix.clear();

      for (int i = 0; i < message.length(); i++)
      {
        int index = fontIndex(message[i]);

        if (index < 0)
          continue;

        int xBase = offset + i * 6;

        for (int x = 0; x < 5; x++)
        {
          if (xBase + x >= 0 &&
              xBase + x < 5)
          {
            drawColumn(
              font[index][x],
              xBase + x
            );
          }
        }
      }

      matrix.show();

      // Give the CPU to other FreeRTOS tasks.
      vTaskDelay(
        pdMS_TO_TICKS(SCROLL_DELAY_MS)
      );
    }

    vTaskDelay(
      pdMS_TO_TICKS(MESSAGE_PAUSE_MS)
    );
  }
}

void setup()
{
  Serial.begin(115200);

  matrix.begin();
  matrix.setBrightness(25);
  matrix.clear();
  matrix.show();

  // Create the scrolling task.
  xTaskCreate(
    scrollTask,
    "ScrollTask",
    4096,
    NULL,
    1,
    NULL
  );
}

void loop()
{
  // The scrolling is handled by the FreeRTOS task.
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
