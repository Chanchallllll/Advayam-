/*
 * This program creates a temperature-controlled fire effect
 * on the 5x5 RGB LED matrix using the LMT87 sensor inside a FreeRTOS task.
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include <LMT87.h>

#define MATRIX_PIN 7
#define TEMP_PIN   1
#define NUM_LEDS   25

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

LMT87 lmt87 = LMT87();

int ADCres = 10;
double voltageRef = 3.3;

uint8_t heat[5][5];

int pixelXY(int x, int y)
{
    if (y % 2 == 0)
    {
        return y * 5 + x;
    }

    return y * 5 + (4 - x);
}

uint32_t heatColor(uint8_t temperature)
{
    if (temperature < 85)
    {
        return matrix.Color(temperature * 3, 0, 0);
    }

    if (temperature < 170)
    {
        return matrix.Color(255, (temperature - 85) * 3, 0);
    }

    return matrix.Color(255, 255, (temperature - 170) * 3);
}

void clearMatrix()
{
    matrix.clear();
    matrix.show();
}

void generateFire(float temperatureC)
{
    if (temperatureC < 30.0)
    {
        clearMatrix();
        return;
    }

    float strength =
        map(
            constrain(
                (int)(temperatureC * 10),
                300,
                600
            ),
            300,
            600,
            60,
            255
        );

    for (int y = 0; y < 5; y++)
    {
        for (int x = 0; x < 5; x++)
        {
            int cooling = random(0, 35);
            if (heat[y][x] > cooling)
            {
                heat[y][x] -= cooling;
            }
            else
            {
                heat[y][x] = 0;
            }
        }
    }

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

    for (int x = 0; x < 5; x++)
    {
        int variation = random(-30, 31);
        int newHeat = constrain((int)strength + variation, 0, 255);
        heat[0][x] = newHeat;
    }

    int flameHeight =
        map(
            constrain(
                (int)(temperatureC * 10),
                300,
                600
            ),
            300,
            600,
            1,
            5
        );

    matrix.clear();

    for (int y = 0; y < flameHeight; y++)
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

// FreeRTOS Task for temperature-controlled fire simulation
void fireEffectTask(void *pvParameters)
{
    while (true)
    {
        double temperatureC =
            -lmt87.getCDegree(
                TEMP_PIN,
                ADCres,
                voltageRef,
                100
            );

        Serial.print("Temperature: ");
        Serial.print(temperatureC, 1);
        Serial.println(" C");

        generateFire(temperatureC);

        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void setup()
{
    Serial.begin(115200);

    matrix.begin();
    matrix.setBrightness(25);

    randomSeed(analogRead(TEMP_PIN));
    clearMatrix();

    Serial.println("Temperature Controlled Fire");

    xTaskCreate(
        fireEffectTask,
        "FireTask",
        2048,
        NULL,
        1,
        NULL
    );
}

void loop()
{
    vTaskDelete(NULL);
}