#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "drivers/sensor_common.h"


//-- Debug options
#define CONFIG_MOCK_SENSORS_BEHAVIOR           false


/***********************
 *                     *
 *   TASK PRIORITIES   *
 *                     *
 ***********************/
#define CONFIG_ULTRASONIC_FAST_TASK_PRIORITY   6
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5


/**********************
 *                    *
 *   USE OF SENSORS   *
 *                    *
 **********************/
#define CONFIG_NUM_SENSORS                     4
//-- Sensors configuration
#define CONFIG_HCSR501_ENABLED                 true
#define CONFIG_RCWL0516_ENABLED                true
#define CONFIG_HCSR04_ENABLED                  true


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



/************************
 *                      *
 *   SENSORS SETTINGS   *
 *                      *
 ************************/
#define CONFIG_HCSR501_PIR_GPIO                1
#define CONFIG_RCWL0516_MICROWAVE_GPIO         2
#define CONFIG_HCSR04_TRIG_GPIO                3
#define CONFIG_HCSR04_ECHO_GPIO                5


#if CONFIG_ESP_EMULATOR_MODE
  #define CONFIG_SENSOR_POLL_PERIOD_MS         15000
#else
  #define CONFIG_SENSOR_POLL_PERIOD_MS         1000
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif


// External declarations
extern sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[CONFIG_NUM_SENSORS];

typedef void *app_driver_handle_t;

esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_config);

/*
bool app_driver_read_sensor_state(uint8_t sensor_idx);
*/
/*
uint32_t app_driver_read_ultrasonic_distance(uint8_t sensor_idx);
*/
void sensor_polling_task(void *pvParameters);

/*
esp_err_t app_driver_reset_sensor_by_type(sensor_type_t type);
*/
/*
esp_err_t app_driver_reset_all_sensors(void);
*/
#if CONFIG_DEBUG_MODE || CONFIG_ESP_EMULATOR_MODE
  void app_driver_log_sensor_statistics(void);
#endif


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

esp_err_t app_attribute_update_main_cb(esp_matter::attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);
esp_err_t app_driver_attribute_update_main(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);
