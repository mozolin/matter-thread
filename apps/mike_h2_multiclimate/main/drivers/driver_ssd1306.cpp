
#include "driver_ssd1306.h"
#include <app_priv.h>

#if CONFIG_SSD1306_ENABLED

  void ssd1306_show_title()
  {
    //-- if not initialized
    if(!ssd1306_initialized) {
      return;
    }

    ssd1306_clear_screen(&ssd1306dev, false);
    ssd1306_contrast(&ssd1306dev, 0xff);        // программное включение панели
    //ssd1306_display_text(&ssd1306dev, 0, " MATTER--THREAD ", 16, false);
    //ssd1306_display_text(&ssd1306dev, 1, "  MULTICLIMATE  ", 16, false);
    //ssd1306_display_text(&ssd1306dev, 2, "----------------", 16, false);
    //ssd1306_display_text(&ssd1306dev, 7, "----------------", 16, false);
    ssd1306_display_text(&ssd1306dev,    1, "ESP32-H2 THREAD", 16, false);
    //ssd1306_display_text(&ssd1306dev,    1, "MATTER/THREAD   ", 16, false);
    ssd1306_display_text_x2(&ssd1306dev, 2, "MULTI   ", 8, false);
    ssd1306_display_text_x2(&ssd1306dev, 4, "CLIMATE ", 8, false);
    ssd1306_display_text_x2(&ssd1306dev, 6, "(c) MIKE", 8, false);
    //ssd1306_display_text_x2(&ssd1306dev, 4, " MATTER ", 8, false);
    //ssd1306_display_text_x2(&ssd1306dev, 6, " THREAD ", 8, false);
    /*
    ssd1306_display_text(&ssd1306dev, 2, "#     #   ##### ", 16, false);
    ssd1306_display_text(&ssd1306dev, 3, "##   ##  #     #", 16, false);
    ssd1306_display_text(&ssd1306dev, 4, "# # # #  #      ", 16, false);
    ssd1306_display_text(&ssd1306dev, 5, "#  #  #  #     #", 16, false);
    ssd1306_display_text(&ssd1306dev, 6, "#     #   ##### ", 16, false);
    */
    
    vTaskDelay(pdMS_TO_TICKS(5000));
    //ssd1306_empty_screen(&ssd1306dev, true);
    ssd1306_clear_screen(&ssd1306dev, false);
  }
  
	void ssd1306_show_sensor_data(uint8_t y_pos, float temp, float hum, float pres, float gas, int8_t extra)
	{
		//-- if not initialized
    if(!ssd1306_initialized) {
		  return;
		}
		
		//=== Если экран выключен — не рисуем ===
		if(!ssd1306_display_enabled) {
		  return;
		}

    char buf[96];
    //-- temperature
    if(extra == -1) {
    	//-- MQ135 parameters (temp => co2)
    	snprintf(buf, sizeof(buf), "%.0f", temp);
    } else {
    	snprintf(buf, sizeof(buf), "%.1f", temp/100.0f);
    }
    
    ssd1306_display_text(&ssd1306dev, y_pos, buf, strlen(buf), false);

    //-- humidity
    if(hum > 0) {
    	char buf2[16];
    	if(extra == -1) {
    		//-- MQ135 parameters (temp => co2)
	    	snprintf(buf2, sizeof(buf2), " %.0f", hum);
  	  } else {
	    	snprintf(buf2, sizeof(buf2), " %.1f", hum/100.0f);
	    }
    	strcat(buf, buf2);
    }
    
    //-- pressure
    if(pres > 0) {
    	char buf3[16];
    	if(extra == -1) {
    		//-- MQ135 parameters (pres => tvoc)
	    	snprintf(buf3, sizeof(buf3), " %.0f", pres);
  	  } else {
	    	snprintf(buf3, sizeof(buf3), " %.0f", pres);
	    }
    	strcat(buf, buf3);
    }
    
    //-- gas resistance
    if(gas > 0) {
    	char buf4[16];
    	snprintf(buf4, sizeof(buf4), " %.0f", gas);
    	strcat(buf, buf4);
    }

    //-- just 1 byte of extra info
    if(extra > 1) {
    	char buf5[16];
    	snprintf(buf5, sizeof(buf5), " (%d)", extra);
    	strcat(buf, buf5);
    }

    ssd1306_display_text(&ssd1306dev, y_pos, buf, strlen(buf), false);
	}

	//-- callback function specific to this application
  //-- (can be used within the SSD1306 common driver)
	#ifdef CONFIG_SSD1306_APP_RUN_CB
    void ssd1306_app_run_cb()
    {
	    ssd1306_show_title();
    }
  #endif
	
#endif
