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
 * DEMO PROGRAM: Matrix Heart Pattern
 * Purpose: Renders a 5x5 heart icon bit-matrix pattern using red LEDs on the RGB matrix.
 * Active Pins Used In This Code:
 *   - GPIO7 (LED_PIN) : Digital Output / PWM -> WS2812B RGB Matrix Data
 * =====================================================================================
 */

#include <Arduino.h>
#include <FastLED.h>

#define LED_PIN        7
#define MATRIX_WIDTH   5
#define MATRIX_HEIGHT  5
#define NUM_LEDS       25
#define BRIGHTNESS     50
#define LED_TYPE       WS2812B
#define COLOR_ORDER    GRB

CRGB leds[NUM_LEDS];

int XY(int x, int y)
{
    y = 4 - y;
    if (y % 2 == 0)
        return y * MATRIX_WIDTH + x;
    else
        return y * MATRIX_WIDTH + (MATRIX_WIDTH - 1 - x);
}

void displayPattern(const byte pattern[5][5], CRGB color)
{
    FastLED.clear();

    for (int y = 0; y < MATRIX_HEIGHT; y++)
        for (int x = 0; x < MATRIX_WIDTH; x++)
            if (pattern[y][x])
                leds[XY(x, y)] = color;

    FastLED.show();
}

const byte heart[5][5] =
{
    {0,1,0,1,0},
    {1,1,1,1,1},
    {1,1,1,1,1},
    {0,1,1,1,0},
    {0,0,1,0,0}
};

void setup()
{
    FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS);
    FastLED.setBrightness(BRIGHTNESS);

    FastLED.clear();
    FastLED.show();
}

void loop()
{
    displayPattern(heart, CRGB::Red);
    delay(2000);

    FastLED.clear();
    FastLED.show();
    delay(500);
}