/*
 * =============================================================================
 * COLOUR TEMPERATURE SIMULATOR WITH AUTOMATIC BRIGHTNESS
 * =============================================================================
 *
 * ESP-IDF 6.0.2 / ESP32-S3
 *
 * Creates a colour-temperature effect on a 5x5 RGB LED matrix.
 *
 * TWO SENSORS:
 *
 * 1. LMT87 Temperature Sensor
 *    - OUT -> GPIO 1
 *    - Determines MATRIX COLOUR
 *
 * 2. RPR-0521RS Ambient Light / Proximity Sensor
 *    - SDA -> GPIO 5
 *    - SCL -> GPIO 6
 *    - Determines MATRIX BRIGHTNESS
 *
 * TEMPERATURE MAPPING:
 *
 *    Cold room -> Warm amber/orange
 *    Warm room -> Cool blue/white
 *
 * LIGHT MAPPING:
 *
 *    Dark room   -> Dim matrix
 *    Bright room -> Bright matrix
 *
 * RGB MATRIX:
 *
 *    Data -> GPIO 7
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/i2c_master.h"

#include "esp_log.h"
#include "esp_err.h"

#include "esp_adc/adc_oneshot.h"

#include "led_strip.h"

#include "rpr0521rs.h"


/* =============================================================================
 * GPIO Configuration
 * =============================================================================
 */

#define TEMP_SENSOR_PIN      1

#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000
#define I2C_PORT             I2C_NUM_0

#define MATRIX_PIN           7
#define NUM_PIXELS           25


/* =============================================================================
 * LMT87 Configuration
 * =============================================================================
 *
 * These values intentionally match the Arduino LMT87 code.
 *
 * Arduino:
 *
 *     ADCres = 10
 *     voltageRef = 3.3
 *     avgAmt = 100
 *
 * The original library converts:
 *
 *     ADC -> mV -> temperature
 *
 * using:
 *
 *     mV = ADC * 3300 / 1024
 *
 * followed by the LMT87 quadratic equation.
 *
 * =============================================================================
 */

#define LMT87_ADC_RESOLUTION     10
#define LMT87_VOLTAGE_REF_MV     3300.0
#define LMT87_AVERAGE_SAMPLES    100


/* =============================================================================
 * Temperature Mapping
 * =============================================================================
 */

#define MIN_TEMPERATURE  15.0
#define MAX_TEMPERATURE  35.0


/* =============================================================================
 * Ambient Light Mapping
 * =============================================================================
 */

#define MIN_LUX          5.0
#define MAX_LUX          500.0

#define MIN_BRIGHTNESS   10
#define MAX_BRIGHTNESS   180


/* =============================================================================
 * Globals
 * =============================================================================
 */

static const char *TAG = "COLOUR_TEMP";

static i2c_master_dev_handle_t sensor_dev = NULL;

static led_strip_handle_t led_strip = NULL;

static adc_oneshot_unit_handle_t adc1_handle = NULL;


/* =============================================================================
 * Utility: Clamp Float
 * =============================================================================
 */

static float clamp_float(float value, float min, float max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}


/* =============================================================================
 * Utility: Clamp Integer
 * =============================================================================
 */

static int clamp_int(int value, int min, int max)
{
    if (value < min) {
        return min;
    }

    if (value > max) {
        return max;
    }

    return value;
}


/* =============================================================================
 * I2C Initialization
 * =============================================================================
 */

static void i2c_master_init(void)
{
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,

        .flags = {
            .enable_internal_pullup = true,
        }
    };

    i2c_master_bus_handle_t bus_handle = NULL;

    ESP_ERROR_CHECK(
        i2c_new_master_bus(
            &bus_config,
            &bus_handle
        )
    );


    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };


    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(
            bus_handle,
            &dev_config,
            &sensor_dev
        )
    );


    ESP_LOGI(TAG, "I2C initialized");
}


/* =============================================================================
 * RPR-0521RS Initialization
 * =============================================================================
 */

static void rpr0521rs_init_sensor(void)
{
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {

        ESP_LOGE(
            TAG,
            "RPR-0521RS initialization failed!"
        );

        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    ESP_LOGI(
        TAG,
        "RPR-0521RS initialized successfully"
    );
}


/* =============================================================================
 * ADC Initialization
 * =============================================================================
 *
 * ESP32-S3:
 *
 *     GPIO 1 = ADC1 Channel 0
 *
 * =============================================================================
 */

static void adc_init(void)
{
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };


    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &adc1_handle
        )
    );


    adc_oneshot_chan_cfg_t channel_config = {
        .bitwidth = ADC_BITWIDTH_12,
        .atten = ADC_ATTEN_DB_12,
    };


    /*
     * GPIO 1 = ADC1_CHANNEL_0 on ESP32-S3.
     */
    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            ADC_CHANNEL_0,
            &channel_config
        )
    );


    ESP_LOGI(
        TAG,
        "LMT87 ADC initialized on GPIO %d",
        TEMP_SENSOR_PIN
    );
}


/* =============================================================================
 * Read LMT87 Temperature
 * =============================================================================
 *
 * This intentionally reproduces the conversion used by the Arduino
 * LMT87 library:
 *
 *     LMT87::getCDegree()
 *
 * Original algorithm:
 *
 *     1. Average 100 ADC readings.
 *     2. Convert ADC to mV using:
 *
 *            mV = ADC * 3300 / 1024
 *
 *     3. Calculate:
 *
 *            tempA = (-13.582)^2
 *
 *            tempB = 2230.8 - mV
 *
 *            tempC = 4 * 0.00433 * tempB
 *
 *            tempD = tempA + tempC
 *
 *            tempE = sqrt(tempD)
 *
 *            tempA = 13.582 - tempE
 *
 *            tempB = 2 * (-0.00433)
 *
 *            tempC = tempA / tempB
 *
 *            temperature = tempC + 30
 *
 * The Arduino code then applies a minus sign:
 *
 *     temperatureC = -lmt87.getCDegree(...)
 *
 * Therefore we preserve that behavior here.
 *
 * =============================================================================
 */

static float read_temperature_c(void)
{
    int raw = 0;

    int64_t adc_sum = 0;


    /*
     * Take the same number of samples as the Arduino library.
     */
    for (int i = 0; i < LMT87_AVERAGE_SAMPLES; i++) {

        esp_err_t ret =
            adc_oneshot_read(
                adc1_handle,
                ADC_CHANNEL_0,
                &raw
            );

        if (ret != ESP_OK) {

            ESP_LOGE(
                TAG,
                "LMT87 ADC read failed: %s",
                esp_err_to_name(ret)
            );

            return 0.0f;
        }


        /*
         * ESP-IDF ADC is configured for 12-bit raw readings.
         *
         * Convert the 12-bit reading to the equivalent 10-bit
         * Arduino-style reading.
         *
         * 12-bit range: 0...4095
         * 10-bit range: 0...1023
         */
        int raw_10bit =
            (raw * 1023) / 4095;

        adc_sum += raw_10bit;


        /*
         * Small delay between samples.
         */
        esp_rom_delay_us(100);
    }


    /*
     * Average exactly like the Arduino library.
     */
    double adc_average =
        (double)adc_sum / LMT87_AVERAGE_SAMPLES;


    /*
     * Convert ADC reading to millivolts.
     *
     * This intentionally uses 1024 rather than 1023 because
     * that is what the original LMT87 library does:
     *
     *     steps = pow(2.0, ADCres)
     *           = 1024
     */
    double mVTemp =
        adc_average *
        LMT87_VOLTAGE_REF_MV /
        1024.0;


    /*
     * -------------------------------------------------------------------------
     * LMT87 temperature equation
     * -------------------------------------------------------------------------
     */

    double tempA =
        (-13.582) * (-13.582);

    double tempB =
        2230.8 - mVTemp;

    double tempC =
        4.0 * 0.00433 * tempB;

    double tempD =
        tempA + tempC;


    /*
     * Guard against sqrt() of a negative value caused by
     * an invalid ADC reading.
     */
    if (tempD < 0.0) {

        ESP_LOGE(
            TAG,
            "Invalid LMT87 ADC conversion: %.2f mV",
            mVTemp
        );

        return 0.0f;
    }


    double tempE =
        sqrt(tempD);


    tempA =
        13.582 - tempE;

    tempB =
        2.0 * (-0.00433);

    tempC =
        tempA / tempB;

    double temperatureC =
        tempC + 30.0;


    /*
     * IMPORTANT:
     *
     * Your Arduino code intentionally applies a negative sign:
     *
     *     -lmt87.getCDegree(...)
     *
     */
    temperatureC = -temperatureC;


    return (float)temperatureC;
}


/* =============================================================================
 * Initialize LED Matrix
 * =============================================================================
 */

static void led_strip_init_board(void)
{
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_PIXELS,

        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,

        .flags = {
            .invert_out = false,
        }
    };


    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,

        /*
         * 10 MHz RMT resolution.
         */
        .resolution_hz = 10 * 1000 * 1000,

        .flags = {
            .with_dma = false,
        }
    };


    ESP_ERROR_CHECK(
        led_strip_new_rmt_device(
            &strip_config,
            &rmt_config,
            &led_strip
        )
    );


    ESP_ERROR_CHECK(
        led_strip_clear(led_strip)
    );


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    ESP_LOGI(
        TAG,
        "RGB matrix initialized"
    );
}


/* =============================================================================
 * Update Matrix
 * =============================================================================
 */

static void update_matrix(float temperatureC, float lux)
{
    /* -------------------------------------------------------------------------
     * Temperature Colour
     * -------------------------------------------------------------------------
     *
     * Low temperature:
     *     RED high
     *     BLUE low
     *
     * High temperature:
     *     RED low
     *     BLUE high
     */

    float temperatureRatio =
        (temperatureC - MIN_TEMPERATURE) /
        (MAX_TEMPERATURE - MIN_TEMPERATURE);


    temperatureRatio =
        clamp_float(
            temperatureRatio,
            0.0f,
            1.0f
        );


    /*
     * Same formulas as the Arduino program.
     */

    int red =
        255 -
        (int)(200.0f * temperatureRatio);


    int green =
        100 +
        (int)(100.0f * (1.0f - temperatureRatio));


    int blue =
        30 +
        (int)(225.0f * temperatureRatio);


    red =
        clamp_int(
            red,
            0,
            255
        );

    green =
        clamp_int(
            green,
            0,
            255
        );

    blue =
        clamp_int(
            blue,
            0,
            255
        );


    /* -------------------------------------------------------------------------
     * Ambient Light -> Brightness
     * -------------------------------------------------------------------------
     */

    float luxRatio =
        (lux - MIN_LUX) /
        (MAX_LUX - MIN_LUX);


    luxRatio =
        clamp_float(
            luxRatio,
            0.0f,
            1.0f
        );


    int brightness =
        MIN_BRIGHTNESS +
        (int)(
            (MAX_BRIGHTNESS - MIN_BRIGHTNESS) *
            luxRatio
        );


    brightness =
        clamp_int(
            brightness,
            MIN_BRIGHTNESS,
            MAX_BRIGHTNESS
        );


    /*
     * -------------------------------------------------------------------------
     * Apply brightness
     * -------------------------------------------------------------------------
     *
     * ESP-IDF's led_strip component doesn't have the same global
     * setBrightness() function as Adafruit_NeoPixel.
     *
     * Therefore we apply the brightness mathematically to the RGB
     * values before sending them to the LEDs.
     */

    uint8_t scaled_red =
        (uint8_t)(
            ((int)red * brightness) /
            255
        );

    uint8_t scaled_green =
        (uint8_t)(
            ((int)green * brightness) /
            255
        );

    uint8_t scaled_blue =
        (uint8_t)(
            ((int)blue * brightness) /
            255
        );


    /*
     * -------------------------------------------------------------------------
     * Update all 25 pixels
     * -------------------------------------------------------------------------
     */

    for (int i = 0; i < NUM_PIXELS; i++) {

        ESP_ERROR_CHECK(
            led_strip_set_pixel(
                led_strip,
                i,
                scaled_red,
                scaled_green,
                scaled_blue
            )
        );
    }


    ESP_ERROR_CHECK(
        led_strip_refresh(led_strip)
    );


    /*
     * Debug information.
     */
    ESP_LOGI(
        TAG,
        "Temp: %.1f C | Lux: %.2f | RGB: %d,%d,%d | Brightness: %d",
        (double)temperatureC,
        (double)lux,
        scaled_red,
        scaled_green,
        scaled_blue,
        brightness
    );
}


/* =============================================================================
 * Main Application
 * =============================================================================
 */

void app_main(void)
{
    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "==============================================");
    ESP_LOGI(TAG, " Colour Temperature + Auto Brightness");
    ESP_LOGI(TAG, "==============================================");


    /* -------------------------------------------------------------------------
     * Initialize ADC / LMT87
     * -------------------------------------------------------------------------
     */

    adc_init();


    /* -------------------------------------------------------------------------
     * Initialize I2C
     * -------------------------------------------------------------------------
     */

    i2c_master_init();


    /* -------------------------------------------------------------------------
     * Initialize RPR-0521RS
     * -------------------------------------------------------------------------
     */

    rpr0521rs_init_sensor();


    /* -------------------------------------------------------------------------
     * Initialize RGB matrix
     * -------------------------------------------------------------------------
     */

    led_strip_init_board();


    ESP_LOGI(TAG, "");
    ESP_LOGI(
        TAG,
        "Temperature controls COLOUR."
    );

    ESP_LOGI(
        TAG,
        "Ambient light controls BRIGHTNESS."
    );

    ESP_LOGI(TAG, "");

    ESP_LOGI(
        TAG,
        "Temperature mapping:"
    );

    ESP_LOGI(
        TAG,
        "  Cold -> Warm Amber"
    );

    ESP_LOGI(
        TAG,
        "  Hot  -> Cool Blue"
    );

    ESP_LOGI(TAG, "");


    /* -------------------------------------------------------------------------
     * Main loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        /* ---------------------------------------------------------------------
         * Read LMT87
         * ---------------------------------------------------------------------
         */

        float temperatureC =
            read_temperature_c();


        /* ---------------------------------------------------------------------
         * Read RPR-0521RS
         * ---------------------------------------------------------------------
         */

        uint16_t proximity_raw = 0;

        float lux = 0.0f;


        esp_err_t ret =
            rpr0521rs_get_psalsval(
                sensor_dev,
                &proximity_raw,
                &lux
            );


        /* ---------------------------------------------------------------------
         * Check sensor reading
         * ---------------------------------------------------------------------
         */

        if (ret == ESP_OK) {

            /*
             * Both sensor readings are valid.
             */
            update_matrix(
                temperatureC,
                lux
            );


            ESP_LOGI(
                TAG,
                "Temperature : %.1f °C",
                (double)temperatureC
            );

            ESP_LOGI(
                TAG,
                "Ambient Lux : %.2f lx",
                (double)lux
            );

            ESP_LOGI(
                TAG,
                "Proximity   : %u",
                proximity_raw
            );

            ESP_LOGI(TAG, "");


        } else {

            /*
             * Do not update the matrix if the ambient
             * light sensor failed.
             */
            ESP_LOGE(
                TAG,
                "Failed to read RPR-0521RS: %s",
                esp_err_to_name(ret)
            );
        }


        /*
         * Take a new measurement every 500 ms.
         */
        vTaskDelay(
            pdMS_TO_TICKS(500)
        );
    }
}
