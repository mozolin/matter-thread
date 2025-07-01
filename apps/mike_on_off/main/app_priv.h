
#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "esp_openthread_types.h"

//-- Default attribute values used during initialization
struct gpio_plug {
  gpio_num_t GPIO_PIN_VALUE;
};

struct plugin_endpoint {
  uint16_t endpoint_id;
  gpio_num_t plug;
};

gpio_num_t get_gpio(uint16_t endpoint_id);

extern plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
extern uint16_t configure_plugs;

typedef void *app_driver_handle_t;

esp_err_t app_driver_plugin_unit_init(const gpio_plug* plug);

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);

app_driver_handle_t app_driver_button_init(gpio_num_t * reset_gpio);

#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
{                                                                                   \
  .radio_mode = RADIO_MODE_NATIVE,                                                \
}

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
{                                                                                   \
  .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
}

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
{                                                                                   \
  .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
}


/********************
 *                  *
 *   RESET BUTTON   *
 *                  *
 ********************/
#include "esp_matter_core.h"
#define REBOOT_BUTTON_GPIO 9
//-- Hold for 3 seconds to reboot
#define REBOOT_HOLD_TIME_MS 3000
//-- Hold for 10 seconds to factory reset
#define FACTORY_RESET_HOLD_TIME_MS 10000
extern void reboot_button_task(void *pvParameter);


/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
#include "led_indicator.h"
//-- GPIO assignment
#define LED_BLINK_GPIO  8
//-- Numbers of the LED in the strip
#define LED_NUMBERS 1
//-- 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_RMT_RES_HZ  (10 * 1000 * 1000)
extern led_indicator_handle_t configure_indicator(void);
//-- Define blinking type and priority.
enum {
  BLINK_ON_YELLOW,
  BLINK_ON_ORANGE,
  BLINK_DOUBLE_RED,
  BLINK_TRIPLE_GREEN,
  BLINK_LONG_BLUE,
  BLINK_WHITE_BREATHE_SLOW,
  BLINK_WHITE_BREATHE_FAST,
  BLINK_BLUE_BREATH,
  BLINK_COLOR_HSV_RING,
  BLINK_COLOR_RGB_RING,
  #if LED_NUMBERS > 1
    BLINK_FLOWING,
  #endif
  BLINK_MAX,
};
//-- Just turn on the yellow color
extern const blink_step_t yellow_on[];
//-- Just turn on the orange color
extern const blink_step_t orange_on[];
//-- Blinking twice times in red
extern const blink_step_t double_red_blink[];
//-- Blinking three times in green
extern const blink_step_t triple_green_blink[];
//-- Blinking long in blue
extern const blink_step_t blue_long_blink[];
//-- Slow breathing in white
extern const blink_step_t breath_white_slow_blink[];
//-- Fast breathing in white
extern const blink_step_t breath_white_fast_blink[];
//-- Breathing in green
extern const blink_step_t breath_blue_blink[];
//-- Color gradient by HSV
extern const blink_step_t color_hsv_ring_blink[];
//-- Color gradient by RGB
extern const blink_step_t color_rgb_ring_blink[];
#if LED_NUMBERS > 1
  //-- Flowing lights. Insert the index:MAX_INDEX to control all the strips
  extern const blink_step_t flowing_blink[];
#endif

extern blink_step_t const *led_mode[];
