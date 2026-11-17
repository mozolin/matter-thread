#pragma once

#if CONFIG_SSD1306_ENABLED
  #include "ssd1306_i2cdev.h"

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

  //extern void ssd1306_empty_screen(void);
  extern void ssd1306_empty_screen(SSD1306_t * dev, bool invert);

  //-- Init LCD SSD1306
  extern void ssd1306_init_app_main(void);
  
  #ifdef CONFIG_SSD1306_APP_RUN_CB
    extern void ssd1306_app_run_cb(void);
  #endif
  
  
  //-- SSD1306 device instance
  extern SSD1306_t ssd1306dev;
  //-- Is SSD1306 initialized?
  extern bool ssd1306_initialized;
  //=== состояние дисплея (по умолчанию — ВЫКЛЮЧЕН) ===
  extern bool ssd1306_display_enabled;
  extern uint64_t ssd1306_last_on_time_ms;
  

#endif
