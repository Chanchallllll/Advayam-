/*
 * This program measures how long the push button on GPIO 17
 * is held and increases the buzzer frequency as the hold time
 * becomes longer.
 *
 * Releasing the button stops the buzzer.
 */

#include <Arduino.h>

#define BUTTON_PIN  17
#define BUZZER_PIN  16

#define BUZZER_CHANNEL  0
#define BUZZER_RES      8

unsigned long pressStart = 0;
bool buttonPressed = false;

void setup()
{
    Serial.begin(115200);

    // Configure button with pull-up
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // Configure buzzer PWM
    ledcSetup(BUZZER_CHANNEL, 1000, BUZZER_RES);
    ledcAttachPin(BUZZER_PIN, BUZZER_CHANNEL);

    ledcWrite(BUZZER_CHANNEL, 0);
}

void loop()
{
    bool pressed = digitalRead(BUTTON_PIN) == LOW;

    // Detect button press
    if (pressed && !buttonPressed)
    {
        buttonPressed = true;
        pressStart = millis();
    }

    // Increase pitch while held
    if (pressed)
    {
        unsigned long holdTime = millis() - pressStart;

        int frequency = 300 + (holdTime / 10);

        // Limit the maximum frequency
        frequency = constrain(frequency, 300, 4000);

        ledcWriteTone(BUZZER_CHANNEL, frequency);

        Serial.print("Hold Time: ");
        Serial.print(holdTime);
        Serial.print(" ms | Frequency: ");
        Serial.println(frequency);
    }

    // Detect button release
    if (!pressed && buttonPressed)
    {
        buttonPressed = false;

        // Stop buzzer
        ledcWrite(BUZZER_CHANNEL, 0);
    }

    delay(10);
}
