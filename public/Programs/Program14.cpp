/*
 * =====================================================================================
 * BOARD PIN DEFINITION & FUNCTION DOCUMENTATION
 * =====================================================================================
 * Pin Type & Protocol Definitions:
 *   - UART    : Universal Asynchronous Receiver-Transmitter (TX/RX serial lines for communication).
 *   - SPI     : Serial Peripheral Interface (SCK, MOSI, MISO, CS for high-speed sensor/display buses).
 *   - I²C     : Inter-Integrated Circuit (SDA, SCL lines for multi-device peripheral buses).
 *   - Digital : General Purpose Input/Output (GPIO) for binary state reading/control (HIGH/LOW).
 *   - PWM     : Pulse Width Modulation for precise analog-like signal output (brightness, tone, speed).
 *   - ADC     : Analog-to-Digital Converter for reading continuous analog sensor voltages.
 *   - Touch   : Capacitive touch input sensing capability.
 * 
 * Supported Development Board Pin Mapping / Reference (ESP32 / Advayam Core Board):
 * -------------------------------------------------------------------------------------
 * Pin        UART    SPI     I²C     Digital PWM     ADC/Touch  Function / Hardware Mapping
 * -------------------------------------------------------------------------------------
 * GPIO1      TX      —       —       ✓       ✓       ADC        Analog Temp Sensor / Serial TX
 * GPIO3      RX      —       —       ✓       ✓       —          Serial RX
 * GPIO7      —       —       —       ✓       ✓       —          5x5 RGB Matrix Data (WS2812B)
 * GPIO8      —       —       —       ✓       —       Touch      Capacitive Touch Sensor Input
 * GPIO10     —       CS      —       ✓       ✓       —          SPI Chip Select (LSM6DSOX IMU CS)
 * GPIO11     —       MOSI    —       ✓       ✓       —          SPI MOSI (LSM6DSOX IMU Data In)
 * GPIO12     —       SCK     —       ✓       ✓       —          SPI Clock (LSM6DSOX IMU Clock)
 * GPIO13     —       MISO    —       ✓       ✓       —          SPI MISO (LSM6DSOX IMU Data Out)
 * GPIO16     —       —       —       ✓       ✓       —          Piezo Buzzer Output
 * GPIO17     —       —       —       ✓       —       —          User Push Button Input (Pullup)
 * GPIO18     —       SCK     —       ✓       ✓       —          SPI SCK Default Pin
 * GPIO21     —       —       SDA     ✓       —       —          I²C Data Pin (SDA)
 * GPIO22     —       —       SCL     ✓       —       —          I²C Clock Pin (SCL)
 * GPIO23     —       MOSI    —       ✓       ✓       —          SPI MOSI Default Pin
 * GPIO38     —       —       —       ✓       —       —          LSM6DSOX IMU Interrupt Pin
 * GPIO45     —       —       —       ✓       ✓       —          User Onboard LED Indicator
 * -------------------------------------------------------------------------------------
 * 
 * DEMO PROGRAM: Touch Cycle Colors
 * Purpose: Cycles through solid matrix colors upon each capacitive touch sensor detection.
 * Active Pins Used In This Code:
 *   - GPIO7 (LED_PIN)   : Digital Output / PWM -> WS2812B RGB Matrix Data
 *   - GPIO8 (TOUCH_PIN) : Digital Input (Touch) -> Capacitive Touch Sensor
 * =====================================================================================
 */

#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN        7
#define TOUCH_PIN      8
#define NUM_LEDS       25
#define BRIGHTNESS     50
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB

CRGB leds[NUM_LEDS];

CRGB colors[] =
{
    CRGB::Red, CRGB::Green, CRGB::Blue, CRGB::Yellow,
    CRGB::Purple, CRGB::Cyan, CRGB::White
};

const int totalColors = sizeof(colors) / sizeof(colors[0]);
int colorIndex = 0;
bool lastTouchState = HIGH;

void setup()
{
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    pinMode(TOUCH_PIN, INPUT_PULLUP);

    fill_solid(leds, NUM_LEDS, colors[colorIndex]);
    FastLED.show();
}

void loop()
{
    bool touchState = digitalRead(TOUCH_PIN);

    if (lastTouchState == HIGH && touchState == LOW)
    {
        colorIndex++;

        if (colorIndex >= totalColors)
            colorIndex = 0;

        fill_solid(leds, NUM_LEDS, colors[colorIndex]);
        FastLED.show();

        delay(200);
    }

    lastTouchState = touchState;
}