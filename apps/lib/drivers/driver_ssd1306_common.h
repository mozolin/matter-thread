#pragma once

#include <app_priv.h>

#if CONFIG_SSD1306_ENABLED

  //-- Custom Degree Symbol (8x8px)
  extern const uint8_t degree_symbol[];
  
  extern esp_err_t ssd1306_init_app(void);

  extern void ssd1306_draw_degree_symbol(uint8_t x, uint8_t y);
  
  //=== управление включением/выключением экрана ===
  // Включает/выключает дисплей SSD1306.
  // enabled=true  -> включить панель, показать заголовок, запомнить время.
  // enabled=false -> очистить экран и погасить панель.
  extern void ssd1306_set_display_enabled(bool enabled);
  //=== обратный отсчёт до авто-отключения ===
  extern void ssd1306_update_timeout_line(void);

#endif
