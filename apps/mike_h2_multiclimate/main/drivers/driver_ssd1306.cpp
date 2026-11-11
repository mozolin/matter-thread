
#include "driver_ssd1306.h"
#include <app_priv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306_i2cdev.h"
#include "font8x8_basic.h"

#include <type_traits>

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
    //ssd1306_display_text(&ssd1306dev, 1, "  MULTICLIMATE  ", 16, false);
    //ssd1306_display_text(&ssd1306dev, 2, "----------------", 16, false);
    //ssd1306_display_text(&ssd1306dev, 7, "----------------", 16, false);
    ssd1306_display_text(&ssd1306dev, 0, "  MULTICLIMATE  ", 16, false);

    /*
    char buf[32];
    uint8_t idx = 0;
    snprintf(buf, sizeof(buf), " SSD1306: %d %d %d", CONFIG_SSD1306_I2C_PORT, CONFIG_SSD1306_SDA_GPIO, CONFIG_SSD1306_SCL_GPIO);
    idx = 1;
    ssd1306_display_text(&ssd1306dev, idx, buf, strlen(buf), false);
    
    #if CONFIG_BME280_ENABLED
      snprintf(buf, sizeof(buf), "  BMP280: %d %d %d", CONFIG_BME280_I2C_PORT, CONFIG_BME280_SDA_GPIO, CONFIG_BME280_SCL_GPIO);
      idx++;
      ssd1306_display_text(&ssd1306dev, idx, buf, strlen(buf), false);
    #endif

    #if CONFIG_BME680_ENABLED
      snprintf(buf, sizeof(buf), "  BME680: %d %d %d", CONFIG_BME680_I2C_PORT, CONFIG_BME680_SDA_GPIO, CONFIG_BME680_SCL_GPIO);
      idx++;
      ssd1306_display_text(&ssd1306dev, idx, buf, strlen(buf), false);
    #endif
    */
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

#endif

