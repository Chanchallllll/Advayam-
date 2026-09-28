#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <fcntl.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "led_strip.h"

#define MATRIX_PIN         7
#define NUM_LEDS           25
#define MATRIX_BRIGHTNESS  25 // Scale (0 - 255)

#define IMU_CS             10
#define IMU_SCK            12
#define IMU_MOSI           11
#define IMU_MISO           13

#define PIN_EXTRA          16

#define MIRROR_FONT        true

static const char *TAG = "matrix_scroll";

static led_strip_handle_t led_strip;
static spi_device_handle_t imu_handle;

static char message[64] = "ADVAYAM";
static char input_buf[64] = "";
static int input_len = 0;

typedef struct {
    uint8_t r, g, b;
} pixel_color_t;
static pixel_color_t frame_buffer[NUM_LEDS];

// =====================================================
// 5x5 Font Definition
// =====================================================
static const uint8_t font[26][5] = {
    {0b01110,0b10001,0b11111,0b10001,0b10001}, // A
    {0b11110,0b10001,0b11110,0b10001,0b11110}, // B
    {0b01111,0b10000,0b10000,0b10000,0b01111}, // C
    {0b11110,0b10001,0b10001,0b10001,0b11110}, // D
    {0b11111,0b10000,0b11110,0b10000,0b11111}, // E
    {0b11111,0b10000,0b11110,0b10000,0b10000}, // F
    {0b01111,0b10000,0b10111,0b10001,0b01111}, // G
    {0b10001,0b10001,0b11111,0b10001,0b10001}, // H
    {0b01110,0b00100,0b00100,0b00100,0b01110}, // I
    {0b00111,0b00010,0b00010,0b10010,0b01100}, // J
    {0b10001,0b10010,0b11100,0b10010,0b10001}, // K
    {0b10000,0b10000,0b10000,0b10000,0b11111}, // L
    {0b10001,0b11011,0b10101,0b10001,0b10001}, // M
    {0b10001,0b11001,0b10101,0b10011,0b10001}, // N
    {0b01110,0b10001,0b10001,0b10001,0b01110}, // O
    {0b11110,0b10001,0b11110,0b10000,0b10000}, // P
    {0b01110,0b10001,0b10001,0b10101,0b01111}, // Q
    {0b11110,0b10001,0b11110,0b10100,0b10010}, // R
    {0b01111,0b10000,0b01110,0b00001,0b11110}, // S
    {0b11111,0b00100,0b00100,0b00100,0b00100}, // T
    {0b10001,0b10001,0b10001,0b10001,0b01110}, // U
    {0b10001,0b10001,0b10001,0b01010,0b00100}, // V
    {0b10001,0b10001,0b10101,0b11011,0b10001}, // W
    {0b10001,0b01010,0b00100,0b01010,0b10001}, // X
    {0b10001,0b01010,0b00100,0b00100,0b00100}, // Y
    {0b11111,0b00010,0b00100,0b01000,0b11111}  // Z
};

// =====================================================
// Helper Functions: IMU & Matrix Mapping
// =====================================================
static esp_err_t imu_write_reg(spi_device_handle_t handle, uint8_t reg, uint8_t val) {
    uint8_t tx_data[2] = {reg & 0x7F, val};
    spi_transaction_t t = {
        .length = 16,
        .tx_buffer = tx_data,
    };
    return spi_device_transmit(handle, &t);
}

static esp_err_t imu_read_regs(spi_device_handle_t handle, uint8_t reg, uint8_t *rx_buf, size_t len) {
    uint8_t *temp_tx = malloc(len + 1);
    uint8_t *temp_rx = malloc(len + 1);
    if (!temp_tx || !temp_rx) {
        free(temp_tx);
        free(temp_rx);
        return ESP_ERR_NO_MEM;
    }
    temp_tx[0] = reg | 0x80;
    memset(&temp_tx[1], 0, len);

    spi_transaction_t t = {
        .length = (len + 1) * 8,
        .tx_buffer = temp_tx,
        .rx_buffer = temp_rx,
    };
    esp_err_t err = spi_device_transmit(handle, &t);
    if (err == ESP_OK) {
        memcpy(rx_buf, &temp_rx[1], len);
    }
    free(temp_tx);
    free(temp_rx);
    return err;
}

int pixelXY(int x, int y) {
    if (x < 0 || x >= 5 || y < 0 || y >= 5) return -1;
    if (y % 2 == 0) return y * 5 + x;
    return y * 5 + (4 - x);
}

void setXY(int x, int y, uint8_t r, uint8_t g, uint8_t b) {
    int index = pixelXY(x, y);
    if (index >= 0 && index < NUM_LEDS) {
        frame_buffer[index].r = (r * MATRIX_BRIGHTNESS) / 255;
        frame_buffer[index].g = (g * MATRIX_BRIGHTNESS) / 255;
        frame_buffer[index].b = (b * MATRIX_BRIGHTNESS) / 255;
    }
}

int getOrientation(void) {
    uint8_t raw_data[6];
    if (imu_read_regs(imu_handle, 0x28, raw_data, 6) != ESP_OK) return 0;

    int16_t ax_raw = (int16_t)((raw_data[1] << 8) | raw_data[0]);
    int16_t ay_raw = (int16_t)((raw_data[3] << 8) | raw_data[2]);
    int16_t az_raw = (int16_t)((raw_data[5] << 8) | raw_data[4]);

    float x = (float)ax_raw;
    float y = (float)ay_raw;
    float z = (float)az_raw;

    float ax = fabsf(x);
    float ay = fabsf(y);
    float az = fabsf(z);

    if (ax >= ay && ax >= az) {
        return (x > 0) ? 1 : 0;
    }
    if (ay >= ax && ay >= az) {
        return (y > 0) ? 2 : 3;
    }
    return (z > 0) ? 0 : 1;
}

void drawChar(char c, int xOffset, int orientation, uint8_t r, uint8_t g, uint8_t b) {
    if (c >= 'a' && c <= 'z') c -= 32;
    if (c < 'A' || c > 'Z') return;

    const uint8_t* glyph = font[c - 'A'];

    for (int y = 0; y < 5; y++) {
        for (int x = 0; x < 5; x++) {
            if (!(glyph[y] & (1 << (4 - x)))) continue;

            int correctedX = MIRROR_FONT ? (4 - x) : x;
            int rx = correctedX;
            int ry = y;

            if (orientation == 1) {
                rx = 4 - correctedX;
                ry = 4 - y;
            } else if (orientation == 2) {
                rx = y;
                ry = 4 - correctedX;
            } else if (orientation == 3) {
                rx = 4 - y;
                ry = correctedX;
            }

            setXY(rx + xOffset, ry, r, g, b);
        }
    }
}

void scrollText(void) {
    int width = strlen(message) * 6;
    int orientation = getOrientation();

    for (int offset = 5; offset > -width; offset--) {
        memset(frame_buffer, 0, sizeof(frame_buffer));

        for (int i = 0; i < strlen(message); i++) {
            drawChar(message[i], offset + (i * 6), orientation, 0, 180, 255);
        }

        for (int i = 0; i < NUM_LEDS; i++) {
            led_strip_set_pixel(led_strip, i, frame_buffer[i].r, frame_buffer[i].g, frame_buffer[i].b);
        }
        led_strip_refresh(led_strip);

        vTaskDelay(pdMS_TO_TICKS(100));

        // Non-blocking read from stdin (connected to PlatformIO serial monitor)
        char c;
        int len = read(STDIN_FILENO, &c, 1);
        if (len > 0) {
            if (c == '\n' || c == '\r') {
                if (input_len > 0) {
                    input_buf[input_len] = '\0';
                    strncpy(message, input_buf, sizeof(message) - 1);
                    input_len = 0;
                    ESP_LOGI(TAG, "New text: %s", message);
                    return;
                }
            } else if (input_len < sizeof(input_buf) - 1) {
                input_buf[input_len++] = c;
            }
        }
    }
}

void app_main(void) {
    // Set stdin to non-blocking mode so serial input works without driver conflicts
    int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << PIN_EXTRA),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    gpio_set_level(PIN_EXTRA, 0);

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

    spi_bus_config_t buscfg = {
        .miso_io_num = IMU_MISO,
        .mosi_io_num = IMU_MOSI,
        .sclk_io_num = IMU_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 32,
    };
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &buscfg, SPI_DMA_CH_AUTO));

    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 1 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = IMU_CS,
        .queue_size = 7,
    };
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &devcfg, &imu_handle));

    if (imu_write_reg(imu_handle, 0x10, 0x48) != ESP_OK) {
        ESP_LOGE(TAG, "IMU initialization failed!");
        while (1) { vTaskDelay(pdMS_TO_TICKS(100)); }
    }

    ESP_LOGI(TAG, "Automatic Text Orientation Ready. Default text: ADVAYAM");

    while (1) {
        scrollText();
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}