
#include "driver_led_indicator.h"
#include "led_indicator.h"
#include <app_priv.h>

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

//-- Blinking once in red
const blink_step_t red_once_blink[] = {
  //-- Set color to red by R:128 G:0 B:0
  {LED_BLINK_RGB, SET_RGB(32, 0, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 50},
  {LED_BLINK_HOLD, LED_STATE_OFF, 50},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once in green
const blink_step_t green_once_blink[] = {
  //-- Set color to green by R:0 G:128 B:0
  {LED_BLINK_RGB, SET_RGB(0, 32, 0), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 50},
  {LED_BLINK_HOLD, LED_STATE_OFF, 50},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once in blue
const blink_step_t blue_once_blink[] = {
  //-- Set color to blue by R:0 G:0 B:128
  {LED_BLINK_RGB, SET_RGB(0, 0, 32), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 50},
  {LED_BLINK_HOLD, LED_STATE_OFF, 50},
  //{LED_BLINK_LOOP, 0, 0},
};


//-- Blinking once
const blink_step_t live_once_blink[] = {
  {LED_BLINK_RGB, SET_RGB(32, 0, 32), 0},
  {LED_BLINK_HOLD, LED_STATE_ON, 50},
  {LED_BLINK_HOLD, LED_STATE_OFF, 50},
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
