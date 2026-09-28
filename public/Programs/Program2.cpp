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
 * DEMO PROGRAM: Button Toggle LED (with Debounce)
 * Purpose: Toggles the user LED state (ON/OFF) on button press detection with software debounce.
 * Active Pins Used In This Code:
 *   - GPIO45 (USER_LED)   : Digital Output -> User LED
 *   - GPIO17 (BUTTON_PIN) : Digital Input (Pullup) -> Push Button
 * =====================================================================================
 */

#include <Arduino.h>

#define USER_LED       45
#define BUTTON_PIN     17

bool ledState = false;
bool lastButtonState = HIGH;

void setup()
{
    pinMode(USER_LED, OUTPUT);
    pinMode(BUTTON_PIN, INPUT_PULLUP);
    digitalWrite(USER_LED, LOW);
}

void loop()
{
    bool currentButtonState = digitalRead(BUTTON_PIN);

    if (lastButtonState == HIGH && currentButtonState == LOW)
    {
        ledState = !ledState;
        digitalWrite(USER_LED, ledState);
        delay(200);
    }

    lastButtonState = currentButtonState;
}