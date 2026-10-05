#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
//#include <driver/i2c.h>
#include "driver/i2c_master.h"
#include <button_gpio.h>
//#include "driver_led_indicator.h"
#include "driver_reset_button.h"

#define MOCK_SENSORS_BEHAVIOR                 false
#define DO_DEBUG                              true


#define TAG_MULTI_CLIMATE                     "MIKE MULTICLIMATE H2"
#define CONFIG_NUM_SENSORS                    8

//-- Sensors configuration
#define CONFIG_SSD1306_ENABLED                true
#define CONFIG_BME280_ENABLED                 true
#define CONFIG_BME680_ENABLED                 true
#define CONFIG_DS18B20_ENABLED                false
#define CONFIG_DHT11_ENABLED                  false
#define CONFIG_SENSOR_POLL_PERIOD_MS          5000

//-- task priorities
#define CONFIG_SENSOR_POLL_TASK_PRIORITY      5
#define CONFIG_REBOOT_BUTTON_TASK_PRIORITY    4

//-- I2C Configuration
#define CONFIG_BME280_I2C_BUS                 0
#define CONFIG_BME280_I2C_ADDR                0x76
#define CONFIG_BME680_I2C_BUS                 0
#define CONFIG_BME680_I2C_ADDR                0x77

//-- GPIO Configuration
#define CONFIG_BME280_SDA_GPIO                1
#define CONFIG_BME280_SCL_GPIO                2
#define CONFIG_BME680_SDA_GPIO                3
#define CONFIG_BME680_SCL_GPIO                5
#define CONFIG_DS18B20_GPIO                   10
#define CONFIG_DHT11_GPIO                     11

#define CONFIG_BME280_I2C_PORT                I2C_NUM_0
#define CONFIG_BME680_I2C_PORT                I2C_NUM_1

#define LIVE_BLINK_TIME_MS                    0
#define I2C_MASTER_BUS_FREQ_HZ                100000

#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306.h"
  #include "ssd1306_i2cdev.h"
  
  //-- SSD1306
  #define CONFIG_SCL_GPIO        13
  #define CONFIG_SDA_GPIO        14
  #define CONFIG_RESET_GPIO      -1
  #define CONFIG_I2C_ADDRESS     0x3C
  #define I2C_MASTER_FREQ_HZ     400000 // I2C clock of SSD1306 can run at 400 kHz max.
  
  //#define CONFIG_I2C_INTERFACE   true
  //#define CONFIG_SSD1306_128x64  true
  
  extern bool ssd1306_initialized;
  extern SSD1306_t ssd1306dev;
#endif


// Sensor types
typedef enum {
  SENSOR_TYPE_BME280  = 0,
  SENSOR_TYPE_BME680  = 1,
  SENSOR_TYPE_DS18B20 = 2,
  SENSOR_TYPE_DHT11   = 3,
  SENSOR_TYPE_MAX
} sensor_type_t;

// Sensor data structure
typedef struct {
  sensor_type_t config;
  float last_temperature;
  float last_humidity;
  float last_pressure;
  float last_gas_resistance;  // For BME680
  uint64_t last_read_time;
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


//!!! I2C !!!
typedef struct {
    i2c_master_bus_handle_t bus_handle;
    i2c_master_dev_handle_t dev_handle;
} i2c_bus_dev_t;

esp_err_t i2c_init_bus_and_device(i2c_port_num_t port, gpio_num_t sda, gpio_num_t scl, uint8_t dev_addr, i2c_bus_dev_t *out_dev);
// Инициализация I2C шины
extern void i2c_master_init(void);
// Добавление устройства на шину
extern i2c_master_dev_handle_t add_device(uint16_t address, const char *name);
// Проверка наличия устройства на шине
extern bool probe_device(uint16_t address);

//!!! I2C !!!


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

/*
extern uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay);
*/
