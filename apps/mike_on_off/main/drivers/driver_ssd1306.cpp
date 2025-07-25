
#include "driver_ssd1306.h"
#include <app_priv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "ssd1306.h"
#include "font8x8_basic.h"

#include <type_traits>

#if USE_SSD1306_DRIVER
	esp_err_t ssd1306_init(void)
	{
		esp_err_t err = ESP_OK;
		
		#if CONFIG_I2C_INTERFACE
			ESP_LOGW(TAG_MIKE_APP, "~~~ INTERFACE is i2c");
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
			err = i2c_master_init(&ssd1306dev, CONFIG_SDA_GPIO, CONFIG_SCL_GPIO, CONFIG_RESET_GPIO);
			if(err != ESP_OK) {
				ESP_LOGW(TAG_MIKE_APP, "~~~ i2c_master_init() failed!");
				return err;
			}
		#endif // CONFIG_I2C_INTERFACE
	  
		#if CONFIG_SPI_INTERFACE
			ESP_LOGW(TAG_MIKE_APP, "~~~ INTERFACE is SPI");
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_MOSI_GPIO=%d",CONFIG_MOSI_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_SCLK_GPIO=%d",CONFIG_SCLK_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_CS_GPIO=%d",CONFIG_CS_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_DC_GPIO=%d",CONFIG_DC_GPIO);
			ESP_LOGW(TAG_MIKE_APP, "~~~ CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
			spi_master_init(&ssd1306dev, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO, CONFIG_CS_GPIO, CONFIG_DC_GPIO, CONFIG_RESET_GPIO);
		#endif // CONFIG_SPI_INTERFACE
	  
		#if CONFIG_FLIP
			ssd1306dev._flip = true;
			ESP_LOGW(TAG_MIKE_APP, "~~~ Flip upside down");
		#endif
	  
		#if CONFIG_SSD1306_128x64
			ESP_LOGW(TAG_MIKE_APP, "~~~ Panel is 128x64");
			err = ssd1306_init(&ssd1306dev, 128, 64);
			if(err != ESP_OK) {
				ESP_LOGW(TAG_MIKE_APP, "~~~ i2c_master_init() failed!");
				return err;
			}
		#endif // CONFIG_SSD1306_128x64
	  
		#if CONFIG_SSD1306_128x32
			ESP_LOGW(TAG_MIKE_APP, "~~~ Panel is 128x32");
			err = ssd1306_init(&ssd1306dev, 128, 32);
			if(err != ESP_OK) {
				ESP_LOGW(TAG_MIKE_APP, "~~~ i2c_master_init() failed!");
				return err;
			}
		#endif // CONFIG_SSD1306_128x32
	  
		ESP_LOGW(TAG_MIKE_APP, "~~~ _address=%x, _width=%x, _height=%x, _pages=%x", ssd1306dev._address, ssd1306dev._width, ssd1306dev._height, ssd1306dev._pages);
		
		if(std::is_same<decltype(ssd1306dev), SSD1306_t>::value) {
			ESP_LOGW(TAG_MIKE_APP, "~~~ OK!");
		} else {
			ESP_LOGE(TAG_MIKE_APP, "~~~ FAIL!");
		}
		
		if(err != ERR_OK) {
			get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
			get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
			get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
			return err;
		}
	  
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, 0, "     MATTER     ", 16, false);
		ssd1306_display_text(&ssd1306dev, 1, "   OVER THREAD  ", 16, false);

		//ssd1306_display_text_x2(&ssd1306dev, 0, " THREAD ", 8, true);

		/*
		ssd1306_clear_screen(&ssd1306dev, false);

		char buf[32];
    snprintf(buf, sizeof(buf), "Plug %d: %s", plug_num, state ? "ON " : "OFF");
    ssd1306_display_text(&ssd1306dev, (plug_num-1)*1, buf, strlen(buf), false);

		int center, top, bottom;
		char lineChar[20];
	  
		#if CONFIG_SSD1306_128x64
			top = 2;
			center = 3;
			bottom = 8;
			ssd1306_display_text(&ssd1306dev, 0, "SSD1306 128x64", 14, false);
			ssd1306_display_text(&ssd1306dev, 1, "ABCDEFGHIJKLMNOP", 16, false);
			ssd1306_display_text(&ssd1306dev, 2, "abcdefghijklmnop",16, false);
			ssd1306_display_text(&ssd1306dev, 3, "Hello World!!", 13, false);
			//ssd1306_clear_line(&ssd1306dev, 4, true);
			//ssd1306_clear_line(&ssd1306dev, 5, true);
			//ssd1306_clear_line(&ssd1306dev, 6, true);
			//ssd1306_clear_line(&ssd1306dev, 7, true);
			ssd1306_display_text(&ssd1306dev, 4, "SSD1306 128x64", 14, true);
			ssd1306_display_text(&ssd1306dev, 5, "ABCDEFGHIJKLMNOP", 16, true);
			ssd1306_display_text(&ssd1306dev, 6, "abcdefghijklmnop",16, true);
			ssd1306_display_text(&ssd1306dev, 7, "Hello World!!", 13, true);
		#endif // CONFIG_SSD1306_128x64
	  
		#if CONFIG_SSD1306_128x32
			top = 1;
			center = 1;
			bottom = 4;
			ssd1306_display_text(&ssd1306dev, 0, "SSD1306 128x32", 14, false);
			ssd1306_display_text(&ssd1306dev, 1, "Hello World!!", 13, false);
			//ssd1306_clear_line(&ssd1306dev, 2, true);
			//ssd1306_clear_line(&ssd1306dev, 3, true);
			ssd1306_display_text(&ssd1306dev, 2, "SSD1306 128x32", 14, true);
			ssd1306_display_text(&ssd1306dev, 3, "Hello World!!", 13, true);
		#endif // CONFIG_SSD1306_128x32
		vTaskDelay(3000 / portTICK_PERIOD_MS);
		
		//-- Display Count Down
		uint8_t image[24];
		memset(image, 0, sizeof(image));
		ssd1306_display_image(&ssd1306dev, top, (6*8-1), image, sizeof(image));
		ssd1306_display_image(&ssd1306dev, top+1, (6*8-1), image, sizeof(image));
		ssd1306_display_image(&ssd1306dev, top+2, (6*8-1), image, sizeof(image));
		for(int font=0x39;font>0x30;font--) {
			memset(image, 0, sizeof(image));
			ssd1306_display_image(&ssd1306dev, top+1, (7*8-1), image, 8);
			memcpy(image, font8x8_basic_tr[font], 8);
			if (ssd1306dev._flip) ssd1306_flip(image, 8);
			ssd1306_display_image(&ssd1306dev, top+1, (7*8-1), image, 8);
			vTaskDelay(1000 / portTICK_PERIOD_MS);
		}
		
		//-- Scroll Up
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, 0, "---Scroll  UP---", 16, true);
		//ssd1306_software_scroll(&ssd1306dev, 7, 1);
		ssd1306_software_scroll(&ssd1306dev, (ssd1306dev._pages - 1), 1);
		for (int line=0;line<bottom+10;line++) {
			lineChar[0] = 0x01;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&ssd1306dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);
		
		//-- Scroll Down
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, 0, "--Scroll  DOWN--", 16, true);
		//ssd1306_software_scroll(&ssd1306dev, 1, 7);
		ssd1306_software_scroll(&ssd1306dev, 1, (ssd1306dev._pages - 1) );
		for (int line=0;line<bottom+10;line++) {
			lineChar[0] = 0x02;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&ssd1306dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	  
		//-- Page Down
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, 0, "---Page	DOWN---", 16, true);
		ssd1306_software_scroll(&ssd1306dev, 1, (ssd1306dev._pages-1) );
		for (int line=0;line<bottom+10;line++) {
			//if ( (line % 7) == 0) ssd1306_scroll_clear(&ssd1306dev);
			if ( (line % (ssd1306dev._pages-1)) == 0) ssd1306_scroll_clear(&ssd1306dev);
			lineChar[0] = 0x02;
			sprintf(&lineChar[1], " Line %02d", line);
			ssd1306_scroll_text(&ssd1306dev, lineChar, strlen(lineChar), false);
			vTaskDelay(500 / portTICK_PERIOD_MS);
		}
		vTaskDelay(3000 / portTICK_PERIOD_MS);
	  
		//-- Horizontal Scroll
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, center, "Horizontal", 10, false);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_RIGHT);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_LEFT);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_STOP);
		
		//-- Vertical Scroll
		ssd1306_clear_screen(&ssd1306dev, false);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, center, "Vertical", 8, false);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_DOWN);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_UP);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
		ssd1306_hardware_scroll(&ssd1306dev, SCROLL_STOP);
		
		//-- Invert
		ssd1306_clear_screen(&ssd1306dev, true);
		ssd1306_contrast(&ssd1306dev, 0xff);
		ssd1306_display_text(&ssd1306dev, center, "  Good Bye!!", 12, true);
		vTaskDelay(5000 / portTICK_PERIOD_MS);
	  
	  
		//-- Fade Out
		ssd1306_fadeout(&ssd1306dev);
		
		#if 0
			//-- Fade Out
			for(int contrast=0xff;contrast>0;contrast=contrast-0x20) {
				ssd1306_contrast(&ssd1306dev, contrast);
				vTaskDelay(40);
			}
		#endif
	  
		//-- Restart module
		esp_restart();
		*/
	  
		return err;
	}

	void show_plug_status(uint8_t plug_num, bool state)
	{
		//-- if not initialized
		if(!ssd1306_initialized) {
			ESP_LOGW(TAG_MIKE_APP, "~~~ Unable to show plug status: SSD1306 is not initialized!");
			return;
		}
	  
		//-- if wrong plug number
		if(plug_num > CONFIG_NUM_VIRTUAL_PLUGS) {
			ESP_LOGW(TAG_MIKE_APP, "~~~ Unable to show plug status: Wrong plug number (%d)!", plug_num);
			return;
		}
		
		//ESP_LOGW(TAG_MIKE_APP, "~~~ Plug %d state (ESP NVS): %s", plug_num, state ? "ON " : "OFF");
		
	  show_square(plug_num - 1, state);
	}
	
	void show_square(int idx, bool state)
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

	  //ESP_LOGW(TAG_MIKE_APP, "~~~ !SQUARE: TOP=%d, LEFT=%d", top, left);

		memset(image, 0, sizeof(image));
		if(state) {
			ssd1306_invert(image, 24);
		}
	  ssd1306_display_image(&ssd1306dev, top,   left, image, sizeof(image));
		ssd1306_display_image(&ssd1306dev, top+1, left, image, sizeof(image));
		ssd1306_display_image(&ssd1306dev, top+2, left, image, sizeof(image));

		int font = 0x30 + idx + 1;
		memcpy(image, font8x8_basic_tr[font], 8);
		if(state) {
			ssd1306_invert(image, 8);
		}
		ssd1306_display_image(&ssd1306dev, top+1, left+8, image, 8);
	}
#endif
