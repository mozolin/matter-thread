#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <driver/i2c.h>
#include <button_gpio.h>
#include "driver_reset_button.h"

#define TAG_MULTI_SENSOR   "MIKE MULTISENSOR H2"
#define CONFIG_NUM_SENSORS 8

//-- Sensors configuration
#define CONFIG_BME280_ENABLED                 true
#define CONFIG_BME680_ENABLED                  true
#define CONFIG_DS18B20_ENABLED                  true
#define CONFIG_DHT11_ENABLED                  true
#define CONFIG_SENSOR_POLL_PERIOD_MS           2000

//-- task priorities
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5
#define CONFIG_REBOOT_BUTTON_TASK_PRIORITY     4

//-- I2C Configuration
#define CONFIG_BME280_I2C_BUS                  0
#define CONFIG_BME280_I2C_ADDR                 0x76
#define CONFIG_BME680_I2C_BUS                   0
#define CONFIG_BME680_I2C_ADDR                   0x76

//-- GPIO Configuration
#define CONFIG_BME280_SDA_GPIO                  1
#define CONFIG_BME280_SCL_GPIO                  2
#define CONFIG_BME680_SDA_GPIO                   3
#define CONFIG_BME680_SCL_GPIO                   5
#define CONFIG_DS18B20_GPIO                     6
#define CONFIG_DHT11_GPIO                        7

#define CONFIG_BME280_I2C_PORT                  I2C_NUM_0
#define CONFIG_BME680_I2C_PORT                  I2C_NUM_0

// Sensor types
typedef enum {
    SENSOR_TYPE_BME280 = 0,
    SENSOR_TYPE_BME680,
    SENSOR_TYPE_DS18B20,
    SENSOR_TYPE_DHT11,
    SENSOR_TYPE_MAX
} sensor_type_t;

// Sensor configuration structure
typedef struct {
    sensor_type_t type;
    gpio_num_t sda_pin;      // For I2C sensors
    gpio_num_t scl_pin;      // For I2C sensors
    gpio_num_t data_pin;     // For 1-wire (DS18B20) and DHT11
    i2c_port_t i2c_bus;
    uint8_t i2c_addr;
    uint16_t endpoint_id;
    const char* name;
} sensor_config_t;

// Sensor data structure
typedef struct {
    sensor_config_t config;
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

extern sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[CONFIG_NUM_SENSORS];

// Function to get sensor by endpoint ID
sensor_data_t* get_sensor_by_endpoint(uint16_t endpoint_id);

// Function to get endpoint by sensor type
uint16_t get_endpoint_by_sensor_type(sensor_type_t type);

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
esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_config);

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
