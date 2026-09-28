/*
 * =============================================================================
 * AUTO-DIMMING MATRIX DISPLAY
 * =============================================================================
 * Reads ambient light from the ROHM RPR-0521RS sensor and automatically
 * adjusts the brightness of the 5x5 WS2812B RGB matrix.
 *
 * I2C:
 *   SDA = GPIO 5
 *   SCL = GPIO 6
 *
 * RGB Matrix:
 *   DATA = GPIO 7
 *
 * Sensor:
 *   RPR-0521RS
 * =============================================================================
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "driver/i2c_master.h"
#include "led_strip.h"
#include "rpr0521rs.h"

#define I2C_MASTER_SDA_IO           5
#define I2C_MASTER_SCL_IO           6
#define I2C_MASTER_FREQ_HZ          100000
#define I2C_PORT                    I2C_NUM_0

#define MATRIX_PIN                  7
#define NUM_PIXELS                  25

static const char *TAG = "AUTO_DIM_MATRIX";

// Helper function to mimic Arduino's map and constrain
static int calculate_brightness(float lux) {
    if (lux < 0.0f) lux = 0.0f;
    if (lux > 500.0f) lux = 500.0f;
    // Map 0-500 lux to 10-255 brightness
    return (int)(10.0f + (lux / 500.0f) * (255.0f - 10.0f));
}

void app_main(void) {
    // 1. Initialize I2C master bus for the sensor
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    // 2. Add RPR-0521RS device to the master bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    i2c_master_dev_handle_t dev_handle;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    // 3. Initialize the sensor
    if (rpr0521rs_init(dev_handle) != ESP_OK) {
        ESP_LOGE(TAG, "RPR-0521RS initialization failed!");
        return;
    }

    // 4. Initialize WS2812 LED Strip via RMT backend
    led_strip_handle_t led_strip;
    led_strip_config_t strip_config = {
        .strip_gpio_num = MATRIX_PIN,
        .max_leds = NUM_PIXELS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        .flags.invert_out = false,
    };
    
    led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = 10 * 1000 * 1000, // 10MHz resolution
        .flags.with_dma = false,
    };
    
    ESP_ERROR_CHECK(led_strip_new_rmt_device(&strip_config, &rmt_config, &led_strip));
    
    // Clear strip initially
    led_strip_clear(led_strip);
    led_strip_refresh(led_strip);

    ESP_LOGI(TAG, "Auto-Dimming Matrix Display started.");

    // 5. Main loop
    while (1) {
        uint16_t proximity;
        float lux;

        if (rpr0521rs_get_psalsval(dev_handle, &proximity, &lux) == ESP_OK) {
            int brightness = calculate_brightness(lux);

            // Set all pixels to soft blue (R:0, G:80, B:255) modulated by brightness scaling
            // The led_strip library sets absolute RGB values, so we scale them according to brightness ratio
            uint32_t red_val   = (0  * brightness) / 255;
            uint32_t green_val = (80 * brightness) / 255;
            uint32_t blue_val  = (255 * brightness) / 255;

            for (int i = 0; i < NUM_PIXELS; i++) {
                led_strip_set_pixel(led_strip, i, red_val, green_val, blue_val);
            }
            led_strip_refresh(led_strip);

            ESP_LOGI(TAG, "Lux: %.2f | Brightness Scale: %d", lux, brightness);
        } else {
            ESP_LOGW(TAG, "Failed to read sensor values.");
        }

        vTaskDelay(pdMS_TO_TICKS(300));
    }
}