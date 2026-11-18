#include "oled_display.h"

#include <stdint.h>
#include "driver/i2c_master.h"
#include "esp_check.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_ssd1306.h"

#define OLED_WIDTH    128
#define OLED_HEIGHT   64
#define OLED_ADDRESS  0x3C

static const char *TAG = "OLED";

/* This module owns the bus and display resources. */
static i2c_master_bus_handle_t s_bus = NULL;
static esp_lcd_panel_io_handle_t s_io = NULL;
static esp_lcd_panel_handle_t s_panel = NULL;

esp_err_t oled_display_init(void)
{
    ESP_RETURN_ON_FALSE(!s_bus, ESP_ERR_INVALID_STATE, TAG,
                        "OLED already initialized");

    esp_err_t ret = ESP_OK;

    i2c_master_bus_config_t bus_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };

    esp_lcd_panel_io_i2c_config_t io_config = {
        .dev_addr = OLED_ADDRESS,
        .scl_speed_hz = 100000,
        .control_phase_bytes = 1,
        .dc_bit_offset = 6,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .transaction_timeout_ms = 1000,
    };

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,
        .bits_per_pixel = 1,
    };

    ESP_GOTO_ON_ERROR(i2c_new_master_bus(&bus_config, &s_bus),
                      cleanup, TAG, "Create I2C bus");
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_io_i2c(s_bus, &io_config, &s_io),
                      cleanup, TAG, "Create OLED connection");
    ESP_GOTO_ON_ERROR(esp_lcd_new_panel_ssd1306(s_io, &panel_config, &s_panel),
                      cleanup, TAG, "Create SSD1306 driver");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_reset(s_panel),
                      cleanup, TAG, "Reset OLED");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_init(s_panel),
                      cleanup, TAG, "Initialize OLED");
    ESP_GOTO_ON_ERROR(esp_lcd_panel_disp_on_off(s_panel, true),
                      cleanup, TAG, "Enable OLED");

    return ESP_OK;

cleanup:
    /* Release partially created resources if initialization fails. */
    if (s_panel) {
        esp_lcd_panel_del(s_panel);
        s_panel = NULL;
    }
    if (s_io) {
        esp_lcd_panel_io_del(s_io);
        s_io = NULL;
    }
    if (s_bus) {
        i2c_del_master_bus(s_bus);
        s_bus = NULL;
    }
    return ret;
}

esp_err_t oled_display_show_test_pattern(void)
{
    ESP_RETURN_ON_FALSE(s_panel, ESP_ERR_INVALID_STATE, TAG,
                        "Initialize OLED before drawing");

    static uint8_t pattern[OLED_WIDTH * OLED_HEIGHT / 8];

    for (size_t i = 0; i < sizeof(pattern); i++) {
        pattern[i] = ((i / 16) % 2 == 0) ? 0xFF : 0x00;
    }

    return esp_lcd_panel_draw_bitmap(
        s_panel, 0, 0, OLED_WIDTH, OLED_HEIGHT, pattern);
}