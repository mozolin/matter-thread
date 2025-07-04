#pragma once

#include "driver_reset_button.h"
#include "driver_led_indicator.h"
#include "driver_relay.h"

//-- Plugs
#define CONFIG_NUM_VIRTUAL_PLUGS     8
#define CONFIG_GPIO_PLUG_1           0
#define CONFIG_GPIO_PLUG_2           1
#define CONFIG_GPIO_PLUG_3           2
#define CONFIG_GPIO_PLUG_4           3
//-- ??? GPIO4 does not work as output - why so ???
#define CONFIG_GPIO_PLUG_5           5
#define CONFIG_GPIO_PLUG_6           10
#define CONFIG_GPIO_PLUG_7           11
#define CONFIG_GPIO_PLUG_8           12

//#define CONFIG_SUBSCRIBE_TO_ON_OFF_SERVER_AFTER_BINDING true

//-- Default attribute values used during initialization
struct gpio_plug {
  gpio_num_t GPIO_PIN_VALUE;
};

struct plugin_endpoint {
  uint16_t endpoint_id;
  gpio_num_t plug;
};

extern plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
extern uint16_t configure_plugs;

typedef void *app_driver_handle_t;

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

extern esp_err_t app_driver_update_gpio_value(gpio_num_t pin, bool value);

extern esp_err_t app_driver_plugin_unit_init(const gpio_plug* plug);

//-- Return GPIO pin from plug-endpoint mapping list
extern gpio_num_t get_gpio(uint16_t endpoint_id);

extern esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);
