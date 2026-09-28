/*
 * =============================================================================
 * AUTOMATIC TEXT ORIENTATION AND SCROLLING - FreeRTOS VERSION
 * =============================================================================
 *
 * This program automatically rotates scrolling text according to
 * the physical orientation of the board.
 *
 * The accelerometer detects the board orientation and the text
 * is rotated accordingly. The font is horizontally corrected so
 * letters appear normally on the physical matrix.
 *
 * The default message is "ADVAYAM".
 *
 * New text entered through the Serial Monitor replaces the current message.
 *
 * FreeRTOS:
 *   A dedicated display task handles IMU readings, text rendering,
 *   scrolling and Serial input.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// Matrix Configuration
//=============================================================================

#define MATRIX_PIN 7
#define NUM_LEDS 25

//=============================================================================
// IMU Configuration
//=============================================================================

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//=============================================================================
// Other GPIO
//=============================================================================

#define UNUSED_OUTPUT_PIN 16

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define DISPLAY_TASK_STACK_SIZE 6144
#define DISPLAY_TASK_PRIORITY   1

#define SCROLL_INTERVAL 100

// Set to true to correct the horizontal font mirroring.
#define MIRROR_FONT true

//=============================================================================
// Hardware Objects
//=============================================================================

Adafruit_LSM6DSOX imu;

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Text Variables
//=============================================================================

String message = "ADVAYAM";
String input = "";

//=============================================================================
// 5x5 Font
//=============================================================================

const uint8_t font[26][5] = {

    {0b01110,0b10001,0b11111,0b10001,0b10001}, // A
    {0b11110,0b10001,0b11110,0b10001,0b11110}, // B
    {0b01111,0b10000,0b10000,0b10000,0b01111}, // C
    {0b11110,0b10001,0b10001,0b10001,0b11110}, // D
    {0b11111,0b10000,0b11110,0b10000,0b11111}, // E
    {0b11111,0b10000,0b11110,0b10000,0b10000}, // F
    {0b01111,0b10000,0b10111,0b10001,0b01111}, // G
    {0b10001,0b10001,0b11111,0b10001,0b10001}, // H
    {0b01110,0b00100,0b00100,0b00100,0b01110}, // I
    {0b00111,0b00010,0b00010,0b10010,0b01100}, // J
    {0b10001,0b10010,0b11100,0b10010,0b10001}, // K
    {0b10000,0b10000,0b10000,0b10000,0b11111}, // L
    {0b10001,0b11011,0b10101,0b10001,0b10001}, // M
    {0b10001,0b11001,0b10101,0b10011,0b10001}, // N
    {0b01110,0b10001,0b10001,0b10001,0b01110}, // O
    {0b11110,0b10001,0b11110,0b10000,0b10000}, // P
    {0b01110,0b10001,0b10001,0b10101,0b01111}, // Q
    {0b11110,0b10001,0b11110,0b10100,0b10010}, // R
    {0b01111,0b10000,0b01110,0b00001,0b11110}, // S
    {0b11111,0b00100,0b00100,0b00100,0b00100}, // T
    {0b10001,0b10001,0b10001,0b10001,0b01110}, // U
    {0b10001,0b10001,0b10001,0b01010,0b00100}, // V
    {0b10001,0b10001,0b10101,0b11011,0b10001}, // W
    {0b10001,0b01010,0b00100,0b01010,0b10001}, // X
    {0b10001,0b01010,0b00100,0b00100,0b00100}, // Y
    {0b11111,0b00010,0b00100,0b01000,0b11111}  // Z
};

//=============================================================================
// Matrix Coordinate Conversion
//=============================================================================

int pixelXY(int x, int y)
{
    if (
        x < 0 ||
        x >= 5 ||
        y < 0 ||
        y >= 5
    )
    {
        return -1;
    }

    // Even rows run left to right.
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    // Odd rows run right to left.
    return y * 5 + (4 - x);
}

//=============================================================================
// Set Pixel Using X/Y Coordinates
//=============================================================================

void setXY(
    int x,
    int y,
    uint32_t colour
)
{
    int index = pixelXY(x, y);

    if (index >= 0)
    {
        matrix.setPixelColor(
            index,
            colour
        );
    }
}

//=============================================================================
// Read Board Orientation
//=============================================================================
//
// 0 = +X
// 1 = -X
// 2 = +Y
// 3 = -Y
//

int getOrientation()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    imu.getEvent(
        &accel,
        &gyro,
        &temp
    );

    float x =
        accel.acceleration.x;

    float y =
        accel.acceleration.y;

    float z =
        accel.acceleration.z;

    float ax = fabs(x);
    float ay = fabs(y);
    float az = fabs(z);

    // X axis is dominant.
    if (ax >= ay && ax >= az)
    {
        if (x > 0)
        {
            return 1;
        }

        return 0;
    }

    // Y axis is dominant.
    if (ay >= ax && ay >= az)
    {
        if (y > 0)
        {
            return 2;
        }

        return 3;
    }

    // Board is approximately flat on its face.
    if (z > 0)
    {
        return 0;
    }

    return 1;
}

//=============================================================================
// Draw Rotated Character
//=============================================================================

void drawChar(
    char c,
    int xOffset,
    int orientation,
    uint32_t colour
)
{
    // Convert lowercase characters to uppercase.
    if (c >= 'a' && c <= 'z')
    {
        c -= 32;
    }

    // Ignore unsupported characters.
    if (c < 'A' || c > 'Z')
    {
        return;
    }

    const uint8_t* glyph =
        font[c - 'A'];

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            // Read the original font pixel.
            if (!(glyph[y] &
                  (1 << (4 - x))))
            {
                continue;
            }

            int correctedX = x;

            // Correct horizontal mirroring.
            if (MIRROR_FONT)
            {
                correctedX = 4 - x;
            }

            int rx = correctedX;
            int ry = y;

            // Apply physical board orientation.
            if (orientation == 1)
            {
                // 180 degrees.
                rx = 4 - correctedX;
                ry = 4 - y;
            }
            else if (orientation == 2)
            {
                // 90 degrees.
                rx = y;
                ry = 4 - correctedX;
            }
            else if (orientation == 3)
            {
                // 270 degrees.
                rx = 4 - y;
                ry = correctedX;
            }

            setXY(
                rx + xOffset,
                ry,
                colour
            );
        }
    }
}

//=============================================================================
// Process Serial Input
//=============================================================================

void checkSerialInput()
{
    while (Serial.available())
    {
        char c = Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (input.length() > 0)
            {
                message = input;
                input = "";

                Serial.print("New text: ");
                Serial.println(message);
            }
        }
        else
        {
            input += c;
        }
    }
}

//=============================================================================
// Scroll Text
//=============================================================================

void scrollText()
{
    int width =
        message.length() * 6;

    // Determine orientation once for this scroll.
    int orientation =
        getOrientation();

    for (
        int offset = 5;
        offset > -width;
        offset--
    )
    {
        // Check for new Serial input during scrolling.
        checkSerialInput();

        matrix.clear();

        uint32_t colour =
            matrix.Color(
                0,
                180,
                255
            );

        // Draw every character.
        for (
            int i = 0;
            i < message.length();
            i++
        )
        {
            drawChar(
                message[i],
                offset + i * 6,
                orientation,
                colour
            );
        }

        matrix.show();

        // Give the CPU to other FreeRTOS tasks.
        vTaskDelay(
            pdMS_TO_TICKS(
                SCROLL_INTERVAL
            )
        );
    }
}

//=============================================================================
// FreeRTOS Display Task
//=============================================================================

void displayTask(void *parameter)
{
    while (true)
    {
        scrollText();

        // Check Serial again after completing a scroll.
        checkSerialInput();

        vTaskDelay(
            pdMS_TO_TICKS(10)
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(
        UNUSED_OUTPUT_PIN,
        OUTPUT
    );

    digitalWrite(
        UNUSED_OUTPUT_PIN,
        LOW
    );

    //-------------------------------------------------------------------------
    // Initialize Matrix
    //-------------------------------------------------------------------------

    matrix.begin();

    matrix.setBrightness(25);

    matrix.clear();
    matrix.show();

    //-------------------------------------------------------------------------
    // Initialize SPI
    //-------------------------------------------------------------------------

    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    //-------------------------------------------------------------------------
    // Initialize IMU
    //-------------------------------------------------------------------------

    if (!imu.begin_SPI(
            IMU_CS,
            &SPI
        ))
    {
        Serial.println(
            "IMU initialization failed!"
        );

        while (true)
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

    Serial.println(
        "Automatic Text Orientation Ready"
    );

    Serial.println(
        "Default text: ADVAYAM"
    );

    Serial.println(
        "Enter new text and press Enter."
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Display Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        displayTask,
        "Text Display Task",
        DISPLAY_TASK_STACK_SIZE,
        NULL,
        DISPLAY_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    // The actual application runs inside displayTask().
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
