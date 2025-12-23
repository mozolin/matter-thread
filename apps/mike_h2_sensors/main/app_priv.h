#pragma once

#include <esp_matter.h>
#include <esp_matter_core.h>
#include <esp_matter_cluster.h>
#include <esp_matter_endpoint.h>
#include <esp_matter_attribute.h>
#include <esp_matter_ota.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include "sensor_driver.h"

// Теги для логов
#define TAG_MAIN "MAIN"
#define TAG_SENSOR "SENSOR"

// Константы для Matter
#define ENDPOINT_ID_MOTION 1
#define ENDPOINT_ID_DISTANCE 2
#define ENDPOINT_ID_OCCUPANCY 3

// Структура для передачи данных датчиков
typedef struct {
    uint16_t endpoint_id;
    uint32_t cluster_id;
    uint32_t attribute_id;
    esp_matter_attr_val_t value;
} sensor_event_t;

// Глобальные переменные
extern QueueHandle_t sensor_event_queue;

// Функции
void app_driver_handle_sensor_event(sensor_event_t *event);
void app_driver_update_attribute(uint16_t endpoint_id, uint32_t cluster_id, 
                                 uint32_t attribute_id, esp_matter_attr_val_t *val);
void create_matter_endpoints();
void sensor_read_task(void *pvParameters);
