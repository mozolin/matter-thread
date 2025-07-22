#pragma once

#include <app_priv.h>

#if USE_SSD1306_DRIVER

extern esp_err_t ssd1306_init(void);
extern void show_plug_status(uint8_t plug_num, bool state);
extern void show_square(int idx, bool state);

#endif
