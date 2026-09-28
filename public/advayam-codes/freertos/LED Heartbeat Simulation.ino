/*
 * =============================================================================
 * HEARTBEAT LED SIMULATION
 * =============================================================================
 *
 * This program simulates a human heartbeat using the User LED
 * on GPIO 45.
 *
 * The LED performs two fading pulses followed by a short pause,
 * creating a repeating heartbeat-like pattern.
 *
 * The heartbeat pattern runs inside a dedicated FreeRTOS task.
 *
 * Hardware:
 *   User LED -> GPIO 45
 *
 * =============================================================================
 */

#include <Arduino.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// =============================================================================
// LED configuration
// =============================================================================

#define LED_PIN 45

#define LED_CHANNEL 0
#define PWM_FREQ 5000
#define PWM_RES 8

// =============================================================================
// Create one smooth LED pulse
// =============================================================================

void pulseLED(
  int maxBrightness,
  int duration
)
{
  // ---------------------------------------------------------------------------
  // Fade in
  // ---------------------------------------------------------------------------

  for (
    int brightness = 0;
    brightness <= maxBrightness;
    brightness += 5
  )
  {
    ledcWrite(
      LED_CHANNEL,
      brightness
    );

    // Yield to FreeRTOS instead of blocking with delay().
    vTaskDelay(
      pdMS_TO_TICKS(duration / 50)
    );
  }

  // ---------------------------------------------------------------------------
  // Fade out
  // ---------------------------------------------------------------------------

  for (
    int brightness = maxBrightness;
    brightness >= 0;
    brightness -= 5
  )
  {
    ledcWrite(
      LED_CHANNEL,
      brightness
    );

    vTaskDelay(
      pdMS_TO_TICKS(duration / 50)
    );
  }
}

// =============================================================================
// Heartbeat FreeRTOS task
// =============================================================================

void heartbeatTask(void *parameter)
{
  while (true)
  {
    // First heartbeat pulse.
    pulseLED(
      255,
      250
    );

    // Short pause between the two pulses.
    vTaskDelay(
      pdMS_TO_TICKS(80)
    );

    // Second heartbeat pulse.
    pulseLED(
      180,
      180
    );

    // Pause before the next heartbeat.
    vTaskDelay(
      pdMS_TO_TICKS(600)
    );
  }
}

// =============================================================================
// Setup
// =============================================================================

void setup()
{
  // ---------------------------------------------------------------------------
  // Configure LEDC PWM
  // ---------------------------------------------------------------------------
  //
  // This uses the older Arduino-ESP32 LEDC API:
  //
  //   ledcSetup()
  //   ledcAttachPin()
  //   ledcWrite()
  //

  ledcSetup(
    LED_CHANNEL,
    PWM_FREQ,
    PWM_RES
  );

  ledcAttachPin(
    LED_PIN,
    LED_CHANNEL
  );

  // Start with the LED switched off.
  ledcWrite(
    LED_CHANNEL,
    0
  );

  // ---------------------------------------------------------------------------
  // Create heartbeat task
  // ---------------------------------------------------------------------------

  xTaskCreatePinnedToCore(
    heartbeatTask,
    "Heartbeat Task",
    2048,
    NULL,
    1,
    NULL,
    1
  );
}

// =============================================================================
// Arduino loop
// =============================================================================

void loop()
{
  // The heartbeat is handled by heartbeatTask().
  vTaskDelay(
    pdMS_TO_TICKS(1000)
  );
}
