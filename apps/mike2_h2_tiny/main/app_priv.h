#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "plugin_device_structure.h"


//-- Debug options
#define DEBUG_MODE                             false
#define TAG_TINY2                              "MIKE2 TINY H2"

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
#endif

//-- LED Indicator
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  extern led_indicator_handle_t led_handle;
#endif

//-- task priorities
#if CONFIG_REBOOT_BUTTON_ENABLED
  #define CONFIG_REBOOT_BUTTON_TASK_PRIORITY   4
#endif

#if CONFIG_REBOOT_BUTTON_ENABLED
  #include "driver_reboot_button.h"
  
  //=== ДОБАВЛЕНО: параметры детекции двойного нажатия кнопки Reboot ===
  //-- интервал между первым и вторым нажатием для распознавания двойного клика (мс)
  #define DOUBLE_CLICK_INTERVAL_MS             400
  //-- максимальная длительность короткого нажатия (мс) — иначе это не "клик"
  #define SHORT_CLICK_MAX_MS                   350
  //=== /ДОБАВЛЕНО ===
#endif
#define LIVE_BLINK_TIME_MS                     0

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include "esp_openthread_types.h"
#endif

//-- task priorities
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5

extern sensor_endpoint_mapping_t sensor_mapping_list[SENSOR_TYPE_MAX];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[SENSOR_TYPE_MAX];

// Sensor types
typedef enum {
  //SENSOR_TYPE_BME280  = 0,
  //SENSOR_TYPE_BME680  = 1,
  SENSOR_TYPE_TEMP = 0,
  //SENSOR_TYPE_DHT11   = 3,
  //SENSOR_TYPE_MQ135   = 4,
  //SENSOR_TYPE_SHT41   = 5,
  SENSOR_TYPE_MAX,
} sensor_type_t;

// Sensor data structure
typedef struct {
  //sensor_type_t config;
  float last_temperature;
  //float last_humidity;
  //float last_pressure;
  //float last_gas_resistance;  // For BME680
  //float last_co2;             // For MQ135
  //float last_co;              // For MQ135
  //float last_tvoc;            // For MQ135
  //uint64_t last_read_time;
  //uint8_t num_sensors;        // For DS18B20
} sensor_data_t;

// Endpoint-sensor mapping
typedef struct {
  uint16_t endpoint_id;
  sensor_type_t sensor_type;
  gpio_num_t primary_gpio;
  gpio_num_t secondary_gpio;
} sensor_endpoint_mapping_t;



typedef void *app_driver_handle_t;

/** Sensor initialization
 *
 * This API is called to initialize sensors
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_sensor_init();

/** Driver Update
 *
 * This API should be called to update the driver for the attribute being updated.
 * This is usually called from the common `app_attribute_update_cb()`.
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 * @param[in] cluster_id Cluster ID of the attribute.
 * @param[in] attribute_id Attribute ID of the attribute.
 * @param[in] val Pointer to `esp_matter_attr_val_t`. Use appropriate elements as per the value type.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

/** Read sensor data
 *
 * This reads the current values from the sensor
 *
 * @return ESP_OK on success
 */
esp_err_t app_driver_read_sensor_data();

/** Task to poll sensors
 *
 * This task periodically reads sensor values and updates Matter attributes
 */
void sensor_polling_task(void *pvParameters);


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
