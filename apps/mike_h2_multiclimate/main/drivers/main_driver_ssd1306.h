#pragma once

#include <app_priv.h>

#if CONFIG_SSD1306_ENABLED
  
  void ssd1306_show_title(void);
  
  extern void ssd1306_show_sensor_data(uint8_t y_pos, float temp, float hum, float pres, float gas, int8_t extra);

  #ifdef CONFIG_SSD1306_APP_RUN_CB
    extern void ssd1306_app_run_cb(void);
  #endif

#endif
