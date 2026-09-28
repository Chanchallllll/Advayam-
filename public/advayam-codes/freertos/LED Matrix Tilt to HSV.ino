/*
 * =============================================================================
 * TILT ANGLE TO HSV MATRIX COLOUR - FreeRTOS VERSION
 * =============================================================================
 *
 * This program maps the board's physical tilt angle to HSV hue.
 *
 * Rotating the board through 360 degrees produces a corresponding
 * 360-degree colour cycle across the entire matrix.
 *
 * FreeRTOS:
 *   A dedicated task continuously reads the accelerometer and updates
 *   the NeoPixel matrix.
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

#define IMU_CS 10
#define IMU_SCK 12
#define IMU_MOSI 11
#define IMU_MISO 13

//=============================================================================
// FreeRTOS Task Configuration
//=============================================================================

#define MATRIX_TASK_STACK_SIZE 4096
#define MATRIX_TASK_PRIORITY   1

// Task period in milliseconds.
#define MATRIX_UPDATE_INTERVAL 30

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
// Matrix Task
//=============================================================================

void matrixTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read the latest IMU measurement.
        imu.getEvent(
            &accel,
            &gyro,
            &temp
        );

        // Calculate the physical angle from the X/Y acceleration.
        float angle = atan2(
            accel.acceleration.y,
            accel.acceleration.x
        ) * 180.0 / PI;

        // Convert negative angles into the 0-360 degree range.
        if (angle < 0)
        {
            angle += 360;
        }

        // Convert the angle into the NeoPixel HSV hue range.
        uint16_t hue =
            (uint16_t)(angle * 65535.0 / 360.0);

        // Generate the corresponding HSV colour.
        uint32_t colour =
            matrix.gamma32(
                matrix.ColorHSV(
                    hue,
                    255,
                    255
                )
            );

        // Display the colour across the complete matrix.
        matrix.fill(colour);
        matrix.show();

        // Give the CPU to other FreeRTOS tasks.
        vTaskDelay(
            pdMS_TO_TICKS(MATRIX_UPDATE_INTERVAL)
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

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println(
            "IMU initialization failed!"
        );

        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    imu.setAccelRange(
        LSM6DS_ACCEL_RANGE_4_G
    );

    imu.setAccelDataRate(
        LSM6DS_RATE_104_HZ
    );

    Serial.println(
        "Tilt Colour Matrix Ready"
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Matrix Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        matrixTask,
        "Tilt Matrix Task",
        MATRIX_TASK_STACK_SIZE,
        NULL,
        MATRIX_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================
//
// The matrix is controlled by the FreeRTOS task.
//

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
