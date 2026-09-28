/*
 * =============================================================================
 * TOUCHLESS VOLUME KNOB - FREERTOS VERSION
 * =============================================================================
 *
 * The RPR-0521RS proximity sensor is used as a virtual volume control.
 *
 * Calibration:
 *   Hold the hand steady near the sensor for CALIBRATION_TIME.
 *   The measured proximity becomes the center position.
 *
 * After calibration:
 *
 *   Move hand closer  -> increase NeoPixel brightness
 *   Move hand farther -> decrease NeoPixel brightness
 *
 * FreeRTOS structure:
 *
 *   VolumeTask
 *       - Reads proximity
 *       - Performs calibration
 *       - Converts proximity to brightness
 *       - Updates the NeoPixel matrix
 *
 * The task updates approximately every 50 ms.
 * =============================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <RPR-0521RS.h>
#include <Adafruit_NeoPixel.h>

//====================================================
// I2C Configuration
//====================================================
#define I2C_SDA 5
#define I2C_SCL 6

//====================================================
// NeoPixel Configuration
//====================================================
#define MATRIX_PIN  7
#define NUM_PIXELS  25

//====================================================
// Calibration Configuration
//====================================================
#define CALIBRATION_TIME      2000
#define CALIBRATION_THRESHOLD 200

RPR0521RS sensor;

Adafruit_NeoPixel matrix(
    NUM_PIXELS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

//====================================================
// Calibration State
//====================================================
unsigned long calibrationStart = 0;
bool calibrated = false;

unsigned short centerProximity = 0;

//====================================================
// Volume Task
//====================================================
void VolumeTask(void *parameter)
{
    while (true)
    {
        unsigned short proximity;
        float lux;

        //================================================
        // Read Sensor
        //================================================
        if (
            sensor.get_psalsval(
                &proximity,
                &lux
            ) != 0
        )
        {
            // Sensor read failed.
            // Wait briefly before trying again.
            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        //================================================
        // Calibration Mode
        //================================================
        if (!calibrated)
        {
            // A sufficiently large proximity value means
            // a hand is present for calibration.
            if (proximity > CALIBRATION_THRESHOLD)
            {
                if (calibrationStart == 0)
                {
                    calibrationStart = millis();

                    Serial.println(
                        "Calibration started..."
                    );
                }

                // The hand must remain in approximately
                // the same position for 2 seconds.
                if (
                    millis() - calibrationStart >=
                    CALIBRATION_TIME
                )
                {
                    centerProximity = proximity;
                    calibrated = true;

                    Serial.print(
                        "Calibration complete. Center = "
                    );

                    Serial.println(centerProximity);
                }
            }
            else
            {
                // Hand moved away before calibration
                // completed, so restart the timer.
                calibrationStart = 0;
            }

            vTaskDelay(pdMS_TO_TICKS(50));
            continue;
        }

        //================================================
        // Calculate Position Difference
        //================================================
        int difference =
            (int)proximity -
            (int)centerProximity;

        //================================================
        // Convert Difference to Brightness
        //================================================
        int brightness =
            map(
                constrain(
                    difference,
                    -800,
                    800
                ),
                -800,
                800,
                10,
                255
            );

        brightness = constrain(
            brightness,
            10,
            255
        );

        matrix.setBrightness(brightness);

        //================================================
        // Display Brightness
        //================================================
        // White pixels make the brightness change easy
        // to observe.
        for (int i = 0; i < NUM_PIXELS; i++)
        {
            matrix.setPixelColor(
                i,
                matrix.Color(
                    255,
                    255,
                    255
                )
            );
        }

        matrix.show();

        //================================================
        // Serial Output
        //================================================
        Serial.print("Proximity: ");
        Serial.print(proximity);

        Serial.print(" | Brightness: ");
        Serial.println(brightness);

        // Update the volume control approximately
        // every 50 ms.
        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    // Initialize I2C.
    Wire.begin(
        I2C_SDA,
        I2C_SCL
    );

    // Initialize proximity sensor.
    if (sensor.init() != 0)
    {
        Serial.println("Sensor initialization failed!");

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Initialize NeoPixel matrix.
    matrix.begin();
    matrix.setBrightness(25);
    matrix.clear();
    matrix.show();

    Serial.println(
        "Hold hand steady to calibrate..."
    );

    Serial.println(
        "FreeRTOS Version"
    );

    xTaskCreate(
        VolumeTask,
        "Volume Task",
        4096,
        NULL,
        2,
        NULL
    );
}

//====================================================
// Loop
//====================================================
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
