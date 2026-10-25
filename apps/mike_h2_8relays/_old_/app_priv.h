#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "plugin_device_structure.h"


/************************************************
 *                                              *
 *   GPIOs USED (relays):  0,1,2,3,5,10,11,12   *
 *   GPIOs USED (ssd1306): 13,14                *
 *                                              *
 ************************************************/

#define TAG_MIKE_APP                           "MIKE 8RELAYS H2"
#define TAG_EMPTY                              ""
#define CONFIG_NUM_VIRTUAL_PLUGS               8
#define RELAY_INVERSE_LEVEL                    true

//-- SSD1306 LED Display
#define CONFIG_SSD1306_ENABLED                 0


#define LIVE_BLINK_TIME_MS                     0
#define UPDATE_ATTRIBUTES_TIME_MS              10000
#define USE_INTERNAL_TEMPERATURE               0
#define USE_INTERNAL_VOLTAGE                   false
#define USE_INTERNAL_UPTIME                    0
#define USE_TIME_DRIVER                        false
#define ADD_CUSTOM_CLUSTERS                    false
#define SHOW_DEVICE_LOG                        false
#define UART_MONITOR_DEBUG                     false

#if USE_TIME_DRIVER
  #include "driver_time.h"
#endif

//-- Reboot button handlers
#define CONFIG_REBOOT_BUTTON_ENABLED           true
#if CONFIG_REBOOT_BUTTON_ENABLED
  //-- Single-click: toggle SSD1306
  //-- Double-click: toggle LED Indicator
  #define REBOOT_BUTTON_GPIO                   9
  //-- Hold to reboot
  #define REBOOT_HOLD_TIME_MS                  3000
  //-- Hold to factory reset
  #define FACTORY_RESET_HOLD_TIME_MS           10000

  //--> uses for CommissioningWindowOpen
  #include <app/server/CommissioningWindowManager.h>
  #include <app/server/Server.h>
  //-- Window opens for NNN seconds
  #define CONFIG_COMMISSIONING_WINDOW_OPEN     300
  #define CONFIG_COMMISSIONING_WINDOW_PRIORITY 6
  //<-- uses for CommissioningWindowOpen
  
  #include "driver_reboot_button.h"

  //-- task priorities
  #define CONFIG_REBOOT_BUTTON_TASK_PRIORITY   4
  //=== ДОБАВЛЕНО: параметры детекции двойного нажатия кнопки Reboot ===
  //-- интервал между первым и вторым нажатием для распознавания двойного клика (мс)
  #define DOUBLE_CLICK_INTERVAL_MS             400
  //-- максимальная длительность короткого нажатия (мс) — иначе это не "клик"
  #define SHORT_CLICK_MAX_MS                   350
  //=== /ДОБАВЛЕНО ===
#endif


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
#endif

#if SHOW_DEVICE_LOG
	//-- what part of tree to show
	#define SHOW_DEVICE_LOG_ONE_CLUSTER   1
	#define SHOW_DEVICE_LOG_CLUSTERS      true
	#define SHOW_DEVICE_LOG_ATTRIBUTES    true
	#define SHOW_DEVICE_LOG_COMMANDS      true
#endif


#include "driver_chip.h"
#include "driver_relay.h"
#include "driver_reboot_button.h"
#include "driver_uptime.h"

#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306.h"
  #include "ssd1306_i2cdev.h"
  /**********************************************************
   *
   *  This driver initializes the I2C bus
   *  on port CONFIG_SSD1306_I2C_PORT;
   *  therefore, any sensors using the I2C interface
   *  must be connected to the same GPIO pins (SDA and SCL)
   *  if they use this port.
   *
   **********************************************************/
  #define CONFIG_SSD1306_SDA_GPIO              14
  #define CONFIG_SSD1306_SCL_GPIO              13
  #define CONFIG_SSD1306_RESET_GPIO            -1
  #define CONFIG_SSD1306_I2C_ADDRESS           0x3C
  #define CONFIG_SSD1306_I2C_PORT              I2C_NUM_0
  
  extern bool ssd1306_initialized;
  extern SSD1306_t ssd1306dev;
  
  //=== ДОБАВЛЕНО: флаг состояния дисплея и таймер авто-выключения ===
  extern bool ssd1306_display_enabled;       // true = экран включён
  extern uint64_t ssd1306_last_on_time_ms;   // момент последнего включения (ms)
  #define SSD1306_AUTO_OFF_TIMEOUT_MS          (5 * 60 * 1000)  // 5 минут
  // Рисует во второй строке экрана (y_pos = 1) оставшееся время в формате "MM:SS".
  //extern void ssd1306_update_timeout_line(void);
  //=== /ДОБАВЛЕНО ===
#endif


//-- LED Indicator
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  extern led_indicator_handle_t led_handle;
#endif

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

extern void update_temperature_value(uint16_t endpoint_id, int16_t temperature_value);
//extern void update_uptime_value(uint16_t endpoint_id, uint32_t uptime_seconds);
extern void update_time_values(uint32_t endpoint_id);

extern esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);

extern uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay);
