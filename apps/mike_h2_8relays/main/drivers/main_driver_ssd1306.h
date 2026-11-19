#pragma once

#include <app_priv.h>

#if CONFIG_SSD1306_ENABLED

  extern void ssd1306_show_title(void);
  extern void ssd1306_show_plug_status(uint8_t plug_num, bool state);
  extern void ssd1306_show_square(int idx, bool state);
  #if USE_TIME_DRIVER
    extern bool ssd1306_show_datetime(const tm& timeinfo);
  #endif

  #ifdef CONFIG_SSD1306_APP_RUN_CB
    extern void ssd1306_app_run_cb(void);
  #endif

#endif
