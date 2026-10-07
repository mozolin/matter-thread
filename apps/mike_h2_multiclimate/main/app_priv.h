#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include "driver/i2c_master.h"
#include <button_gpio.h>
//#include "driver_led_indicator_v2.h"
#include "driver_reset_button.h"


#define MOCK_SENSORS_BEHAVIOR                true
#define DEBUG_MODE                           false


#define TAG_MULTI_CLIMATE                    "MIKE MULTICLIMATE H2"
#define CONFIG_NUM_SENSORS                   8

//-- Sensors configuration
#define CONFIG_SSD1306_ENABLED               false
#define CONFIG_BME280_ENABLED                true
#define CONFIG_BME680_ENABLED                true
#define CONFIG_DS18B20_ENABLED               true
#define CONFIG_DHT11_ENABLED                 true
#define CONFIG_MQ135_ENABLED                 true


//-- task priorities
#define CONFIG_SENSOR_POLL_TASK_PRIORITY     5
#define CONFIG_REBOOT_BUTTON_TASK_PRIORITY   4

//-- BMP280 sensor
//#define CONFIG_BME280_I2C_BUS                0
//#define CONFIG_BME280_I2C_ADDR               0x76
#define CONFIG_BME280_SDA_GPIO               1
#define CONFIG_BME280_SCL_GPIO               2
#define CONFIG_BME280_I2C_PORT               I2C_NUM_0
//-- BME680 sensor
//#define CONFIG_BME680_I2C_BUS                0
//#define CONFIG_BME680_I2C_ADDR               0x77
#define CONFIG_BME680_SDA_GPIO               3
#define CONFIG_BME680_SCL_GPIO               5
#define CONFIG_BME680_I2C_PORT               I2C_NUM_1
//-- DS18B20 sensor
#define CONFIG_DS18B20_GPIO                  10
//-- DHT11 sensor
#define CONFIG_DHT11_GPIO                    11
//-- MQ135 sensor
#define CONFIG_MQ135_GPIO                    4
#define CONFIG_MQ135_ADC_UNIT                ADC_UNIT_1
#define CONFIG_MQ135_ADC_CHANNEL             ADC_CHANNEL_3 // GPIO4 ESP32-H2


#define LIVE_BLINK_TIME_MS                   0
#define CONFIG_SENSOR_POLL_PERIOD_MS         5000


#if CONFIG_MQ135_ENABLED
  #include <esp_matter.h>
  using namespace esp_matter;
  
  #include "mq135_driver.h"
  
  extern mq135_handle_t s_mq135_handle;
  extern cluster_t *s_co2_cluster;
  extern cluster_t *s_co_cluster;
  extern cluster_t *s_tvoc_cluster;
#endif


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
  #define CONFIG_SSD1306_SDA_GPIO            1
  #define CONFIG_SSD1306_SCL_GPIO            2
  #define CONFIG_SSD1306_RESET_GPIO          -1
  #define CONFIG_SSD1306_I2C_ADDRESS         0x3C
  #define CONFIG_SSD1306_I2C_PORT            I2C_NUM_0
  
  extern bool ssd1306_initialized;
  extern SSD1306_t ssd1306dev;
#endif


// Sensor types
typedef enum {
  SENSOR_TYPE_BME280  = 0,
  SENSOR_TYPE_BME680  = 1,
  SENSOR_TYPE_DS18B20 = 2,
  SENSOR_TYPE_DHT11   = 3,
  SENSOR_TYPE_MQ135   = 4,
  SENSOR_TYPE_MAX
} sensor_type_t;

// Sensor data structure
typedef struct {
  sensor_type_t config;
  float last_temperature;
  float last_humidity;
  float last_pressure;
  float last_gas_resistance;  // For BME680
  float last_co2;             // For MQ135
  float last_co;              // For MQ135
  float last_tvoc;            // For MQ135
  uint64_t last_read_time;
  uint8_t num_sensors;        // For DS18B20
} sensor_data_t;

// Endpoint-sensor mapping
typedef struct {
  uint16_t endpoint_id;
  sensor_type_t sensor_type;
  gpio_num_t primary_gpio;
  gpio_num_t secondary_gpio;
} sensor_endpoint_mapping_t;

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include "esp_openthread_types.h"
#endif


extern sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[CONFIG_NUM_SENSORS];

typedef void *app_driver_handle_t;

/** Sensor initialization
 *
 * This API is called to initialize sensors
 *
 * @param[in] sensor_config Sensor configuration.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_sensor_init(sensor_type_t sensor_config);

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
 * @param[in] sensor_idx Index of the sensor in sensors array
 *
 * @return ESP_OK on success
 */
esp_err_t app_driver_read_sensor_data(uint8_t sensor_idx);

/** Task to poll sensors
 *
 * This task periodically reads sensor values and updates Matter attributes
 */
void sensor_polling_task(void *pvParameters);

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
