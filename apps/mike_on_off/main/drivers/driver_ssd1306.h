#pragma once

#include <app_priv.h>

#if USE_SSD1306_DRIVER

  //-- Custom Degree Symbol (8x8px)
  extern const uint8_t degree_symbol[];
  
  extern esp_err_t ssd1306_init(void);
  extern void ssd1306_show_title(void);
  extern void ssd1306_show_plug_status(uint8_t plug_num, bool state);
  extern void ssd1306_show_square(int idx, bool state);
  #if CONFIG_IDF_TARGET_ESP32H2
  	//-- cannot be used outside the main application unless the target board is an ESP32-H2 (which does not have an FPU)
  	extern void ssd1306_refresh_display_task(void);
  #else
  	//-- can be used as a task if the target board is not ESP32-H2 (which does not have an FPU)
  	extern void ssd1306_refresh_display_task(void *pvParameter);
  #endif
  #if USE_TIME_DRIVER
    extern bool ssd1306_show_datetime(const tm& timeinfo);
  #endif
  extern void ssd1306_draw_degree_symbol(uint8_t x, uint8_t y);

#endif
