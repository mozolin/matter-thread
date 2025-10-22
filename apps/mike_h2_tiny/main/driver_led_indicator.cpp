
#include "driver_led_indicator.h"
#include "led_indicator.h"
//#include <app_priv.h>

/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
//-- Just turn on the yellow color
const blink_step_t yellow_on[] = {
  //-- Set color to yellow
  {LED_BLINK_RGB, SET_RGB(128, 128, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 15000},
  {LED_BLINK_STOP, 0, 0},
};

//-- Just turn on the orange color
const blink_step_t orange_on[] = {
  //-- Set color to orange
  {LED_BLINK_RGB, SET_RGB(128, 64, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 15000},
  {LED_BLINK_STOP, 0, 0},
};


//-- Blinking twice times in red
const blink_step_t double_red_blink[] = {
  //-- Set color to red
  {LED_BLINK_RGB, SET_RGB(128, 0, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_STOP, 0, 0},
};

//-- Blinking three times in green
const blink_step_t triple_green_blink[] = {
  //-- Set color to green
  {LED_BLINK_RGB, SET_RGB(0, 128, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_HOLD, LED_STATE_ON, 500},
  {LED_BLINK_HOLD, LED_STATE_OFF, 500},
  {LED_BLINK_STOP, 0, 0},
};

//-- Blinking once in red
const blink_step_t red_once_blink[] = {
  //-- Set color to red
  {LED_BLINK_RGB, SET_RGB(32, 0, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 40},
  {LED_BLINK_HOLD, LED_STATE_OFF, 40},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once in green
const blink_step_t green_once_blink[] = {
  //-- Set color to green
  //{LED_BLINK_RGB, SET_RGB(0, 32, 0), 0},
  //-- Set color to purple
  {LED_BLINK_RGB, SET_RGB(32, 0, 32), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 40},
  {LED_BLINK_HOLD, LED_STATE_OFF, 40},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once in blue
const blink_step_t blue_once_blink[] = {
  //-- Set color to blue
  {LED_BLINK_RGB, SET_RGB(0, 0, 32), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 40},
  {LED_BLINK_HOLD, LED_STATE_OFF, 40},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once
const blink_step_t live_once_blink[] = {
  {LED_BLINK_RGB, SET_RGB(32, 0, 32), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 40},
  {LED_BLINK_HOLD, LED_STATE_OFF, 40},
  //{LED_BLINK_LOOP, 0, 0},
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
  //-- LED strip general initialization, according to your led board design
  led_strip_config_t strip_config = {
    .strip_gpio_num = LED_BLINK_GPIO,
    .max_leds = LED_NUMBERS,
    .led_pixel_format = LED_PIXEL_FORMAT_GRB,
    .led_model = LED_MODEL_WS2812,
    {
      .invert_out = false,
    },
  };
  
  //-- LED strip backend configuration: RMT
  led_strip_rmt_config_t rmt_config = {
    .clk_src = RMT_CLK_SRC_DEFAULT,
    .resolution_hz = LED_RMT_RES_HZ,
    .mem_block_symbols = 0,
    {
      .with_dma = false,
    },
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

#if LIVE_BLINK_TIME_MS > 0
void init_indicator_task(void *pvParameter)
{
  
  uint32_t live_blink_time = 0;
  
  while(1) {
    //-- blink every "LIVE_BLINK_TIME_MS" milliseconds
    uint32_t blinked_duration = esp_log_timestamp() - live_blink_time;
    if(blinked_duration >= LIVE_BLINK_TIME_MS) {
      get_led_indicator_blink_idx(BLINK_ONCE_LIVE, 60, 0);
      live_blink_time = esp_log_timestamp();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
#endif

uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay)
{
  uint8_t idx = 255;
  
  int size = sizeof(led_mode)/sizeof(led_mode[0]);

  auto item = led_mode[blink_type];
  for(int i=0; i<size; i++) {
    if(led_mode[i] == item) {
      //ESP_LOGW(TAG_MIKE_APP, "~~~ ###!!!@@@ FOUND: %d", i);
      idx = i;

      if(start_delay > 0) {
        led_indicator_start(led_handle, idx);
        //vTaskDelay(pdMS_TO_TICKS(start_delay));
        vTaskDelay(start_delay / portTICK_PERIOD_MS);

        led_indicator_stop(led_handle, idx);
        if(stop_delay > 0) {
          //vTaskDelay(pdMS_TO_TICKS(stop_delay));
          vTaskDelay(stop_delay / portTICK_PERIOD_MS);
        }
      }

      break;
    }
  }

  return idx;
}
