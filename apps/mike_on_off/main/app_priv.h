#pragma once

#include "driver_reset_button.h"
#include "driver_led_indicator.h"
#include "driver_relay.h"

//-- Plugs
#define CONFIG_NUM_VIRTUAL_PLUGS     8
/*
#define CONFIG_GPIO_PLUG_1           3
#define CONFIG_GPIO_PLUG_2           5
#define CONFIG_GPIO_PLUG_3           2
#define CONFIG_GPIO_PLUG_4           1
//-- ??? GPIO4 does not work as output - why so ???
#define CONFIG_GPIO_PLUG_5           0
#define CONFIG_GPIO_PLUG_6           12
#define CONFIG_GPIO_PLUG_7           11
#define CONFIG_GPIO_PLUG_8           10
*/

//-- List of all relays (expandable)
const std::vector<RelayConfig> relays = {
  {1, GPIO_NUM_3},
	{2, GPIO_NUM_5},
	{3, GPIO_NUM_2},
	{4, GPIO_NUM_1},
	{5, GPIO_NUM_0},
	{6, GPIO_NUM_12},
	{7, GPIO_NUM_11},
	{8, GPIO_NUM_10},
};

static const char *TAG = "Mike's App";

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

