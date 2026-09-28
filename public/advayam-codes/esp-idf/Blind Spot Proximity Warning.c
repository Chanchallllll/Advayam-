/*
 * =============================================================================
 * BLIND SPOT PROXIMITY WARNING
 * =============================================================================
 * Provides an audible warning when an object approaches the RPR-0521RS.
 *
 * Farther obstacle -> slow intermittent beep
 * Closer obstacle  -> faster beep
 *
 * Buzzer:
 *   GPIO 16
 * =============================================================================
 */
 #include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rpr0521rs.h"

// Hardware Pin & Bus Definitions
#define I2C_SDA_PIN     GPIO_NUM_5
#define I2C_SCL_PIN     GPIO_NUM_6
#define BUZZER_GPIO     GPIO_NUM_16
#define I2C_PORT        I2C_NUM_0

static const char *TAG = "BLIND_SPOT";

// Helper functions for mapping and constraining
static inline int constrain_val(int x, int low, int high) {
    return (x < low) ? low : (x > high) ? high : x;
}

static inline int map_val(int x, int in_min, int in_max, int out_min, int out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Initialize LEDC PWM for Buzzer (Replaces Arduino tone())
void buzzer_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_LOW_SPEED_MODE,
        .timer_num        = LEDC_TIMER_0,
        .duty_resolution  = LEDC_TIMER_10_BIT,
        .freq_hz          = 1800, // 1.8 kHz tone matching Arduino sketch
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_LOW_SPEED_MODE,
        .channel        = LEDC_CHANNEL_0,
        .timer_sel      = LEDC_TIMER_0,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_GPIO,
        .duty           = 0, // Off initially
        .hpoint         = 0
    };
    ledc_channel_config(&ledc_channel);
}

void buzzer_on(void) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // 50% duty cycle
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void buzzer_off(void) {
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting Blind Spot Proximity Warning System...");

    // 1. Initialize Modern ESP-IDF v5.x I2C Master Bus
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_PORT,
        .sda_io_num = I2C_SDA_PIN,
        .scl_io_num = I2C_SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = { .enable_internal_pullup = true }
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    // 2. Add RPR-0521RS Device to Bus
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = 100000,
    };
    i2c_master_dev_handle_t sensor_dev;
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &sensor_dev));

    // 3. Initialize Sensor using your provided driver header
    if (rpr0521rs_init(sensor_dev) != ESP_OK) {
        ESP_LOGE(TAG, "Sensor initialization failed!");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // 4. Initialize Buzzer PWM
    buzzer_init();

    int64_t last_beep = 0;
    bool buzzer_state = false;

    while (1) {
        uint16_t proximity = 0;
        float lux = 0.0f;

        // Read sensor values using your exact header API function
        if (rpr0521rs_get_psalsval(sensor_dev, &proximity, &lux) == ESP_OK) {
            if (proximity > 100) {
                // Map proximity to beep interval (matches Arduino logic)
                int constrained = constrain_val(proximity, 100, 2000);
                int interval = map_val(constrained, 100, 2000, 1000, 100);

                int64_t now = esp_timer_get_time() / 1000; // Convert microseconds to ms

                if (now - last_beep >= interval) {
                    last_beep = now;

                    if (!buzzer_state) {
                        buzzer_on();
                        buzzer_state = true;
                    } else {
                        buzzer_off();
                        buzzer_state = false;
                    }
                }
            } else {
                buzzer_off();
                buzzer_state = false;
            }
        } else {
            ESP_LOGW(TAG, "Failed to read sensor data");
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}