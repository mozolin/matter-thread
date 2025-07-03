
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
#include "driver_reset_button.h"


/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
#include "driver_led_indicator.h"


/**************
 *            *
 *   RELAYS   *
 *            *
 **************/
#include "driver_relay.h"
