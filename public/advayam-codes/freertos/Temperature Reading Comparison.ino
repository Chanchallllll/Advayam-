/*
 * =============================================================================
 * IMU VS LMT87 TEMPERATURE - FREERTOS VERSION
 * =============================================================================
 *
 * This program compares the LSM6DSOX internal temperature sensor with the
 * external LMT87 analog temperature sensor.
 *
 * FreeRTOS structure:
 *
 *   TemperatureTask
 *       - Reads both temperature sensors
 *       - Prints both values over UART
 *
 * The task executes once every second.
 * =============================================================================
 */

#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_LSM6DSOX.h>
#include <LMT87.h>

//====================================================
// GPIO Configuration
//====================================================
#define LMT_PIN 1

#define IMU_CS   10
#define IMU_SCK  12
#define IMU_MOSI 11
#define IMU_MISO 13

//====================================================
// LMT87 Configuration
//====================================================
int ADCres = 10;
double voltageRef = 3.3;

LMT87 lmt87 = LMT87();

//====================================================
// IMU
//====================================================
Adafruit_LSM6DSOX imu;

//====================================================
// Temperature Task
//====================================================
void TemperatureTask(void *parameter)
{
    while (true)
    {
        sensors_event_t accel;
        sensors_event_t gyro;
        sensors_event_t temp;

        // Read the LSM6DSOX internal temperature.
        imu.getEvent(&accel, &gyro, &temp);

        // Read the external LMT87.
        // The minus sign is intentional for this board.
        double analogTemp =
            -lmt87.getCDegree(
                LMT_PIN,
                ADCres,
                voltageRef,
                100
            );

        //================================================
        // Display Results
        //================================================
        Serial.print("IMU: ");
        Serial.print(temp.temperature, 1);
        Serial.print(" C");

        Serial.print(" | LMT87: ");
        Serial.print(analogTemp, 1);
        Serial.println(" C");

        // Take another measurement after 1 second.
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

//====================================================
// Setup
//====================================================
void setup()
{
    Serial.begin(115200);

    pinMode(16, OUTPUT);
    digitalWrite(16, LOW);

    // Initialize the SPI bus used by the IMU.
    SPI.begin(
        IMU_SCK,
        IMU_MISO,
        IMU_MOSI,
        IMU_CS
    );

    // Initialize the LSM6DSOX.
    if (!imu.begin_SPI(IMU_CS, &SPI))
    {
        Serial.println("IMU initialization failed!");

        // Stop this Arduino task if initialization fails.
        while (true)
        {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    imu.setAccelRange(LSM6DS_ACCEL_RANGE_4_G);
    imu.setAccelDataRate(LSM6DS_RATE_104_HZ);

    Serial.println("IMU vs LMT87 Temperature");
    Serial.println("FreeRTOS Version");

    // Create temperature monitoring task.
    xTaskCreate(
        TemperatureTask,
        "Temperature Task",
        4096,
        NULL,
        2,
        NULL
    );
}

//====================================================
// Loop
//====================================================
void loop()
{
    vTaskDelay(pdMS_TO_TICKS(1000));
}
