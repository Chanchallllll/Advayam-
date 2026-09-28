/*
 * Fire Effect - FreeRTOS Version
 *
 * This program creates a flickering fire effect on the 5x5
 * RGB LED matrix using a simulated heat map.
 *
 * Heat is converted into red, orange, and yellow colors.
 *
 * FreeRTOS tasks:
 *   1. HeatTask  - Updates the simulated heat map.
 *   2. DrawTask  - Displays the heat map on the LED matrix.
 *
 * A mutex protects the shared heat map.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);


// ============================================================================
// Heat map
// ============================================================================

uint8_t heat[5][5];


// Mutex protecting the heat map.
SemaphoreHandle_t heatMutex;


// ============================================================================
// Convert coordinates to the serpentine wiring
// ============================================================================

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}


// ============================================================================
// Convert heat value to fire color
// ============================================================================

uint32_t heatColor(
    uint8_t temperature
)
{
    if (temperature < 85)
    {
        return matrix.Color(
            temperature * 3,
            0,
            0
        );
    }

    if (temperature < 170)
    {
        uint8_t green =
            (temperature - 85) * 3;

        return matrix.Color(
            255,
            green,
            0
        );
    }

    return matrix.Color(
        255,
        255,
        (temperature - 170) * 3
    );
}


// ============================================================================
// Update the simulated heat
// ============================================================================

void updateHeat()
{
    // Cool the flames.
    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            int cooling =
                random(0, 40);

            heat[y][x] =
                (heat[y][x] > cooling)
                ? heat[y][x] - cooling
                : 0;
        }
    }


    // Move heat upward.
    for (int y = 4; y > 0; y--)
    {
        for (int x = 0; x < 5; x++)
        {
            heat[y][x] =
                (
                    heat[y - 1][x] +
                    heat[y - 1][max(0, x - 1)] +
                    heat[y - 1][min(4, x + 1)]
                ) / 3;
        }
    }


    // Add new heat at the bottom.
    for (int x = 0; x < 5; x++)
    {
        heat[0][x] =
            random(160, 256);
    }
}


// ============================================================================
// Draw the fire
// ============================================================================

void drawFire()
{
    matrix.clear();

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            matrix.setPixelColor(
                pixelXY(x, y),
                heatColor(heat[y][x])
            );
        }
    }

    matrix.show();
}


// ============================================================================
// Heat simulation task
// ============================================================================

void HeatTask(void *parameter)
{
    while (true)
    {
        if (xSemaphoreTake(
                heatMutex,
                portMAX_DELAY
            ) == pdTRUE)
        {
            updateHeat();

            xSemaphoreGive(
                heatMutex
            );
        }

        // Update the fire approximately every 80 ms.
        vTaskDelay(
            pdMS_TO_TICKS(80)
        );
    }
}


// ============================================================================
// Display task
// ============================================================================

void DrawTask(void *parameter)
{
    uint8_t displayHeat[5][5];

    while (true)
    {
        // Copy the heat map while protected.
        if (xSemaphoreTake(
                heatMutex,
                portMAX_DELAY
            ) == pdTRUE)
        {
            memcpy(
                displayHeat,
                heat,
                sizeof(heat)
            );

            xSemaphoreGive(
                heatMutex
            );
        }


        // Draw from the local copy so the mutex
        // does not need to remain locked during matrix.show().
        matrix.clear();

        for (int y = 0; y < 5; y++)
        {
            for (int x = 0; x < 5; x++)
            {
                matrix.setPixelColor(
                    pixelXY(x, y),
                    heatColor(displayHeat[y][x])
                );
            }
        }

        matrix.show();

        vTaskDelay(
            pdMS_TO_TICKS(80)
        );
    }
}


// ============================================================================
// Setup
// ============================================================================

void setup()
{
    Serial.begin(115200);

    matrix.begin();

    matrix.setBrightness(15);

    randomSeed(
        analogRead(1)
    );

    matrix.clear();
    matrix.show();


    // Create mutex.
    heatMutex =
        xSemaphoreCreateMutex();

    if (heatMutex == NULL)
    {
        Serial.println(
            "ERROR: Failed to create heat mutex."
        );

        while (true)
        {
            vTaskDelay(
                pdMS_TO_TICKS(1000)
            );
        }
    }


    // Create FreeRTOS tasks.

    xTaskCreatePinnedToCore(
        HeatTask,
        "HeatTask",
        2048,
        NULL,
        1,
        NULL,
        0
    );

    xTaskCreatePinnedToCore(
        DrawTask,
        "DrawTask",
        2048,
        NULL,
        1,
        NULL,
        1
    );
}


// ============================================================================
// Arduino loop
// ============================================================================

void loop()
{
    // All application work is performed by FreeRTOS tasks.
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
