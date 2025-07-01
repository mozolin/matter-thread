
#include <stdlib.h>
#include <string.h>

#include "driver/gpio.h"
#include "soc/gpio_num.h"
#include <esp_log.h>
#include <bsp/esp-bsp.h>
#include "bsp/esp_bsp_devkit.h"
#include "support/CodeUtils.h"

#include <esp_matter.h>

#include <app_priv.h>
#include <button_gpio.h>
#include <iot_button.h>

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";

static esp_err_t app_driver_update_gpio_value(gpio_num_t pin, bool value)
{
  esp_err_t err = ESP_OK;

  err = gpio_set_level(pin, value);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "Failed to set GPIO level");
    return ESP_FAIL;
  } else {
    ESP_LOGI(TAG, "GPIO pin : %d set to %d", pin, value);
  }
  return err;
}

esp_err_t app_driver_plugin_unit_init(const gpio_plug* plug)
{
  esp_err_t err = ESP_OK;

  gpio_reset_pin(plug->GPIO_PIN_VALUE);

  err = gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "Unable to set GPIO OUTPUT mode");
    return ESP_FAIL;
  }

  err = gpio_set_level(plug->GPIO_PIN_VALUE, 0);
  if(err != ESP_OK) {
    ESP_LOGI(TAG, "Unable to set GPIO level");
  }
  return err;
}

// Return GPIO pin from plug-endpoint mapping list
gpio_num_t get_gpio(uint16_t endpoint_id)
{
  gpio_num_t gpio_pin = GPIO_NUM_NC;
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].endpoint_id == endpoint_id) {
      gpio_pin = plugin_unit_list[i].plug;
    }
  }
  return gpio_pin;
}


esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  esp_err_t err = ESP_OK;

  if(cluster_id == OnOff::Id) {
    if(attribute_id == OnOff::Attributes::OnOff::Id) {
      gpio_num_t gpio_pin = get_gpio(endpoint_id);
      if(gpio_pin != GPIO_NUM_NC) {
        err = app_driver_update_gpio_value(gpio_pin, val->val.b);
      } else {
        ESP_LOGE(TAG, "GPIO pin mapping for endpoint_id: %d not found", endpoint_id);
        return ESP_FAIL;
      }
    }
  }
  return err;
}

app_driver_handle_t app_driver_button_init(gpio_num_t * reset_gpio)
{
  VerifyOrReturnValue((reset_gpio), (app_driver_handle_t)NULL, ESP_LOGE(TAG, "reset_gpio cannot be NULL"));
	#ifdef CONFIG_USER_BUTTON
    *reset_gpio = (gpio_num_t)CONFIG_USER_BUTTON_GPIO;
	#elif CONFIG_BSP_BUTTONS_NUM >= 1
    *reset_gpio = (gpio_num_t)BSP_BUTTON_1_IO;
	#else
    *reset_gpio = gpio_num_t::GPIO_NUM_NC;
    return (app_driver_handle_t)NULL;
	#endif
  ESP_LOGI(TAG, "Initializing reset button with gpio pin %d ...", (int)*reset_gpio);

  // Make sure button's IO pin isn't assigned to a plug's IO pin
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == *reset_gpio) {
      ESP_LOGE(TAG, "Button's gpio pin %d is already configured for plug %d", *reset_gpio, i);
      *reset_gpio = gpio_num_t::GPIO_NUM_NC;
      return (app_driver_handle_t)NULL;
    }
  }

  /* Initialize button */
  app_driver_handle_t reset_handle = NULL;
	#ifdef CONFIG_USER_BUTTON
    const button_config_t btn_cfg = {0};
    const button_gpio_config_t btn_gpio_cfg = {
      .gpio_num = CONFIG_USER_BUTTON_GPIO,
      .active_level = CONFIG_USER_BUTTON_LEVEL,
    };

    if(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &reset_handle) != ESP_OK) {
      ESP_LOGE(TAG, "Failed to create button device");
      return NULL;
    }
	#else
    button_handle_t bsp_buttons[BSP_BUTTON_NUM];
    int btn_cnt = 0;// will contain # of buttons that were created by BSP
    bsp_iot_button_create(bsp_buttons, &btn_cnt, BSP_BUTTON_NUM);
    if(btn_cnt >= 1) {
      // return handle to dev board's 1st built-in button
      reset_handle = (app_driver_handle_t)bsp_buttons[0];
    } else {
      ESP_LOGE(TAG, "bsp_iot_button_create() didn't return a usable button count: %d", btn_cnt);
    }
	#endif

  if (!reset_handle) {
      *reset_gpio = gpio_num_t::GPIO_NUM_NC;
  }
  return reset_handle;
}



/********************
 *                  *
 *   RESET BUTTON   *
 *                  *
 ********************/
//#include "esp_matter_core.h"
void reboot_button_task(void *pvParameter)
{
  uint32_t button_pressed_time = 0;
  bool button_pressed = false;

  //-- Configure Reset GPIO
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << (gpio_num_t)REBOOT_BUTTON_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  while(1) {
    //-- Reset button pressed
    if(gpio_get_level((gpio_num_t)REBOOT_BUTTON_GPIO) == 0) {
      if(!button_pressed) {
        button_pressed_time = esp_log_timestamp();
        button_pressed = true;
        //ESP_LOGI("RebootButton", "Button pressed");
      }
    } else {
      //-- Reset button released
      if(button_pressed) {
        //ESP_LOGI("RebootButton", "Button released");

        uint32_t pressed_duration = esp_log_timestamp() - button_pressed_time;
        
        if(pressed_duration >= FACTORY_RESET_HOLD_TIME_MS) {
          ESP_LOGE("RebootButton", "\nFactory reset triggered\n");
          esp_matter::factory_reset();
          esp_restart();
        } else if (pressed_duration >= REBOOT_HOLD_TIME_MS) {
          ESP_LOGW("RebootButton", "\nReboot triggered\n");
          esp_restart();
        }

      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}



/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
//-- Just turn on the yellow color
const blink_step_t yellow_on[] = {
  //-- Set color to yellow by R:255 G:255 B:0
  //{LED_BLINK_RGB, SET_RGB(255, 255, 0), 0},
  {LED_BLINK_RGB, SET_RGB(128, 128, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 15000},
  {LED_BLINK_STOP, 0, 0},
};

//-- Just turn on the orange color
const blink_step_t orange_on[] = {
  //-- Set color to orange by R:255 G:128 B:0
  {LED_BLINK_RGB, SET_RGB(128, 64, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 15000},
  {LED_BLINK_STOP, 0, 0},
};


//-- Blinking twice times in red
const blink_step_t double_red_blink[] = {
  //-- Set color to red by R:255 G:0 B:0
  {LED_BLINK_RGB, SET_RGB(128, 0, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_STOP, 0, 0},
};

//-- Blinking three times in green
const blink_step_t triple_green_blink[] = {
  //-- Set color to green by R:0 G:255 B:0
  {LED_BLINK_RGB, SET_RGB(0, 128, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_STOP, 0, 0},
};

//-- Blinking long in blue
const blink_step_t blue_long_blink[] = {
  //-- Set color to blue by R:0 G:0 B:255
  {LED_BLINK_RGB, SET_RGB(0, 0, 128), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 300},
  {LED_BLINK_HOLD, LED_STATE_OFF, 300},
  {LED_BLINK_LOOP, 0, 0},
};


//-- Slow breathing in white
const blink_step_t breath_white_slow_blink[] = {
  //-- Set Color to white and brightness to zero by H:0 S:0 V:0
  {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
  {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
  {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
  {LED_BLINK_LOOP, 0, 0},
};

//-- Fast breathing in white
const blink_step_t breath_white_fast_blink[] = {
  //-- Set Color to white and brightness to zero by H:0 S:0 V:0
  {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
  {LED_BLINK_BREATHE, LED_STATE_ON, 500},
  {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
  {LED_BLINK_LOOP, 0, 0},
};

//-- Breathing in green
const blink_step_t breath_blue_blink[] = {
  //-- Set Color to blue and brightness to zero by H:240 S:255 V:0
  {LED_BLINK_HSV, SET_HSV(240, MAX_SATURATION, 0), 0},
  {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
  {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
  {LED_BLINK_LOOP, 0, 0},
};

//-- Color gradient by HSV
const blink_step_t color_hsv_ring_blink[] = {
  //-- Set Color to RED
  {LED_BLINK_HSV, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
  {LED_BLINK_HSV_RING, SET_HSV(240, MAX_SATURATION, 127), 2000},
  {LED_BLINK_HSV_RING, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
  {LED_BLINK_LOOP, 0, 0},
};

//-- Color gradient by RGB
const blink_step_t color_rgb_ring_blink[] = {
  //-- Set Color to Green
  {LED_BLINK_RGB, SET_RGB(0, 255, 0), 0},
  {LED_BLINK_RGB_RING, SET_RGB(255, 0, 255), 2000},
  {LED_BLINK_RGB_RING, SET_RGB(0, 255, 0), 2000},
  {LED_BLINK_LOOP, 0, 0},
};

#if LED_NUMBERS > 1
  //-- Flowing lights. Insert the index:MAX_INDEX to control all the strips
  const blink_step_t flowing_blink[] = {
    {LED_BLINK_HSV, SET_IHSV(MAX_INDEX, 0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
    {LED_BLINK_HSV_RING, SET_IHSV(MAX_INDEX, MAX_HUE, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
    {LED_BLINK_LOOP, 0, 0},
  };
#endif


led_indicator_handle_t configure_indicator(void)
{
	// LED strip general initialization, according to your led board design
  led_strip_config_t strip_config = {
    .strip_gpio_num = LED_BLINK_GPIO,   // The GPIO that connected to the LED strip's data line
    .max_leds = LED_NUMBERS,        // The number of LEDs in the strip,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
    .led_model = LED_MODEL_WS2812,            // LED strip model
    {
      .invert_out = false,                    // whether to invert the output signal
    },
  };
  
  // LED strip backend configuration: RMT
  led_strip_rmt_config_t rmt_config = {
		//#if ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(5, 0, 0)
    //  .rmt_channel = 0,
		//#else
      .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
      .resolution_hz = LED_RMT_RES_HZ, // RMT counter clock frequency
      .mem_block_symbols = 0,                // How many RMT symbols can one RMT channel hold at one time. Set to 0 will fallback to use the default size.
      {
      	.with_dma = false,                   // DMA feature is available on ESP target like ESP32-S3
      },
		//#endif
  };
  
  led_indicator_strips_config_t strips_config = {
    .led_strip_cfg = strip_config,
    .led_strip_driver = LED_STRIP_RMT,
    .led_strip_rmt_cfg = rmt_config,
  };
  
  const led_indicator_config_t config = {
    .mode = LED_STRIPS_MODE,
    .led_indicator_strips_config = &strips_config,
    .blink_lists = led_mode,
    .blink_list_num = BLINK_MAX,
  };
  
  led_indicator_handle_t led_handle = NULL;
  led_handle = led_indicator_create(&config);
  assert(led_handle != NULL);

  return led_handle;
}
