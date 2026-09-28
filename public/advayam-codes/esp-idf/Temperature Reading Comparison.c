/*
 * =============================================================================
 * ESP-IDF v5+
 *
 * LSM6DSOX internal temperature vs LMT87 analog temperature
 *
 * LSM6DSOX:
 *   CS   -> GPIO 10
 *   SCK  -> GPIO 12
 *   MOSI -> GPIO 11
 *   MISO -> GPIO 13
 *
 * LMT87:
 *   OUT  -> GPIO 1 / ADC1 Channel 0
 *
 * GPIO 16:
 *   Held LOW, matching the original Arduino program.
 *
 * UART:
 *   115200 baud
 *
 * =============================================================================
 */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"

#include "esp_log.h"


/*
 * =============================================================================
 * Configuration
 * =============================================================================
 */

/* ---------------------------------------------------------------------------
 * LSM6DSOX SPI
 * ------------------------------------------------------------------------- */

#define IMU_CS              GPIO_NUM_10
#define IMU_SCK             GPIO_NUM_12
#define IMU_MOSI            GPIO_NUM_11
#define IMU_MISO            GPIO_NUM_13

#define IMU_SPI_HOST        SPI2_HOST


/* ---------------------------------------------------------------------------
 * Board control pin
 * ------------------------------------------------------------------------- */

#define BOARD_CONTROL_PIN   GPIO_NUM_16


/* ---------------------------------------------------------------------------
 * LMT87
 * ------------------------------------------------------------------------- */

#define LMT_PIN             GPIO_NUM_1
#define LMT_ADC_CHANNEL     ADC_CHANNEL_0

#define ADC_ATTENUATION     ADC_ATTEN_DB_12
#define ADC_BITWIDTH        ADC_BITWIDTH_DEFAULT


static const char *TAG = "TEMP_COMPARE";


/*
 * =============================================================================
 * LSM6DSOX Registers
 * =============================================================================
 */

#define LSM6DSOX_WHO_AM_I       0x0F

#define LSM6DSOX_CTRL1_XL       0x10
#define LSM6DSOX_CTRL2_G        0x11
#define LSM6DSOX_CTRL3_C        0x12

#define LSM6DSOX_OUT_TEMP_L     0x20
#define LSM6DSOX_OUT_TEMP_H     0x21

#define LSM6DSOX_WHO_AM_I_VALUE 0x6C


/*
 * =============================================================================
 * Global Handles
 * =============================================================================
 */

static spi_device_handle_t imu_spi = NULL;

static adc_oneshot_unit_handle_t adc1_handle = NULL;

static adc_cali_handle_t adc_cali_handle = NULL;

static bool adc_calibrated = false;


/*
 * =============================================================================
 * ADC Calibration
 * =============================================================================
 */

static bool adc_calibration_init(
    adc_unit_t unit,
    adc_channel_t channel,
    adc_cali_handle_t *out_handle
)
{
    adc_cali_handle_t handle = NULL;

    esp_err_t ret = ESP_FAIL;

    bool calibrated = false;


#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED

    ESP_LOGI(
        TAG,
        "Trying ADC curve fitting calibration..."
    );

    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = unit,
        .chan = channel,
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BITWIDTH
    };

    ret =
        adc_cali_create_scheme_curve_fitting(
            &cali_config,
            &handle
        );

    if (ret == ESP_OK) {

        calibrated = true;

        ESP_LOGI(
            TAG,
            "ADC curve fitting calibration enabled"
        );
    }

#endif


#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED

    if (!calibrated) {

        ESP_LOGI(
            TAG,
            "Trying ADC line fitting calibration..."
        );

        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTENUATION,
            .bitwidth = ADC_BITWIDTH
        };

        ret =
            adc_cali_create_scheme_line_fitting(
                &cali_config,
                &handle
            );

        if (ret == ESP_OK) {

            calibrated = true;

            ESP_LOGI(
                TAG,
                "ADC line fitting calibration enabled"
            );
        }
    }

#endif


    if (!calibrated) {

        ESP_LOGW(
            TAG,
            "ADC calibration unavailable"
        );
    }


    *out_handle = handle;

    return calibrated;
}


/*
 * =============================================================================
 * LMT87 ADC Initialization
 * =============================================================================
 */

static void lmt87_adc_init(void)
{
    /*
     * Create ADC1.
     */
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = ADC_UNIT_1
    };


    ESP_ERROR_CHECK(
        adc_oneshot_new_unit(
            &init_config,
            &adc1_handle
        )
    );


    /*
     * GPIO 1 / ADC1 Channel 0.
     */
    adc_oneshot_chan_cfg_t channel_config = {
        .atten = ADC_ATTENUATION,
        .bitwidth = ADC_BITWIDTH
    };


    ESP_ERROR_CHECK(
        adc_oneshot_config_channel(
            adc1_handle,
            LMT_ADC_CHANNEL,
            &channel_config
        )
    );


    /*
     * Initialize ADC calibration.
     *
     * The calibration is not used in the LMT87 temperature
     * calculation because your known-working implementation
     * uses the raw ADC value.
     */
    adc_calibrated =
        adc_calibration_init(
            ADC_UNIT_1,
            LMT_ADC_CHANNEL,
            &adc_cali_handle
        );
}


/*
 * =============================================================================
 * Read LMT87 Temperature
 * =============================================================================
 *
 * Known-working conversion for your board:
 *
 *     temperatureC = adc_raw * 0.03151
 *
 * This intentionally does NOT use ADC calibration.
 * =============================================================================
 */

static double read_lmt87_temperature(void)
{
    int adc_raw = 0;


    ESP_ERROR_CHECK(
        adc_oneshot_read(
            adc1_handle,
            LMT_ADC_CHANNEL,
            &adc_raw
        )
    );


    double temperatureC =
        adc_raw * 0.03151;


    return temperatureC;
}


/*
 * =============================================================================
 * LSM6DSOX SPI WRITE
 * =============================================================================
 *
 * Write:
 *
 *     bit 7 = 0
 *
 * Transaction:
 *
 *     [REGISTER] [DATA]
 *
 * =============================================================================
 */

static esp_err_t lsm6dsox_write_register(
    uint8_t reg,
    uint8_t value
)
{
    uint8_t tx[2];


    /*
     * Clear bit 7 for write.
     */
    tx[0] =
        reg & 0x7F;


    tx[1] =
        value;


    spi_transaction_t transaction = {
        .length = 16,

        .tx_buffer = tx,

        .rx_buffer = NULL
    };


    return spi_device_transmit(
        imu_spi,
        &transaction
    );
}


/*
 * =============================================================================
 * LSM6DSOX SPI READ
 * =============================================================================
 *
 * Read:
 *
 *     bit 7 = 1
 *
 * Transaction:
 *
 *     TX: [REGISTER | 0x80] [DUMMY]
 *     RX: [IGNORED]         [DATA]
 *
 * =============================================================================
 */

static esp_err_t lsm6dsox_read_register(
    uint8_t reg,
    uint8_t *value
)
{
    if (value == NULL) {
        return ESP_ERR_INVALID_ARG;
    }


    uint8_t tx[2] = {
        (uint8_t)(reg | 0x80),
        0x00
    };


    uint8_t rx[2] = {
        0x00,
        0x00
    };


    spi_transaction_t transaction = {
        .length = 16,

        .tx_buffer = tx,

        .rx_buffer = rx
    };


    esp_err_t ret =
        spi_device_transmit(
            imu_spi,
            &transaction
        );


    if (ret != ESP_OK) {
        return ret;
    }


    /*
     * rx[0] is the data received during the address byte.
     *
     * rx[1] is the actual register value.
     */
    *value =
        rx[1];


    return ESP_OK;
}


/*
 * =============================================================================
 * LSM6DSOX INITIALIZATION
 * =============================================================================
 */

static void lsm6dsox_init(void)
{
    /*
     * -------------------------------------------------------------------------
     * SPI BUS
     * -------------------------------------------------------------------------
     */

    spi_bus_config_t bus_config = {
        .mosi_io_num = IMU_MOSI,

        .miso_io_num = IMU_MISO,

        .sclk_io_num = IMU_SCK,

        .quadwp_io_num = -1,

        .quadhd_io_num = -1,

        .max_transfer_sz = 8
    };


    ESP_ERROR_CHECK(
        spi_bus_initialize(
            IMU_SPI_HOST,
            &bus_config,
            SPI_DMA_DISABLED
        )
    );


    /*
     * -------------------------------------------------------------------------
     * SPI DEVICE
     * -------------------------------------------------------------------------
     *
     * Start at 100 kHz while debugging.
     *
     * LSM6DSOX supports 4-wire SPI.
     */
    spi_device_interface_config_t device_config = {
        .clock_speed_hz = 100000,

        .mode = 3,

        .spics_io_num = IMU_CS,

        .queue_size = 1
    };


    ESP_ERROR_CHECK(
        spi_bus_add_device(
            IMU_SPI_HOST,
            &device_config,
            &imu_spi
        )
    );


    /*
     * Give the IMU time to settle after power-up.
     */
    vTaskDelay(
        pdMS_TO_TICKS(20)
    );


    /*
     * -------------------------------------------------------------------------
     * WHO_AM_I
     * -------------------------------------------------------------------------
     */

    uint8_t who_am_i = 0;


    ESP_ERROR_CHECK(
        lsm6dsox_read_register(
            LSM6DSOX_WHO_AM_I,
            &who_am_i
        )
    );


    ESP_LOGI(
        TAG,
        "LSM6DSOX WHO_AM_I = 0x%02X",
        who_am_i
    );


    if (who_am_i != LSM6DSOX_WHO_AM_I_VALUE) {

        ESP_LOGE(
            TAG,
            "LSM6DSOX WHO_AM_I ERROR!"
        );


        ESP_LOGE(
            TAG,
            "Expected 0x6C, received 0x%02X",
            who_am_i
        );


        abort();
    }


    /*
     * -------------------------------------------------------------------------
     * CTRL3_C
     * -------------------------------------------------------------------------
     *
     * 0x44:
     *
     * bit 6 = BDU
     * bit 2 = IF_INC
     *
     * Also keeps SIM = 0, meaning 4-wire SPI.
     */
    ESP_ERROR_CHECK(
        lsm6dsox_write_register(
            LSM6DSOX_CTRL3_C,
            0x44
        )
    );


    /*
     * -------------------------------------------------------------------------
     * CTRL1_XL
     * -------------------------------------------------------------------------
     *
     * Accelerometer:
     *
     * ODR = 104 Hz
     * FS  = +/-4 g
     *
     * 0x48
     */
    ESP_ERROR_CHECK(
        lsm6dsox_write_register(
            LSM6DSOX_CTRL1_XL,
            0x48
        )
    );


    /*
     * -------------------------------------------------------------------------
     * CTRL2_G
     * -------------------------------------------------------------------------
     *
     * Gyroscope:
     *
     * ODR = 104 Hz
     * FS  = +/-2000 dps
     *
     * 0x4C
     */
    ESP_ERROR_CHECK(
        lsm6dsox_write_register(
            LSM6DSOX_CTRL2_G,
            0x4C
        )
    );


    /*
     * -------------------------------------------------------------------------
     * Verify CTRL3_C
     * -------------------------------------------------------------------------
     *
     * This is useful while debugging SPI.
     */
    uint8_t ctrl3_c = 0;


    ESP_ERROR_CHECK(
        lsm6dsox_read_register(
            LSM6DSOX_CTRL3_C,
            &ctrl3_c
        )
    );


    ESP_LOGI(
        TAG,
        "LSM6DSOX CTRL3_C = 0x%02X",
        ctrl3_c
    );


    ESP_LOGI(
        TAG,
        "LSM6DSOX initialized successfully"
    );
}


/*
 * =============================================================================
 * READ LSM6DSOX INTERNAL TEMPERATURE
 * =============================================================================
 *
 * Registers:
 *
 *     0x20 = OUT_TEMP_L
 *     0x21 = OUT_TEMP_H
 *
 * Raw value:
 *
 *     signed 16-bit
 *
 * Conversion:
 *
 *     temperature = raw / 256 + 25
 *
 * =============================================================================
 */

static double read_lsm6dsox_temperature(void)
{
    uint8_t temp_l = 0;

    uint8_t temp_h = 0;


    /*
     * -------------------------------------------------------------------------
     * Read temperature LOW byte
     * -------------------------------------------------------------------------
     */

    esp_err_t ret =
        lsm6dsox_read_register(
            LSM6DSOX_OUT_TEMP_L,
            &temp_l
        );


    if (ret != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Failed reading OUT_TEMP_L: %s",
            esp_err_to_name(ret)
        );

        return 0.0;
    }


    /*
     * -------------------------------------------------------------------------
     * Read temperature HIGH byte
     * -------------------------------------------------------------------------
     */

    ret =
        lsm6dsox_read_register(
            LSM6DSOX_OUT_TEMP_H,
            &temp_h
        );


    if (ret != ESP_OK) {

        ESP_LOGE(
            TAG,
            "Failed reading OUT_TEMP_H: %s",
            esp_err_to_name(ret)
        );

        return 0.0;
    }


    /*
     * -------------------------------------------------------------------------
     * Combine bytes
     * -------------------------------------------------------------------------
     *
     * LSM6DSOX uses:
     *
     *     TEMP_H:TEMP_L
     *
     * with TEMP_L at the lower address.
     */

    int16_t raw =
        (int16_t)(
            ((uint16_t)temp_h << 8) |
            (uint16_t)temp_l
        );


    /*
     * -------------------------------------------------------------------------
     * Convert to Celsius
     * -------------------------------------------------------------------------
     */

    double temperatureC =
        ((double)raw / 256.0) + 25.0;


    /*
     * Debug information.
     *
     * This is intentionally kept for now because your previous
     * readings were clearly invalid.
     */
    ESP_LOGI(
        TAG,
        "IMU raw: H=0x%02X L=0x%02X raw=%d temp=%.2f C",
        temp_h,
        temp_l,
        raw,
        temperatureC
    );


    return temperatureC;
}


/*
 * =============================================================================
 * GPIO 16 INITIALIZATION
 * =============================================================================
 */

static void board_control_init(void)
{
    gpio_config_t config = {
        .pin_bit_mask =
            (1ULL << BOARD_CONTROL_PIN),

        .mode =
            GPIO_MODE_OUTPUT,

        .pull_up_en =
            GPIO_PULLUP_DISABLE,

        .pull_down_en =
            GPIO_PULLDOWN_DISABLE,

        .intr_type =
            GPIO_INTR_DISABLE
    };


    ESP_ERROR_CHECK(
        gpio_config(
            &config
        )
    );


    /*
     * Match original Arduino:
     *
     *     pinMode(16, OUTPUT);
     *     digitalWrite(16, LOW);
     */
    ESP_ERROR_CHECK(
        gpio_set_level(
            BOARD_CONTROL_PIN,
            0
        )
    );
}


/*
 * =============================================================================
 * MAIN APPLICATION
 * =============================================================================
 */

void app_main(void)
{
    /*
     * -------------------------------------------------------------------------
     * GPIO 16
     * -------------------------------------------------------------------------
     */

    board_control_init();


    /*
     * -------------------------------------------------------------------------
     * LMT87
     * -------------------------------------------------------------------------
     */

    lmt87_adc_init();


    /*
     * -------------------------------------------------------------------------
     * LSM6DSOX
     * -------------------------------------------------------------------------
     */

    lsm6dsox_init();


    /*
     * -------------------------------------------------------------------------
     * Startup information
     * -------------------------------------------------------------------------
     */

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX vs LMT87 Temperature"
    );

    ESP_LOGI(
        TAG,
        "========================================"
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX CS   : GPIO %d",
        IMU_CS
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX SCK  : GPIO %d",
        IMU_SCK
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX MOSI : GPIO %d",
        IMU_MOSI
    );

    ESP_LOGI(
        TAG,
        "LSM6DSOX MISO : GPIO %d",
        IMU_MISO
    );

    ESP_LOGI(
        TAG,
        "LMT87        : GPIO %d / ADC1 CH0",
        LMT_PIN
    );


    /*
     * -------------------------------------------------------------------------
     * Main loop
     * -------------------------------------------------------------------------
     */

    while (1) {

        /*
         * Read LSM6DSOX internal temperature.
         */
        double imu_temperature =
            read_lsm6dsox_temperature();


        /*
         * Read LMT87.
         */
        double lmt_temperature =
            read_lmt87_temperature();


        /*
         * Print comparison.
         */
        ESP_LOGI(
            TAG,
            "IMU: %.1f C | LMT87: %.1f C",
            imu_temperature,
            lmt_temperature
        );


        /*
         * One second between measurements.
         */
        vTaskDelay(
            pdMS_TO_TICKS(1000)
        );
    }
}
