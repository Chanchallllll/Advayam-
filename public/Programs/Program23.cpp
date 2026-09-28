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
 * DEMO PROGRAM: LSM6DSOX SPI Readout
 * Purpose: Interfaces with the 6-DOF LSM6DSOX accelerometer & gyroscope using SPI communication protocol.
 * Active Pins Used In This Code:
 *   - GPIO10 (IMU_CS)   : SPI Chip Select (CS)
 *   - GPIO12 (IMU_SCK)  : SPI Serial Clock (SCK)
 *   - GPIO11 (IMU_MOSI) : SPI Master-Out-Slave-In (MOSI)
 *   - GPIO13 (IMU_MISO) : SPI Master-In-Slave-Out (MISO)
 *   - GPIO38 (IMU_INT)  : Digital Input -> Hardware Interrupt Pin
 * =====================================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>

#define IMU_CS      10
#define IMU_SCK     12
#define IMU_MOSI    11
#define IMU_MISO    13
#define IMU_INT     38

Adafruit_LSM6DSOX imu;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println();
    Serial.println("LSM6DSOX Accelerometer Test");

    SPI.begin(IMU_SCK, IMU_MISO, IMU_MOSI, IMU_CS);

    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println("Failed to initialize LSM6DSOX!");
        while (1)
        {
            delay(100);
        }
    }

    Serial.println("LSM6DSOX Initialized");

    imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);
}

void loop()
{
    sensors_event_t accel;
    sensors_event_t gyro;
    sensors_event_t temp;

    imu.getEvent(&accel, &gyro, &temp);

    Serial.print("Accel X : ");
    Serial.print(accel.acceleration.x, 2);
    Serial.print(" m/s²\t");

    Serial.print("Y : ");
    Serial.print(accel.acceleration.y, 2);
    Serial.print(" m/s²\t");

    Serial.print("Z : ");
    Serial.print(accel.acceleration.z, 2);
    Serial.println(" m/s²");

    delay(500);
}