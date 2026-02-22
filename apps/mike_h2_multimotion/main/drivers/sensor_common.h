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

// Общие определения для всех датчиков
#define OCCUPANCY_UNOCCUPIED                   0x00
#define OCCUPANCY_OCCUPIED                      0x01

// Общие типы датчиков
typedef enum {
    SENSOR_TYPE_PIR = 0,
    SENSOR_TYPE_MICROWAVE,
    SENSOR_TYPE_ULTRASONIC,
    SENSOR_TYPE_MAX
} sensor_type_t;

// Общая структура конфигурации
typedef struct {
    sensor_type_t type;
    gpio_num_t trigger_pin;
    gpio_num_t echo_pin;  // For HC-SR04 only
    uint16_t endpoint_id;
    const char* name;
} sensor_config_t;

// Общая структура данных
typedef struct {
    sensor_config_t config;
    bool last_state;
    uint32_t last_distance_cm;  // For ultrasonic sensor
    uint64_t last_detection_time;
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
