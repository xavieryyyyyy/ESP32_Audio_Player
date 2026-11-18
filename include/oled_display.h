#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include "esp_err.h"

/* Initialize the OLED hardware once at startup. */
esp_err_t oled_display_init(void);

/* Draw stripes to verify the display before adding the UI. */
esp_err_t oled_display_show_test_pattern(void);

#endif