
#include "driver_ssd1306.h"
#include <app_priv.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/i2c_master.h"
#include "ssd1306.h"
//#include "font8x8_basic.h"

#include <type_traits>

#define I2C_ADDRESS 0x3C
#define I2C_MASTER_FREQ_HZ 400000 // I2C clock of SSD1306 can run at 400 kHz max.
#define I2C_TICKS_TO_WAIT 100	  // Maximum ticks to wait before issuing a timeout.


#if USE_SSD1306_DRIVER
  const uint8_t degree_symbol[] = {
    0b00110000,
    0b01001000,
    0b01001000,
    0b00110000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
  };

  esp_err_t ssd1306_i2c_init(void)
  {
    esp_err_t err = ESP_OK;
    
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ INTERFACE is i2c");
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ CONFIG_SDA_GPIO=%d",CONFIG_SDA_GPIO);
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ CONFIG_SCL_GPIO=%d",CONFIG_SCL_GPIO);
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ CONFIG_RESET_GPIO=%d",CONFIG_RESET_GPIO);
    
    // 1. Configure I2C bus with your desired GPIO pins
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_NUM_0,           // Choose I2C port (0 or 1)
        .sda_io_num = (gpio_num_t)CONFIG_SDA_GPIO,       // <-- SET YOUR SDA GPIO HERE
        .scl_io_num = (gpio_num_t)CONFIG_SCL_GPIO,       // <-- SET YOUR SCL GPIO HERE
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags {
            .enable_internal_pullup = true,  // Enable internal pull-ups
        },
    };

    // 2. Create the I2C master bus
    i2c_master_bus_handle_t i2c_bus_handle;
    err = i2c_new_master_bus(&i2c_mst_config, &i2c_bus_handle);
    if(err != ESP_OK) {
      ESP_LOGW(TAG_MULTI_SENSOR, "~~~ i2c_master_init() failed!");
      return err;
    }
    i2c_device_config_t dev_cfg = {
    		.dev_addr_length = I2C_ADDR_BIT_LEN_7,
    		.device_address = I2C_ADDRESS,
    		.scl_speed_hz = I2C_MASTER_FREQ_HZ,
    	};
    	i2c_master_dev_handle_t i2c_dev_handle;
    	//ESP_ERROR_CHECK(i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle));
    	err = i2c_master_bus_add_device(i2c_bus_handle, &dev_cfg, &i2c_dev_handle);
    	if(err != ESP_OK) {
    		ESP_LOGW(TAG_MULTI_SENSOR, "~~~ i2c_master_bus_add_device() failed! %d (%s)", err, esp_err_to_name(err));
    		return err;
    }

    // 3. Initialize SSD1306 with default config
    /*
    ssd1306_config_t ssd1306_config = SSD1306_128x64_CONFIG_DEFAULT;
    ssd1306_handle_t ssd1306_handle;
    */
    err = ssd1306_init(i2c_bus_handle, &ssd1306_config, &ssd1306_handle);
    if(err != ESP_OK) {
      ESP_LOGW(TAG_MULTI_SENSOR, "~~~ ssd1306_init() failed!");
      return err;
    }
    if(ssd1306_handle == NULL) {
      ESP_LOGE(TAG_MULTI_SENSOR, "ssd1306 handle init failed");
      assert(ssd1306_handle);
    }
    
    if(err != ERR_OK) {
      /*
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      */
      return err;
    }
    
    ssd1306_clear_display(ssd1306_handle, false);
    
    
    return err;
  }

  void ssd1306_show_title()
  {
    //-- if not initialized
    if(!ssd1306_initialized) {
      return;
    }

    ssd1306_clear_display(ssd1306_handle, false);
    ssd1306_set_contrast(ssd1306_handle, 0xff);
    ssd1306_display_text(ssd1306_handle, 0, " MATTER/THREAD  ", false);
  }

  void ssd1306_draw_degree_symbol(uint8_t x, uint8_t y)
  {
    ssd1306_display_bitmap(ssd1306_handle, x, y, (uint8_t*)degree_symbol, 8, 8, false);
  }

	void ssd1306_show_sensor_data(uint8_t y_pos, float temp, float hum, float pres, float gas)
	{
		//-- if not initialized
    if(!ssd1306_initialized) {
		  return;
		}

    char buf[32];
    
    snprintf(buf, sizeof(buf), "%.1f", temp/100.0f);
    ssd1306_display_text(ssd1306_handle, y_pos, buf, false);

    if(hum > 0) {
    	char buf2[32];
    	snprintf(buf2, sizeof(buf2), " %.1f", hum/100.0f);
    	strcat(buf, buf2);
    }
    
    if(pres > 0) {
    	char buf3[32];
    	snprintf(buf3, sizeof(buf3), " %.0f", pres);
    	strcat(buf, buf3);
    }
    
    if(gas > 0) {
    	char buf4[32];
    	snprintf(buf4, sizeof(buf4), " %.0f", gas);
    	strcat(buf, buf4);
    }

    ssd1306_display_text(ssd1306_handle, y_pos, buf, false);
	}

#endif

