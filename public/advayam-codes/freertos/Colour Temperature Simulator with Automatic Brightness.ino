/*
 * =============================================================================
 * COLOUR TEMPERATURE SIMULATOR WITH AUTOMATIC BRIGHTNESS
 * FreeRTOS VERSION
 * =============================================================================
 *
 * This program creates a colour-temperature effect on the 5x5 RGB LED matrix.
 *
 * TWO SENSORS ARE USED:
 *
 * 1. LMT87 Temperature Sensor
 *    - Connected to GPIO 1
 *    - Determines the colour of the matrix.
 *
 * 2. RPR-0521RS Ambient Light / Proximity Sensor
 *    - SDA = GPIO 5
 *    - SCL = GPIO 6
 *    - Determines the BRIGHTNESS of the matrix.
 *
 * TEMPERATURE COLOUR MAPPING IS INTENTIONALLY INVERTED:
 *
 *    Cool room  -> Warm amber/orange colour
 *    Warm room  -> Cool blue/white colour
 *
 * LIGHT LEVEL MAPPING:
 *
 *    Dark room   -> Dim matrix
 *    Bright room -> Bright matrix
 *
 * RGB Matrix:
 *    Data = GPIO 7
 *
 * FREErtOS:
 *    A dedicated task reads both sensors and updates the matrix every
 *    500 ms.
 *
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LMT87.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define TEMP_SENSOR_PIN 1

#define I2C_SDA 5
#define I2C_SCL 6

#define MATRIX_PIN 7
#define NUM_PIXELS 25

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define SENSOR_DISPLAY_TASK_STACK_SIZE 4096
#define SENSOR_DISPLAY_TASK_PRIORITY   1

// Take a new measurement every 500 ms.
#define SENSOR_DISPLAY_INTERVAL 500

//=============================================================================
// LMT87 Configuration
//=============================================================================

// ADC resolution used by the LMT87 library.
int ADCres = 10;

// ADC reference voltage for this board.
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//=============================================================================
// Sensor Objects
//=============================================================================

RPR0521RS rpr0521rs;

Adafruit_NeoPixel matrix(
    NUM_PIXELS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//=============================================================================
// Temperature Range
//=============================================================================
//
// These values define the temperature range used for colour mapping.
//
// At or below MIN_TEMPERATURE:
//     Warmest colour
//
// At or above MAX_TEMPERATURE:
//     Coolest colour
//
// Temperatures between these values smoothly transition between
// warm amber and cool blue.
//

#define MIN_TEMPERATURE 15.0
#define MAX_TEMPERATURE 35.0

//=============================================================================
// Ambient Light Range
//=============================================================================
//
// These values define how room brightness controls LED brightness.
//
// At or below MIN_LUX:
//     Matrix is at minimum brightness.
//
// At or above MAX_LUX:
//     Matrix is at maximum brightness.
//

#define MIN_LUX 5.0
#define MAX_LUX 500.0

#define MIN_BRIGHTNESS 10
#define MAX_BRIGHTNESS 180

//=============================================================================
// Read Temperature
//=============================================================================

double readTemperatureC()
{
    // The minus sign is intentional.
    // This is the working LMT87 conversion used on this board.

    double temperatureC =
        -lmt87.getCDegree(
            TEMP_SENSOR_PIN,
            ADCres,
            voltageRef,
            100
        );

    return temperatureC;
}

//=============================================================================
// Update Matrix
//=============================================================================

void updateMatrix(
    double temperatureC,
    float lux
)
{
    //-------------------------------------------------------------------------
    // Calculate Temperature Colour
    //-------------------------------------------------------------------------
    //
    // The temperature mapping is deliberately inverted.
    //
    // Low temperature:
    //     More RED
    //     Less BLUE
    //
    // High temperature:
    //     Less RED
    //     More BLUE
    //
    // Therefore:
    //
    //     Cold room -> Warm colour
    //     Warm room -> Cool colour
    //

    float temperatureRatio =
        (temperatureC - MIN_TEMPERATURE) /
        (MAX_TEMPERATURE - MIN_TEMPERATURE);

    // Keep the ratio inside the usable range.
    temperatureRatio =
        constrain(
            temperatureRatio,
            0.0,
            1.0
        );

    //-------------------------------------------------------------------------
    // Create the Inverted Temperature Colour
    //-------------------------------------------------------------------------

    // Red is strongest when the room is cold.
    int red =
        255 -
        (int)(200 * temperatureRatio);

    // Green is kept relatively moderate so that the colours
    // transition through amber rather than becoming pure green.
    int green =
        100 +
        (int)(100 * (1.0 - temperatureRatio));

    // Blue is strongest when the room is warm.
    int blue =
        30 +
        (int)(225 * temperatureRatio);

    // Make sure all RGB values remain valid.
    red = constrain(red, 0, 255);
    green = constrain(green, 0, 255);
    blue = constrain(blue, 0, 255);

    //-------------------------------------------------------------------------
    // Calculate Brightness From Ambient Lux
    //-------------------------------------------------------------------------
    //
    // The temperature controls colour.
    // The ambient light controls brightness.
    //
    // This means the matrix does not become unnecessarily bright
    // in a dark room.
    //

    float luxRatio =
        (lux - MIN_LUX) /
        (MAX_LUX - MIN_LUX);

    luxRatio =
        constrain(
            luxRatio,
            0.0,
            1.0
        );

    int brightness =
        MIN_BRIGHTNESS +
        (int)(
            (MAX_BRIGHTNESS - MIN_BRIGHTNESS) *
            luxRatio
        );

    brightness =
        constrain(
            brightness,
            MIN_BRIGHTNESS,
            MAX_BRIGHTNESS
        );

    //-------------------------------------------------------------------------
    // Apply Brightness
    //-------------------------------------------------------------------------

    matrix.setBrightness(
        brightness
    );

    //-------------------------------------------------------------------------
    // Update All 25 Pixels
    //-------------------------------------------------------------------------

    uint32_t colour =
        matrix.Color(
            red,
            green,
            blue
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

    // Send the new colour and brightness information
    // to the LEDs.
    matrix.show();
}

//=============================================================================
// Sensor + Display FreeRTOS Task
//=============================================================================

void sensorDisplayTask(void *parameter)
{
    while (true)
    {
        //-------------------------------------------------------------------------
        // Read Temperature
        //-------------------------------------------------------------------------

        // Read the LMT87 using the same conversion method
        // as the known-working temperature example.
        double temperatureC =
            readTemperatureC();

        //-------------------------------------------------------------------------
        // Read Ambient Light
        //-------------------------------------------------------------------------

        unsigned short proximity_raw = 0;
        float lux = 0.0;

        byte rc =
            rpr0521rs.get_psalsval(
                &proximity_raw,
                &lux
            );

        //-------------------------------------------------------------------------
        // Check Ambient Sensor Reading
        //-------------------------------------------------------------------------

        if (rc == 0)
        {
            // Both sensor readings are valid.
            updateMatrix(
                temperatureC,
                lux
            );

            // Display the readings so the user can see
            // how the temperature and brightness are affecting
            // the matrix.

            Serial.print("Temperature : ");
            Serial.print(
                temperatureC,
                1
            );
            Serial.println(" °C");

            Serial.print("Ambient Lux : ");
            Serial.print(
                lux,
                2
            );
            Serial.println(" lx");

            // Display the raw proximity value as
            // additional information.
            Serial.print("Proximity   : ");
            Serial.println(
                proximity_raw
            );

            Serial.println();
        }
        else
        {
            // If the ambient-light sensor cannot be read,
            // don't update the matrix using invalid lux data.
            Serial.println(
                "Error: Failed to read RPR-0521RS."
            );
        }

        //-------------------------------------------------------------------------
        // Wait Before Next Measurement
        //-------------------------------------------------------------------------

        vTaskDelay(
            pdMS_TO_TICKS(
                SENSOR_DISPLAY_INTERVAL
            )
        );
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    //-------------------------------------------------------------------------
    // Serial
    //-------------------------------------------------------------------------

    Serial.begin(115200);

    // On ESP32 hardware, Serial is normally already available.
    // The short delay gives the Serial Monitor time to connect
    // without permanently blocking the FreeRTOS application.

    vTaskDelay(
        pdMS_TO_TICKS(100)
    );

    Serial.println();
    Serial.println(
        "=============================================="
    );

    Serial.println(
        " Colour Temperature + Auto Brightness"
    );

    Serial.println(
        "=============================================="
    );

    //-------------------------------------------------------------------------
    // Initialize I2C
    //-------------------------------------------------------------------------

    // The RPR-0521RS uses the board's dedicated I2C pins.
    if (!Wire.begin(
            I2C_SDA,
            I2C_SCL
        ))
    {
        Serial.println(
            "Failed to initialize I2C!"
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    //-------------------------------------------------------------------------
    // Initialize RPR-0521RS
    //-------------------------------------------------------------------------

    if (rpr0521rs.init() != 0)
    {
        Serial.println(
            "RPR-0521RS initialization failed!"
        );

        Serial.println(
            "Check the sensor and I2C connections."
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }

    Serial.println(
        "RPR-0521RS initialized successfully."
    );

    //-------------------------------------------------------------------------
    // Initialize RGB Matrix
    //-------------------------------------------------------------------------

    matrix.begin();

    // Start with a moderate brightness.
    matrix.setBrightness(50);

    // Clear all LEDs before starting.
    matrix.clear();
    matrix.show();

    Serial.println(
        "RGB matrix initialized."
    );

    Serial.println();

    Serial.println(
        "Temperature controls COLOUR."
    );

    Serial.println(
        "Ambient light controls BRIGHTNESS."
    );

    Serial.println();

    Serial.println(
        "Temperature mapping:"
    );

    Serial.println(
        "  Cold -> Warm Amber"
    );

    Serial.println(
        "  Hot  -> Cool Blue"
    );

    Serial.println();

    //-------------------------------------------------------------------------
    // Create FreeRTOS Sensor/Display Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        sensorDisplayTask,
        "Sensor Display Task",
        SENSOR_DISPLAY_TASK_STACK_SIZE,
        NULL,
        SENSOR_DISPLAY_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================
//
// The actual application is running inside sensorDisplayTask().
//

void loop()
{
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
