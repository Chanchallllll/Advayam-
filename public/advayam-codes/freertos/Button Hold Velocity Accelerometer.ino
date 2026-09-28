/*
 * =============================================================================
 * BUTTON HOLD PITCH BUZZER - FreeRTOS VERSION
 * =============================================================================
 *
 * This program measures how long the push button on GPIO 17
 * is held and increases the buzzer frequency as the hold time
 * becomes longer.
 *
 * Releasing the button stops the buzzer.
 *
 * FreeRTOS:
 *   A dedicated task monitors the button and updates the buzzer frequency.
 *
 * =============================================================================
 */

#include <Arduino.h>

//=============================================================================
// GPIO Configuration
//=============================================================================

#define BUTTON_PIN 17
#define BUZZER_PIN 16

//=============================================================================
// Buzzer PWM Configuration
//=============================================================================

#define BUZZER_CHANNEL 0
#define BUZZER_RES     8

//=============================================================================
// FreeRTOS Configuration
//=============================================================================

#define BUTTON_TASK_STACK_SIZE 4096
#define BUTTON_TASK_PRIORITY   1

#define BUTTON_CHECK_INTERVAL 10

//=============================================================================
// Button State
//=============================================================================

unsigned long pressStart = 0;
bool buttonPressed = false;

//=============================================================================
// Button/Buzzer Task
//=============================================================================

void buttonBuzzerTask(void *parameter)
{
    while (true)
    {
        bool pressed =
            digitalRead(BUTTON_PIN) == LOW;

        //-------------------------------------------------------------------------
        // Detect Button Press
        //-------------------------------------------------------------------------

        if (
            pressed &&
            !buttonPressed
        )
        {
            buttonPressed = true;

            pressStart = millis();

            Serial.println(
                "Button pressed"
            );
        }

        //-------------------------------------------------------------------------
        // Increase Pitch While Held
        //-------------------------------------------------------------------------

        if (pressed)
        {
            unsigned long holdTime =
                millis() - pressStart;

            int frequency =
                300 +
                (holdTime / 10);

            // Limit the maximum frequency.
            frequency =
                constrain(
                    frequency,
                    300,
                    4000
                );

            ledcWriteTone(
                BUZZER_CHANNEL,
                frequency
            );

            Serial.print(
                "Hold Time: "
            );

            Serial.print(
                holdTime
            );

            Serial.print(
                " ms | Frequency: "
            );

            Serial.println(
                frequency
            );
        }

        //-------------------------------------------------------------------------
        // Detect Button Release
        //-------------------------------------------------------------------------

        if (
            !pressed &&
            buttonPressed
        )
        {
            buttonPressed = false;

            // Stop the buzzer.
            ledcWrite(
                BUZZER_CHANNEL,
                0
            );

            Serial.println(
                "Button released"
            );
        }

        // Give the CPU to other FreeRTOS tasks.
        vTaskDelay(
            pdMS_TO_TICKS(
                BUTTON_CHECK_INTERVAL
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
    // Button
    //-------------------------------------------------------------------------

    pinMode(
        BUTTON_PIN,
        INPUT_PULLUP
    );

    //-------------------------------------------------------------------------
    // Buzzer PWM
    //-------------------------------------------------------------------------

    ledcSetup(
        BUZZER_CHANNEL,
        1000,
        BUZZER_RES
    );

    ledcAttachPin(
        BUZZER_PIN,
        BUZZER_CHANNEL
    );

    ledcWrite(
        BUZZER_CHANNEL,
        0
    );

    Serial.println(
        "Button Hold Pitch Buzzer Ready"
    );

    //-------------------------------------------------------------------------
    // Create FreeRTOS Task
    //-------------------------------------------------------------------------

    xTaskCreatePinnedToCore(
        buttonBuzzerTask,
        "Button Buzzer Task",
        BUTTON_TASK_STACK_SIZE,
        NULL,
        BUTTON_TASK_PRIORITY,
        NULL,
        1
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    vTaskDelay(
        pdMS_TO_TICKS(1000)
    );
}
