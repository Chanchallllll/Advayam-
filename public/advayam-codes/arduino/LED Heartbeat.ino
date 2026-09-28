/*
 * This program simulates a human heartbeat using the User LED
 * on GPIO 45.
 *
 * The LED performs two fading pulses followed by a short pause,
 * creating a repeating heartbeat-like pattern.
 */

#include <Arduino.h>

#define LED_PIN  45

#define LED_CHANNEL  0
#define PWM_FREQ     5000
#define PWM_RES      8

// Create a smooth LED pulse
void pulseLED(int maxBrightness, int duration)
{
    // Fade in
    for (int brightness = 0; brightness <= maxBrightness; brightness += 5)
    {
        ledcWrite(LED_CHANNEL, brightness);
        delay(duration / 50);
    }

    // Fade out
    for (int brightness = maxBrightness; brightness >= 0; brightness -= 5)
    {
        ledcWrite(LED_CHANNEL, brightness);
        delay(duration / 50);
    }
}

void setup()
{
    // Configure LED PWM
    ledcSetup(LED_CHANNEL, PWM_FREQ, PWM_RES);
    ledcAttachPin(LED_PIN, LED_CHANNEL);

    ledcWrite(LED_CHANNEL, 0);
}

void loop()
{
    // First heartbeat pulse
    pulseLED(255, 250);

    delay(80);

    // Second heartbeat pulse
    pulseLED(180, 180);

    // Pause between heartbeats
    delay(600);
}
