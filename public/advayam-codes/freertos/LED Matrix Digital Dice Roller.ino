/*
 * ============================================================================
 * DIGITAL DICE ROLLER - FREERTOS VERSION
 * ============================================================================
 *
 * Pressing the button starts an animated dice-rolling effect.
 *
 * After the animation, a random number from 1 to 6 is selected and
 * displayed on the 5x5 NeoPixel matrix using dice dots.
 *
 * Rolling dots:
 *   Orange
 *
 * Final result:
 *   Green
 *
 * Hardware:
 *   Matrix -> GPIO 7
 *   Button -> GPIO 17
 *   Buzzer -> GPIO 16
 * ============================================================================
 */

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define MATRIX_PIN 7
#define NUM_LEDS 25

#define BUTTON_PIN 17
#define BUZZER_PIN 16

Adafruit_NeoPixel matrix(
    NUM_LEDS,
    MATRIX_PIN,
    NEO_GRB + NEO_KHZ800
);

bool lastButton = HIGH;

//=============================================================================
// Convert x/y coordinates to LED number.
//
// This version retains the original row-major mapping.
//=============================================================================

int pixelXY(int x, int y)
{
    return y * 5 + x;
}

//=============================================================================
// Turn off all LEDs.
//=============================================================================

void clearMatrix()
{
    matrix.clear();
    matrix.show();
}

//=============================================================================
// Set one LED using x/y coordinates.
//=============================================================================

void setPixelXY(
    int x,
    int y,
    uint32_t colour
)
{
    if (x < 0 || x > 4 ||
        y < 0 || y > 4)
    {
        return;
    }

    matrix.setPixelColor(
        pixelXY(x, y),
        colour
    );
}

//=============================================================================
// Draw a dice number from 1 to 6.
//=============================================================================

void drawDie(
    int number,
    uint32_t colour
)
{
    matrix.clear();

    // Centre dot.
    if (number == 1 ||
        number == 3 ||
        number == 5)
    {
        setPixelXY(2, 2, colour);
    }

    // Top-left and bottom-right dots.
    if (number == 2 ||
        number == 3 ||
        number == 4 ||
        number == 5 ||
        number == 6)
    {
        setPixelXY(1, 1, colour);
        setPixelXY(3, 3, colour);
    }

    // Top-right and bottom-left dots.
    if (number == 4 ||
        number == 5 ||
        number == 6)
    {
        setPixelXY(3, 1, colour);
        setPixelXY(1, 3, colour);
    }

    // Middle-left and middle-right dots.
    if (number == 6)
    {
        setPixelXY(1, 2, colour);
        setPixelXY(3, 2, colour);
    }

    matrix.show();
}

//=============================================================================
// Animate the dice and display the final result.
//=============================================================================

void rollDice()
{
    Serial.println("Rolling...");

    for (int i = 0; i < 15; i++)
    {
        int randomNumber = random(1, 7);

        // Display the rolling number in orange.
        drawDie(
            randomNumber,
            matrix.Color(255, 80, 0)
        );

        // Gradually slow down the animation.
        vTaskDelay(
            pdMS_TO_TICKS(80 + i * 10)
        );
    }

    // Select the final dice result.
    int result = random(1, 7);

    // Display the final result in green.
    drawDie(
        result,
        matrix.Color(0, 255, 0)
    );

    Serial.print("Dice result: ");
    Serial.println(result);
}

//=============================================================================
// Dice task.
//
// The task monitors the button and runs the dice animation.
//=============================================================================

void diceTask(void *parameter)
{
    while (true)
    {
        bool button = digitalRead(BUTTON_PIN);

        // Detect a new button press.
        if (lastButton == HIGH &&
            button == LOW)
        {
            rollDice();

            // Basic button debounce.
            vTaskDelay(pdMS_TO_TICKS(200));
        }

        lastButton = button;

        // Allow other FreeRTOS tasks to execute.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//=============================================================================
// Setup.
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    pinMode(
        BUZZER_PIN,
        OUTPUT
    );

    digitalWrite(
        BUZZER_PIN,
        LOW
    );

    randomSeed(
        micros()
    );

    matrix.begin();
    matrix.setBrightness(50);

    clearMatrix();

    Serial.println("Digital Dice Roller");
    Serial.println("Press button to roll.");

    xTaskCreate(
        diceTask,
        "Dice Task",
        4096,
        NULL,
        1,
        NULL
    );
}

//=============================================================================
// Arduino loop.
//
// Dice operation is handled by the FreeRTOS task.
//=============================================================================

void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
