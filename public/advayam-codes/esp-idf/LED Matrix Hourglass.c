/*
  5x5 Falling Sand Timer

  - Sand starts at the TOP row, CENTER pixel
  - Each grain falls downward
  - Grains pile up naturally
  - Matrix gradually fills with sand
  - All 25 pixels are filled in approximately 60 seconds

  Matrix coordinates:

      0 1 2 3 4
    0 . . S . .
    1 . . ↓ . .
    2 . . ↓ . .
    3 . . ↓ . .
    4 . . ↓ . .

  S = starting point
*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_timer.h"
#include "esp_random.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define WIDTH              5
#define HEIGHT             5
#define NUM_LEDS           25
#define POWER_PIN          16

#define TOTAL_TIME         60000UL

// IMU SPI Pins
#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

// LSM6DSOX Registers
#define LSM6DSOX_WHO_AM_I  0x0F
#define LSM6DSOX_CTRL1_XL  0x10
#define LSM6DSOX_OUTX_L_XL 0x28

static const char *TAG = "sand_timer";
static led_strip_handle_t led_strip;
static spi_device_handle_t spi_imu;

// Sand Timer State
static bool orientationFlipped = false;
static int fillDirection = 1; // 1 = filling, -1 = draining
static float refDown[3] = {0, 0, 0};

static bool flipCandidate = false;
static uint32_t flipHoldStart = 0;
static uint32_t lastFlipTime = 0;

const float FLIP_DOT_THRESHOLD = -0.5f;
const uint32_t FLIP_HOLD_MS = 300;
const uint32_t FLIP_COOLDOWN_MS = 800;

static bool sand[HEIGHT][WIDTH];
static int grainOrder[NUM_LEDS][2];
static int grainCount = 0;

// Serpentine wiring coordinate conversion
static int pixel_xy(int x, int y) {
    if (y % 2 == 0) {
        return y * WIDTH + x;
    }
    return y * WIDTH + (WIDTH - 1 - x);
}

// Low-level SPI read/write for LSM6DSOX
static void imu_write_reg(uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    gpio_set_level(IMU_CS, 0);
    spi_device_transmit(spi_imu, &t);
    gpio_set_level(IMU_CS, 1);
}

static void imu_read_regs(uint8_t reg, uint8_t *data, size_t len) {
    uint8_t tx_data[1 + len];
    uint8_t rx_data[1 + len];
    memset(tx_data, 0, sizeof(tx_data));
    tx_data[0] = reg | 0x80; // Read bit set

    spi_transaction_t t = {
        .length = (1 + len) * 8,
        .tx_buffer = tx_data,
        .rx_buffer = rx_data,
    };
    gpio_set_level(IMU_CS, 0);
    spi_device_transmit(spi_imu, &t);
    gpio_set_level(IMU_CS, 1);

    memcpy(data, &rx_data[1], len);
}

static void init_imu_sensor(void) {
    spi_bus_config_t buscfg = {
        .miso_io_num = IMU_MISO,
        .mosi_io_num = IMU_MOSI,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = -1, // Manually control CS
        .queue_size = 7,
    };

    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &spi_imu));

    gpio_config_t cs_conf = {
        .pin_bit_mask = (1ULL << IMU_CS),
        .mode = GPIO_MODE_OUTPUT,
    };
    gpio_config(&cs_conf);
    gpio_set_level(IMU_CS, 1);

    vTaskDelay(pdMS_TO_TICKS(50));

    uint8_t whoami = 0;
    imu_read_regs(LSM6DSOX_WHO_AM_I, &whoami, 1);
    ESP_LOGI(TAG, "LSM6DSOX WHO_AM_I: 0x%02X", whoami);

    // Configure Accelerometer: 104 Hz, +/- 4g
    imu_write_reg(LSM6DSOX_CTRL1_XL, 0x48);
}

static void read_gravity(float *x, float *y, float *z) {
    uint8_t raw_data[6];
    imu_read_regs(LSM6DSOX_OUTX_L_XL, raw_data, 6);

    int16_t raw_x = (int16_t)(raw_data[1] << 8 | raw_data[0]);
    int16_t raw_y = (int16_t)(raw_data[3] << 8 | raw_data[2]);
    int16_t raw_z = (int16_t)(raw_data[5] << 8 | raw_data[4]);

    // Sensitivity for ±4g is 0.122 mg/LSB -> convert to g's
    *x = (raw_x * 0.122f) / 1000.0f;
    *y = (raw_y * 0.122f) / 1000.0f;
    *z = (raw_z * 0.122f) / 1000.0f;

    float mag = sqrt((*x) * (*x) + (*y) * (*y) + (*z) * (*z));
    if (mag > 0.0001f) {
        *x /= mag;
        *y /= mag;
        *z /= mag;
    }
}

static void set_pixel_xy(int x, int y, uint32_t color) {
    if (x < 0 || x >= WIDTH || y < 0 || y >= HEIGHT) return;

    int hx = (HEIGHT - 1) - y;
    int hy = x;

    if (orientationFlipped) {
        hx = (WIDTH - 1) - hx;
        hy = (HEIGHT - 1) - hy;
    }

    led_strip_set_pixel(led_strip, pixel_xy(hx, hy), (color >> 16) & 0xFF, (color >> 8) & 0xFF, color & 0xFF);
}

static void draw_sand(void) {
    led_strip_clear(led_strip);
    uint32_t sandColor = 0xFF7800; // Orange

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            if (sand[y][x]) {
                set_pixel_xy(x, y, sandColor);
            }
        }
    }
    led_strip_refresh(led_strip);
}

static bool check_flip(void) {
    float gx, gy, gz;
    read_gravity(&gx, &gy, &gz);

    uint32_t now_ms = (uint32_t)(esp_timer_get_time() / 1000ULL);

    if (now_ms - lastFlipTime < FLIP_COOLDOWN_MS)
        return false;

    float dot = gx * refDown[0] + gy * refDown[1] + gz * refDown[2];

    if (dot < FLIP_DOT_THRESHOLD) {
        if (!flipCandidate) {
            flipCandidate = true;
            flipHoldStart = now_ms;
        }

        if (now_ms - flipHoldStart >= FLIP_HOLD_MS) {
            orientationFlipped = !orientationFlipped;
            fillDirection = -fillDirection;

            refDown[0] = gx;
            refDown[1] = gy;
            refDown[2] = gz;

            flipCandidate = false;
            lastFlipTime = now_ms;
            return true;
        }
    } else {
        flipCandidate = false;
    }

    return false;
}

static bool add_grain(void) {
    int x = 2;
    int y = 0;

    if (sand[y][x]) {
        bool found = false;
        for (int offset = 1; offset <= 2; offset++) {
            int left = 2 - offset;
            int right = 2 + offset;

            if (left >= 0 && !sand[0][left]) {
                x = left;
                found = true;
                break;
            }
            if (right < WIDTH && !sand[0][right]) {
                x = right;
                found = true;
                break;
            }
        }
        if (!found) return false;
    }

    while (true) {
        if (check_flip()) return false;

        if (y + 1 < HEIGHT && !sand[y + 1][x]) {
            y++;
            draw_sand();
            set_pixel_xy(x, y, 0xFF9614);
            led_strip_refresh(led_strip);
            vTaskDelay(pdMS_TO_TICKS(80));
            continue;
        }

        bool moved = false;
        if (esp_random() % 2 == 0) {
            if (x > 0 && y + 1 < HEIGHT && !sand[y + 1][x - 1]) {
                x--; y++; moved = true;
            } else if (x < WIDTH - 1 && y + 1 < HEIGHT && !sand[y + 1][x + 1]) {
                x++; y++; moved = true;
            }
        } else {
            if (x < WIDTH - 1 && y + 1 < HEIGHT && !sand[y + 1][x + 1]) {
                x++; y++; moved = true;
            } else if (x > 0 && y + 1 < HEIGHT && !sand[y + 1][x - 1]) {
                x--; y++; moved = true;
            }
        }

        if (moved) {
            draw_sand();
            set_pixel_xy(x, y, 0xFF9614);
            led_strip_refresh(led_strip);
            vTaskDelay(pdMS_TO_TICKS(80));
            continue;
        }

        sand[y][x] = true;
        grainOrder[grainCount][0] = x;
        grainOrder[grainCount][1] = y;
        grainCount++;

        draw_sand();
        return true;
    }
}

static void remove_grain(void) {
    if (grainCount <= 0) return;

    grainCount--;
    int x = grainOrder[grainCount][0];
    int y = grainOrder[grainCount][1];

    draw_sand();
    set_pixel_xy(x, y, 0xFFDC78);
    led_strip_refresh(led_strip);
    vTaskDelay(pdMS_TO_TICKS(80));

    sand[y][x] = false;
    draw_sand();
}

void app_main(void) {
    // 1. Configure Power Pin (GPIO 16) as output and keep LOW
    gpio_config_t power_conf = {
        .pin_bit_mask = (1ULL << POWER_PIN),
        .mode = GPIO_MODE_OUTPUT,
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

    memset(sand, false, sizeof(sand));
    grainCount = 0;

    // Initialize IMU
    init_imu_sensor();

    // Calibrate initial reference gravity direction
    float sx = 0, sy = 0, sz = 0;
    for (int i = 0; i < 20; i++) {
        float gx, gy, gz;
        read_gravity(&gx, &gy, &gz);
        sx += gx; sy += gy; sz += gz;
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    refDown[0] = sx / 20.0f;
    refDown[1] = sy / 20.0f;
    refDown[2] = sz / 20.0f;

    float mag = sqrt(refDown[0] * refDown[0] + refDown[1] * refDown[1] + refDown[2] * refDown[2]);
    if (mag > 0.0001f) {
        refDown[0] /= mag;
        refDown[1] /= mag;
        refDown[2] /= mag;
    }

    orientationFlipped = false;
    fillDirection = 1;

    ESP_LOGI(TAG, "Falling Sand Timer Initialized.");

    const uint32_t grainInterval = TOTAL_TIME / NUM_LEDS;
    uint32_t lastGrainTime = (uint32_t)(esp_timer_get_time() / 1000ULL);

    while (1) {
        check_flip();

        uint32_t now = (uint32_t)(esp_timer_get_time() / 1000ULL);

        if (now - lastGrainTime >= grainInterval) {
            if (fillDirection == 1 && grainCount < NUM_LEDS) {
                add_grain();
                lastGrainTime = now;
            } else if (fillDirection == -1 && grainCount > 0) {
                remove_grain();
                lastGrainTime = now;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(20));
    }
}