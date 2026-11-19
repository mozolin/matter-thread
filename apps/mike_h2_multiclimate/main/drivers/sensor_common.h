#pragma once

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "soc/gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SENSOR_I2C_FREQ_HZ 1000000 // Up to 3.4MHz, but esp-idf only supports 1MHz

// Sensor types
typedef enum {
  SENSOR_TYPE_BME280  = 0,
  SENSOR_TYPE_BME680  = 1,
  SENSOR_TYPE_DS18B20 = 2,
  SENSOR_TYPE_DHT11   = 3,
  SENSOR_TYPE_MQ135   = 4,
  SENSOR_TYPE_SHT4X   = 5,
  SENSOR_TYPE_SCD4X   = 6,
  SENSOR_TYPE_MAX,
} sensor_type_t;

// Sensor data structure
typedef struct {
  sensor_type_t config;
  float last_temperature;
  float last_humidity;
  float last_pressure;
  float last_gas_resistance;  // For BME680
  float last_co2;             // For MQ135 & SCD4X
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


#ifdef __cplusplus
}
#endif
