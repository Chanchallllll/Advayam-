/*
 * =============================================================================
 * REACTION TIMER GAME - FREERTOS VERSION
 * =============================================================================
 *
 * The game waits for the player to press and release the button.
 *
 * It then waits for a random amount of time before sounding the buzzer.
 *
 * Once the buzzer sounds, the player must press the button as quickly as
 * possible.
 *
 * The reaction time is measured in milliseconds and displayed through UART.
 *
 * Button:
 *   GPIO 17
 *
 * Buzzer:
 *   GPIO 16
 *
 * The game logic runs as a FreeRTOS task. Button waiting uses vTaskDelay()
 * instead of a continuously spinning while loop so the CPU is yielded while
 * waiting.
 * =============================================================================
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#define BUTTON_PIN 17
#define BUZZER_PIN 16

//=============================================================================
// Wait for Button Release
//=============================================================================

void waitForRelease()
{
    while (digitalRead(BUTTON_PIN) == LOW)
    {
        // Yield the task while waiting for release.
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

//=============================================================================
// Reaction Timer Task
//=============================================================================

void reactionTimerTask(void *parameter)
{
    while (true)
    {
        //=====================================================================
        // Wait for the player to start the game.
        //=====================================================================

        while (digitalRead(BUTTON_PIN) == HIGH)
        {
            vTaskDelay(pdMS_TO_TICKS(10));
        }

        // Make sure the starting button press has been released.
        waitForRelease();

        Serial.println();
        Serial.println("Get ready...");

        //=====================================================================
        // Random waiting period
        //=====================================================================

        int randomDelay =
            random(1000, 4000);

        vTaskDelay(
            pdMS_TO_TICKS(randomDelay)
        );

        Serial.println("GO!");

        //=====================================================================
        // Start the reaction timer.
        //=====================================================================

        tone(BUZZER_PIN, 1000);

        unsigned long startTime = millis();

        // Wait for the player's response.
        // The task sleeps briefly between button checks.
        while (digitalRead(BUTTON_PIN) == HIGH)
        {
            vTaskDelay(pdMS_TO_TICKS(1));
        }

        // Calculate the reaction time.
        unsigned long reactionTime =
            millis() - startTime;

        // Stop the buzzer.
        noTone(BUZZER_PIN);

        //=====================================================================
        // Display result
        //=====================================================================

        Serial.print("Reaction time: ");
        Serial.print(reactionTime);
        Serial.println(" ms");

        // Give the player a short pause before another round.
        vTaskDelay(pdMS_TO_TICKS(1000));

        Serial.println();
        Serial.println("Press button to play again.");
    }
}

//=============================================================================
// Setup
//=============================================================================

void setup()
{
    Serial.begin(115200);

    pinMode(BUTTON_PIN, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);

    digitalWrite(BUZZER_PIN, LOW);

    // Seed the pseudo-random number generator.
    randomSeed(
        (uint32_t)esp_random()
    );

    Serial.println();
    Serial.println("================================");
    Serial.println("       REACTION TIMER");
    Serial.println("       FreeRTOS Version");
    Serial.println("================================");
    Serial.println("Press the button to start.");

    // Start the game task.
    xTaskCreate(
        reactionTimerTask,
        "ReactionTimer",
        4096,
        NULL,
        1,
        NULL
    );
}

//=============================================================================
// Arduino Loop
//=============================================================================

void loop()
{
    // The game is completely controlled by the FreeRTOS task.
    vTaskDelay(portMAX_DELAY);
}
