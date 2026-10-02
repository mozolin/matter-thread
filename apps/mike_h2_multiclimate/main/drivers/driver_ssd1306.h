#pragma once

#include <app_priv.h>

#if USE_SSD1306_DRIVER

  //-- Custom Degree Symbol (8x8px)
  extern const uint8_t degree_symbol[];
  
  extern esp_err_t ssd1306_i2c_init(void);
  extern void ssd1306_draw_degree_symbol(uint8_t x, uint8_t y);

  extern void ssd1306_show_sensor_data(uint8_t y_pos, float temp, float hum, float pres, float gas);

#endif
