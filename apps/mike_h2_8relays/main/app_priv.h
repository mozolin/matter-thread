#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>


#define CONFIG_MOCK_SENSORS_BEHAVIOR           false


/************************************************
 *                                              *
 *   GPIOs USED (relays):  0,1,2,3,5,10,11,12   *
 *   GPIOs USED (ssd1306): 13,14                *
 *                                              *
 ************************************************/

#define CONFIG_NUM_VIRTUAL_PLUGS               8
#define CONFIG_RELAY_INVERSE_LEVEL             true

/****************************************
 *                                      *
 *   I2C BUSES (0 and 1 for ESP32-H2)   *
 *                                      *
 ****************************************/
#include "i2c_adv.h"


//-- Reboot button handlers
#if CONFIG_REBOOT_BUTTON_ENABLED
  #include <app/server/CommissioningWindowManager.h>
  #include <app/server/Server.h>
  #include "driver_reboot_button.h"
#endif


//-- LED Indicator
#include "driver_led_esp32h2.h"
#if CONFIG_USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
#endif

//-- Boot Switch
#if CONFIG_BOOT_SWITCH_ENABLED
  #include "driver_boot_switch.h"
#endif

#include "driver_relay.h"


#if CONFIG_ESP_EMULATOR_MODE
  #define CONFIG_SENSOR_POLL_PERIOD_MS         15000
#else
  #define CONFIG_SENSOR_POLL_PERIOD_MS         5000
#endif


#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306_common.h"
  #include "main_driver_ssd1306.h"
  #include "ssd1306_i2cdev.h"
#endif

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

#if CONFIG_SSD1306_ENABLED
  #ifdef CONFIG_SSD1306_APP_RUN_CB
    //-- callback function specific to this application
    //-- (can be used within the SSD1306 common driver)
    extern void ssd1306_app_run_cb(void);
  #endif
#endif

esp_err_t app_attribute_update_main_cb(esp_matter::attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);
esp_err_t app_driver_attribute_update_main(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);
