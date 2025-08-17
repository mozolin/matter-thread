#pragma once

/************************************************
 *                                              *
 *   GPIOs USED (relays):  0,1,2,3,5,10,11,12   *
 *   GPIOs USED (ssd1306): 13,14                *
 *                                              *
 ************************************************/

#define TAG_MIKE_APP               "Mike's App"
#define TAG_EMPTY                  ""
#define CONFIG_NUM_VIRTUAL_PLUGS   8
#define RELAY_INVERSE_LEVEL        true
#define USE_SSD1306_DRIVER         true
#define LIVE_BLINK_TIME_MS         0
#define USE_INTERNAL_TEMPERATURE   true
#define USE_INTERNAL_VOLTAGE       false
#define USE_TIME_DRIVER            false
#define USE_OPENTHREAD_DRIVER      false
#define ADD_CUSTOM_CLUSTERS        true


#if ADD_CUSTOM_CLUSTERS
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//-- !!                                                           !!
	//-- !! For custom clusters, use ID from the manufacturer's range !!
	//-- !!            (can be in 0xFC00-0xFFFE)                      !!
	//-- !!                                                           !!
	//-- !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//-- Cluster for chip temperature (int16_t)
	#define CLUSTER_ID_CHIP_TEMP          0xFC00
	//-- Cluster for uptime (uint32_t)
	#define CLUSTER_ID_UPTIME             0xFC01
	//-- Custom endpoint for chip temperature & uptime
	#define CUSTOM_ENDPOINT_ID            1
	
	//-- what part of tree to show
	#define SHOW_CUSTOM_ENDPOINT_ID_ONLY  true
	#define SHOW_DEVICE_LOG_CLUSTERS      true
	#define SHOW_DEVICE_LOG_ATTRIBUTES    true
	#define SHOW_DEVICE_LOG_COMMANDS      true

#endif


#include "driver_chip.h"
#include "driver_led_indicator.h"
#include "driver_relay.h"
#include "driver_reset_button.h"
#include "driver_ssd1306.h"
#include "driver_thread.h"
#include "driver_time.h"

#if ADD_CUSTOM_CLUSTERS
	#include "plugin_custom_cluster.h"
#endif
#include "plugin_device_structure.h"

//-- Plugs (Relays)
//-- List of all relays
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

#if USE_SSD1306_DRIVER
  #include "driver_ssd1306.h"
  #include "ssd1306.h"
  
  //-- SSD1306
  #define CONFIG_SCL_GPIO 13
  #define CONFIG_SDA_GPIO 14
  #define CONFIG_RESET_GPIO -1
  extern bool ssd1306_initialized;
  extern SSD1306_t ssd1306dev;
#endif


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

extern uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay);
