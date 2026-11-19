
#include "main_driver_ssd1306.h"
#include "ssd1306_font8x8_basic.h"
#include <app_priv.h>

#if CONFIG_SSD1306_ENABLED

  void ssd1306_show_title()
  {
    //-- if not initialized
    if(!ssd1306_initialized) {
      return;
    }

    ssd1306_clear_screen(&ssd1306dev, false);
    ssd1306_contrast(&ssd1306dev, 0xff);
    //ssd1306_display_text(&ssd1306dev, 0, " MATTER--THREAD ", 16, false);
    ssd1306_display_text(&ssd1306dev, 0, "    8 RELAYS    ", 16, false);
  }

  void ssd1306_show_plug_status(uint8_t plug_num, bool state)
  {
    //-- if not initialized
    if(!ssd1306_initialized) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Unable to show plug status: SSD1306 is not initialized!");
      #endif
      return;
    }
    
    //-- if wrong plug number
    if(plug_num > CONFIG_NUM_VIRTUAL_PLUGS) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Unable to show plug status: Wrong plug number (%d)!", plug_num);
      #endif
      return;
    }
    
    #if CONFIG_DEBUG_MODE
      //ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Plug %d state (ESP NVS): %s", plug_num, state ? "ON " : "OFF");
    #endif
    
    ssd1306_show_square(plug_num - 1, state);
  }

  void ssd1306_show_square(int idx, bool state)
  {
    uint8_t image[24];
    
    int top = (idx < 4) ? 2 : 5;
    int left = (idx < 4) ? ((idx*3)+2)*8 : ((idx-4)*3+2)*8;

    /*
      SSD1306 128x64
    ##################
    #     MATTER     #
    #   OVER THREAD  #
    #                #
    #   1  2  3  4   #
    #                #
    #                #
    #   5  6  7  8   #
    #                #
    ##################
    */

    #if CONFIG_DEBUG_MODE
      //ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ !SQUARE: TOP=%d, LEFT=%d", top, left);
    #endif

    memset(image, 0, sizeof(image));
    if(state) {
      ssd1306_invert(image, 24);
    }
    ssd1306_display_image(&ssd1306dev, top,   left, image, sizeof(image));
    ssd1306_display_image(&ssd1306dev, top+1, left, image, sizeof(image));
    ssd1306_display_image(&ssd1306dev, top+2, left, image, sizeof(image));

    int font = 0x30 + idx + 1;
    memcpy(image, ssd1306_font8x8_basic_tr[font], 8);
    if(state) {
      ssd1306_invert(image, 8);
    }
    ssd1306_display_image(&ssd1306dev, top+1, left+8, image, 8);
  }
  
  #if USE_TIME_DRIVER
    bool ssd1306_show_datetime(const tm& timeinfo)
    {
      //-- if not initialized
      if(!ssd1306_initialized) {
        return false;
      }
      
      //char time_str[32];
      //strftime(time_str, sizeof(time_str), "%H:%M:%S", &timeinfo);
      
      //char date_str[32];
      //strftime(date_str, sizeof(date_str), "%d.%m.%Y", &timeinfo);
      
      char datetime_str[32];
      strftime(datetime_str, sizeof(datetime_str), "%d%m%y %H:%M:%S", &timeinfo);
      
      //ssd1306_display_text(&ssd1306dev, 0, time_str, strlen(time_str), false);
      //ssd1306_display_text(&ssd1306dev, 1, date_str, strlen(date_str), false);
      ssd1306_display_text(&ssd1306dev, 0, datetime_str, strlen(datetime_str), false);
      
      return true;
    }
  #endif

  //-- callback function specific to this application
  //-- (can be used within the SSD1306 common driver)
	#ifdef CONFIG_SSD1306_APP_RUN_CB
    void ssd1306_app_run_cb()
    {
	    ssd1306_show_title();
    }
  #endif

#endif

