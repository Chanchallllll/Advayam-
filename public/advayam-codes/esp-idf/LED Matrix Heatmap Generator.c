/*
 * This program displays the LMT87 temperature as a vertical
 * heatmap on the 5x5 RGB matrix.
 *
 * Lower temperatures produce blue colors while higher
 * temperatures progress through green and yellow to red.
 */
 #include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define POWER_PIN          16
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

// Define ADC channel for TEMP_PIN (GPIO 1 corresponds to ADC1 Channel 0)
#define TEMP_ADC_CHANNEL   ADC_CHANNEL_0

static const char *TAG = "temperature_heatmap";
static led_strip_handle_t led_strip;
static adc_oneshot_unit_handle_t adc1_handle;
static adc_cali_handle_t adc_cali_handle = NULL;

// Serpentine wiring coordinate conversion
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * 5 + x;
    }
    return y * 5 + (4 - x);
}

// Initialize ADC calibration
static bool adc_calibration_init(adc_unit_t unit, adc_channel_t channel, adc_cali_handle_t *out_handle) {
    adc_cali_handle_t handle = NULL;
    esp_err_t ret = ESP_FAIL;
    bool calibrated = false;

#if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is Curve Fitting");
        adc_cali_curve_fitting_config_t cali_config = {
            .unit_id = unit,
            .chan = channel,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

#if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
    if (!calibrated) {
        ESP_LOGI(TAG, "calibration scheme version is Line Fitting");
        adc_cali_line_fitting_config_t cali_config = {
            .unit_id = unit,
            .atten = ADC_ATTEN_DB_12,
            .bitwidth = ADC_BITWIDTH_DEFAULT,
        };
        ret = adc_cali_create_scheme_line_fitting(&cali_config, &handle);
        if (ret == ESP_OK) {
            calibrated = true;
        }
    }
#endif

    *out_handle = handle;
    if (calibrated) {
        ESP_LOGI(TAG, "Cali Success");
    } else {
        ESP_LOGW(TAG, "Cali Fail");
    }
    return calibrated;
}

// Convert temperature to heat color with global brightness scaling
static void get_heat_color(int row_heat, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint32_t base_r = 0, base_g = 0, base_b = 0;

    if (row_heat < 85) {
        base_r = 0;
        base_g = row_heat * 3;
        base_b = 255;
    } else if (row_heat < 170) {
        base_r = (row_heat - 85) * 3;
        base_g = 255;
        base_b = 255 - (row_heat - 85) * 3;
    } else {
        base_r = 255;
        base_g = 255 - (row_heat - 170) * 3;
        base_b = 0;
    }

    // Apply global brightness scaling
    *r = (base_r * MATRIX_BRIGHTNESS) / 255;
    *g = (base_g * MATRIX_BRIGHTNESS) / 255;
    *b = (base_b * MATRIX_BRIGHTNESS) / 255;
}

void app_main(void) {
    // 1. Configure Power Pin (GPIO 16) as output and keep LOW
    gpio_config_t power_conf = {
        .pin_bit_mask = (1ULL << POWER_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&power_conf);
    gpio_set_level(POWER_PIN, 0);

    // 2. Configure LED Strip via RMT
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_LEDS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
    };
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000,
    };
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));

    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);

    // 3. Configure ADC1 for Temperature Sensor (GPIO 1 / Channel 0)
    adc_oneshot_unit_init_cfg_t init_config1 = {
        .unit_id = ADC_UNIT_1,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&init_config1, &adc1_handle));

    adc_oneshot_chan_cfg_t config = {
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc1_handle, TEMP_ADC_CHANNEL, &config));

    // Initialize ADC calibration
    adc_calibration_init(ADC_UNIT_1, TEMP_ADC_CHANNEL, &adc_cali_handle);

    ESP_LOGI(TAG, "Temperature Heatmap Initialized.");

    // 4. Main Loop
    while (1) {
        int adc_raw = 0;
        ESP_ERROR_CHECK(adc_oneshot_read(adc1_handle, TEMP_ADC_CHANNEL, &adc_raw));

        // Temperature calculation using your gain multiplication factor
        double temperatureC = adc_raw * 0.02863;

        // Print only the temperature to the logs
        ESP_LOGI(TAG, "Temperature: %.1f C", temperatureC);

        // Convert temperature to heat level
        int heat = (int)((temperatureC - 15.0) * 10.0);
        if (heat < 0) heat = 0;
        if (heat > 255) heat = 255;

        // Clear matrix
        for (int i = 0; i < NUM_LEDS; i++) {
            led_strip_set_pixel(led_strip, i, 0, 0, 0);
        }

        // Draw vertical heatmap (Bottom is hottest: y = 4 down to y = 0)
        for (int y = 0; y < 5; y++) {
            int row_heat = heat - y * 35;
            if (row_heat < 0) row_heat = 0;
            if (row_heat > 255) row_heat = 255;

            uint8_t r, g, b;
            get_heat_color(row_heat, &r, &g, &b);

            for (int x = 0; x < 5; x++) {
                int idx = pixel_xy(x, y);
                led_strip_set_pixel(led_strip, idx, r, g, b);
            }
        }

        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(500));
    }
}