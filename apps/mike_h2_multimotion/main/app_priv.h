#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "driver_reboot_button.h"
#include "drivers/sensor_common.h"

#define TAG_MIKE_APP                           "MIKE MULTISENSOR H2"
#define CONFIG_NUM_SENSORS                     4

//-- Sensors configuration
#define CONFIG_HCSR501_ENABLED                 true
#define CONFIG_RCWL0516_ENABLED                true
#define CONFIG_HCSR04_ENABLED                  true
#define CONFIG_SENSOR_POLL_PERIOD_MS           5000

//-- GPIO Configuration
#define CONFIG_HCSR501_PIR_GPIO                1
#define CONFIG_RCWL0516_MICROWAVE_GPIO         2
#define CONFIG_HCSR04_TRIG_GPIO                3
#define CONFIG_HCSR04_ECHO_GPIO                5

//-- Reboot button handlers
#define CONFIG_REBOOT_BUTTON_ENABLED           true
#if CONFIG_REBOOT_BUTTON_ENABLED
  //--> uses for CommissioningWindowOpen
  #include <app/server/CommissioningWindowManager.h>
  #include <app/server/Server.h>
  //-- Window opens for NNN seconds
  #define CONFIG_COMMISSIONING_WINDOW_OPEN     60//300
  #define CONFIG_COMMISSIONING_WINDOW_PRIORITY 6
  //<-- uses for CommissioningWindowOpen
  
  #include "driver_reboot_button.h"
  //-- Single-click: toggle SSD1306
  //-- Double-click: toggle LED Indicator
  #define REBOOT_BUTTON_GPIO                   9
  //-- Hold to reboot
  #define REBOOT_HOLD_TIME_MS                  3000
  //-- Hold to factory reset
  #define FACTORY_RESET_HOLD_TIME_MS           10000
  //=== ДОБАВЛЕНО: параметры детекции двойного нажатия кнопки Reboot ===
  //-- интервал между первым и вторым нажатием для распознавания двойного клика (мс)
  #define DOUBLE_CLICK_INTERVAL_MS             400
  //-- максимальная длительность короткого нажатия (мс) — иначе это не "клик"
  #define SHORT_CLICK_MAX_MS                   350
  //=== /ДОБАВЛЕНО ===
#endif

//-- LED Indicator
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  extern led_indicator_handle_t led_handle;
#endif

//-- task priorities
#define CONFIG_ULTRASONIC_FAST_TASK_PRIORITY   6
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5
#if CONFIG_REBOOT_BUTTON_ENABLED
  #define CONFIG_REBOOT_BUTTON_TASK_PRIORITY   4
#endif

#define LIVE_BLINK_TIME_MS                     0


#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include "esp_openthread_types.h"
#endif

// External declarations
extern sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[CONFIG_NUM_SENSORS];

// Function declarations
sensor_data_t* get_sensor_by_endpoint(uint16_t endpoint_id);
uint16_t get_endpoint_by_sensor_type(sensor_type_t type);

typedef void *app_driver_handle_t;

esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_config);
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);
bool app_driver_read_sensor_state(uint8_t sensor_idx);
uint32_t app_driver_read_ultrasonic_distance(uint8_t sensor_idx);
void sensor_polling_task(void *pvParameters);
esp_err_t app_driver_reset_sensor_by_type(sensor_type_t type);
esp_err_t app_driver_reset_all_sensors(void);
void app_driver_log_sensor_statistics(void);

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
