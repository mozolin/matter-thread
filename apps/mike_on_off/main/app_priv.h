
#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"


#define APP_USE_LED_STRIP false
#define APP_USE_LED_INDICATOR true

//-- GPIO assignment (RGB LED on board)
#define LED_STRIP_BLINK_GPIO  8
//-- Numbers of the LED in the strip
#define LED_STRIP_LED_NUMBERS 1
//-- 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)



#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include "esp_openthread_types.h"
#endif

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

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
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
#endif


#if APP_USE_LED_STRIP
  #include "led_strip.h"
  led_strip_handle_t configure_led(void);
#endif

/*
#if APP_USE_LED_INDICATOR
  #include "led_indicator.h"
  led_indicator_handle_t configure_indicator(void);
#endif
*/
