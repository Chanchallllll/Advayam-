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
 * DEMO PROGRAM: Temperature Readout Formatted
 * Purpose: Reads temperature in °C and °F using ADC reference voltage and prints formatted string outputs.
 * Active Pins Used In This Code:
 *   - GPIO1 (TEMP_SENSOR_PIN) : Analog Input (ADC1_CH0) -> Onboard Temperature Sensor
 *   - GPIO1 (TX) & GPIO3 (RX) : Hardware Serial UART Output
 * =====================================================================================
 */

#include <Arduino.h>

#define TEMP_SENSOR_PIN    1
#define ADC_REFERENCE      3.3
#define ADC_RESOLUTION     4095.0

void setup()
{
    Serial.begin(9600);
    analogReadResolution(12);

    Serial.println("--------------------------------------");
    Serial.println("USB Serial Temperature Monitor");
    Serial.println("--------------------------------------");
}

void loop()
{
    int adcValue = analogRead(TEMP_SENSOR_PIN);
    float voltage = (adcValue * ADC_REFERENCE) / ADC_RESOLUTION;
    float temperatureC = voltage * 100.0;
    float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;

    Serial.print("Temperature : ");
    Serial.print(temperatureC, 1);
    Serial.print(" °C | ");
    Serial.print(temperatureF, 1);
    Serial.println(" °F");

    delay(1000);
}