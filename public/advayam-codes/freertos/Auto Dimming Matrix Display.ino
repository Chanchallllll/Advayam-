/*
 * =============================================================================
 * AUTO-DIMMING MATRIX DISPLAY - FreeRTOS VERSION
 * =============================================================================
 *
 * Reads ambient light from the ROHM RPR-0521RS sensor and automatically
 * adjusts the brightness of the 5x5 WS2812B RGB matrix.
 *
 * FreeRTOS:
 *   A dedicated task periodically reads the light sensor and updates
 *   the matrix brightness.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * RGB Matrix:
 *   DATA = GPIO 7
 *
 * Sensor:
 *   RPR-0521RS
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define DISPLAY_TASK_STACK_SIZE 4096
#define DISPLAY_TASK_PRIORITY   1

#define DISPLAY_UPDATE_INTERVAL 300

//=============================================================================
// Hardware Objects
//=============================================================================

RPR0521RS rpr0521rs;

Adafruit_NeoPixel matrix(
    NUM_PIXELS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Display Task
//=============================================================================

void displayTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        //-------------------------------------------------------------------------
        // Read Ambient Light
        //-------------------------------------------------------------------------

        if (rpr0521rs.get_psalsval(
                &proximity,
                &lux
            ) == 0)
        {
            // Convert the lux reading into a useful LED brightness.
            //
            // Very dark:
            //     Dim display
            //
            // Very bright:
            //     Bright display
            //
            int brightness = map(
                (int)constrain(
                    lux,
                    0,
                    500
                ),
                0,
                500,
                10,
                255
            );

            matrix.setBrightness(
                brightness
            );

            //-------------------------------------------------------------------------
            // Display Blue Colour
            //-------------------------------------------------------------------------

            uint32_t colour =
                matrix.Color(
                    0,
                    80,
                    255
                );

            for (
                int i = 0;
                i < NUM_PIXELS;
                i++
            )
            {
                matrix.setPixelColor(
                    i,
                    colour
                );
            }

            matrix.show();

            //-------------------------------------------------------------------------
            // Serial Output
            //-------------------------------------------------------------------------

            Serial.print("Lux: ");
            Serial.print(lux, 2);

            Serial.print(" | Brightness: ");
            Serial.println(brightness);
        }
        else
        {
            Serial.println(
                "Failed to read RPR-0521RS."
            );
        }

        // Wait 300 ms before the next sensor reading.
        vTaskDelay(
            pdMS_TO_TICKS(
                DISPLAY_UPDATE_INTERVAL
            )
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    //-------------------------------------------------------------------------
    // Initialize I2C
    //-------------------------------------------------------------------------

    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    //-------------------------------------------------------------------------
    // Initialize RPR-0521RS
    //-------------------------------------------------------------------------

    if (rpr0521rs.init() != 0)
    {
        Serial.println(
            "RPR-0521RS initialization failed!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    //-------------------------------------------------------------------------
    // Initialize Matrix
    //-------------------------------------------------------------------------

    matrix.begin();

    matrix.clear();
    matrix.show();

    Serial.println(
        "Auto-Dimming Matrix Display started."
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Display Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        displayTask,
        "Display Task",
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
    // The matrix is controlled by displayTask().
    vTaskDelay(pdMS_TO_TICKS(1000));
}
