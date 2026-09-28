/*
 * =============================================================================
 * HAND PROXIMITY REACTION GAME
 * =============================================================================
 * A simple reaction-time game using the proximity sensor.
 *
 * Game sequence:
 *
 *   1. Player places hand near sensor.
 *   2. System waits a random amount of time.
 *   3. Buzzer sounds.
 *   4. Player pulls hand away.
 *   5. Reaction time is displayed over UART.
 *
 * Buzzer:
 *   GPIO 16
 *
 * =============================================================================
 */
 /*
 * =============================================================================
 * HAND PROXIMITY REACTION GAME (ESP-IDF v5+ COMPATIBLE - PURE C)
 * =============================================================================
 * Uses the native ESP-IDF RPR-0521RS driver and modern I2C Master API.
 * 
 * Pin Configuration:
 *   - I2C SDA  : GPIO 5
 *   - I2C SCL  : GPIO 6
 *   - Buzzer   : GPIO 16
 * =============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "rpr0521rs.h"

#define I2C_MASTER_SDA_IO    5
#define I2C_MASTER_SCL_IO    6
#define I2C_MASTER_FREQ_HZ   100000

#define BUZZER_PIN           GPIO_NUM_16
#define LEDC_TIMER           LEDC_TIMER_0
#define LEDC_MODE            LEDC_LOW_SPEED_MODE
#define LEDC_CHANNEL         LEDC_CHANNEL_0
#define LEDC_DUTY_RES        LEDC_TIMER_13_BIT
#define LEDC_FREQUENCY       1500 // 1500 Hz tone

#define HAND_NEAR_THRESHOLD  500
#define HAND_AWAY_THRESHOLD  200

static const char *TAG = "REACTION_GAME";
static i2c_master_dev_handle_t sensor_dev = NULL;

// Milliseconds helper for ESP-IDF
static unsigned long millis(void) {
    return (unsigned long)(esp_timer_get_time() / 1000ULL);
}

// Initialize Modern ESP-IDF v5+ I2C Master Bus and Add Device
static void i2c_master_init(void) {
    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = I2C_MASTER_SDA_IO,
        .scl_io_num = I2C_MASTER_SCL_IO,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        }
    };
    i2c_master_bus_handle_t bus_handle;
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = RPR0521RS_DEVICE_ADDRESS,
        .scl_speed_hz = I2C_MASTER_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &sensor_dev));
}

// Initialize LEDC for Buzzer
static void buzzer_init(void) {
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_MODE,
        .timer_num = LEDC_TIMER,
        .duty_resolution = LEDC_DUTY_RES,
        .freq_hz = LEDC_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);

    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_MODE,
        .channel = LEDC_CHANNEL,
        .timer_sel = LEDC_TIMER,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = BUZZER_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);
}

static void tone_on(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 4096); // 50% duty cycle
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

static void tone_off(void) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

enum GameState {
    WAITING_FOR_HAND,
    COUNTDOWN,
    REACTION
};

void app_main(void) {
    i2c_master_init();
    buzzer_init();

    if (rpr0521rs_init(sensor_dev) != ESP_OK) {
        ESP_LOGE(TAG, "Sensor initialization failed!");
        while (1) {
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // Seed random generator using system timer ticks
    srand((unsigned int)millis());

    ESP_LOGI(TAG, "=== PROXIMITY REACTION GAME ===");
    ESP_LOGI(TAG, "Place your hand near the sensor.");

    enum GameState state = WAITING_FOR_HAND;
    unsigned long countdownStart = 0;
    unsigned long reactionStart = 0;

    while (1) {
        uint16_t proximity;
        float lux;

        if (rpr0521rs_get_psalsval(sensor_dev, &proximity, &lux) != ESP_OK) {
            vTaskDelay(pdMS_TO_TICKS(20));
            continue;
        }

        switch (state) {
            case WAITING_FOR_HAND:
                if (proximity > HAND_NEAR_THRESHOLD) {
                    ESP_LOGI(TAG, "Hand detected.");
                    ESP_LOGI(TAG, "Get ready...");

                    // Random delay between 1500ms and 4000ms
                    unsigned long random_delay = 1500 + (rand() % 2550);
                    countdownStart = millis() + random_delay;
                    state = COUNTDOWN;
                }
                break;

            case COUNTDOWN:
                if (millis() >= countdownStart) {
                    tone_on();
                    reactionStart = millis();
                    ESP_LOGI(TAG, "GO!");
                    state = REACTION;
                }
                break;

            case REACTION:
                if (proximity < HAND_AWAY_THRESHOLD) {
                    unsigned long reactionTime = millis() - reactionStart;
                    tone_off();

                    ESP_LOGI(TAG, "Reaction time: %lu ms", reactionTime);

                    if (reactionTime < 250) {
                        ESP_LOGI(TAG, "Amazing!");
                    } else if (reactionTime < 400) {
                        ESP_LOGI(TAG, "Great!");
                    } else if (reactionTime < 600) {
                        ESP_LOGI(TAG, "Good!");
                    } else {
                        ESP_LOGI(TAG, "Try again!");
                    }

                    ESP_LOGI(TAG, "\nPlace your hand near the sensor.\n");
                    state = WAITING_FOR_HAND;
                }
                break;
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}