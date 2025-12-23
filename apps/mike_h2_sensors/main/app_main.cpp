#include "app_priv.h"
#include "hc_sr501.h"
#include "rcwl_0516.h"
#include "hc_sr04.h"

#include <app/server/Server.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <app/server/CommissioningWindowManager.h>

using namespace chip;
using namespace esp_matter;
using namespace esp_matter::endpoint;

// Глобальные переменные
static uint16_t motion_endpoint_id = 0;
static uint16_t distance_endpoint_id = 0;
static uint16_t occupancy_endpoint_id = 0;

QueueHandle_t sensor_event_queue = NULL;

// Экземпляры датчиков
static hc_sr501_t pir_sensor;
static rcwl_0516_t microwave_sensor;
static hc_sr04_t ultrasonic_sensor;

// Обработчики событий датчиков
static void pir_motion_callback(bool motion) {
    sensor_event_t event = {
        .endpoint_id = ENDPOINT_ID_MOTION,
        .cluster_id = chip::app::Clusters::BooleanState::Id,
        .attribute_id = chip::app::Clusters::BooleanState::Attributes::StateValue::Id,
        .value = esp_matter_bool(motion)
    };
    xQueueSend(sensor_event_queue, &event, portMAX_DELAY);
}

static void microwave_motion_callback(bool motion) {
    sensor_event_t event = {
        .endpoint_id = ENDPOINT_ID_OCCUPANCY,
        .cluster_id = chip::app::Clusters::OccupancySensing::Id,
        .attribute_id = chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Id,
        .value = esp_matter_uint8(motion ? 1 : 0) // 1 = occupied, 0 = unoccupied
    };
    xQueueSend(sensor_event_queue, &event, portMAX_DELAY);
}

static void distance_callback(float distance_cm) {
    sensor_event_t event = {
        .endpoint_id = ENDPOINT_ID_DISTANCE,
        .cluster_id = chip::app::Clusters::DistanceMeasurement::Id,
        .attribute_id = chip::app::Clusters::DistanceMeasurement::Attributes::MeasuredValue::Id,
        .value = esp_matter_uint16((uint16_t)(distance_cm * 10)) // В мм * 10 для Matter
    };
    xQueueSend(sensor_event_queue, &event, portMAX_DELAY);
}

// Создание эндпоинтов Matter
void create_matter_endpoints() {
    node_t *node = node::get();
    
    // 1. Эндпоинт для PIR датчика (Boolean State)
    on_off_light::config_t motion_config;
    motion_config.on_off.on_off = false;
    motion_config.on_off.lighting.start_up_on_off = false;
    
    endpoint_t *motion_endpoint = on_off_light::create(node, &motion_config, ENDPOINT_FLAG_NONE, NULL);
    motion_endpoint_id = endpoint::get_id(motion_endpoint);
    
    // Добавляем кластер Boolean State
    cluster_t *boolean_state_cluster = cluster::create(
        motion_endpoint, 
        chip::app::Clusters::BooleanState::Id, 
        CLUSTER_FLAG_SERVER
    );
    
    // 2. Эндпоинт для ультразвукового датчика (Distance Measurement)
    endpoint_t *distance_endpoint = endpoint::create(node, NULL, ENDPOINT_FLAG_NONE, NULL);
    distance_endpoint_id = endpoint::get_id(distance_endpoint);
    
    // Создаем кластер Distance Measurement
    cluster_t *distance_cluster = cluster::create(
        distance_endpoint,
        chip::app::Clusters::DistanceMeasurement::Id,
        CLUSTER_FLAG_SERVER
    );
    
    // Атрибуты для Distance Measurement
    attribute::create(
        distance_cluster,
        chip::app::Clusters::DistanceMeasurement::Attributes::MeasuredValue::Id,
        ATTRIBUTE_FLAG_NONE,
        esp_matter_uint16(0)
    );
    
    // 3. Эндпоинт для микроволнового датчика (Occupancy Sensing)
    endpoint_t *occupancy_endpoint = endpoint::create(node, NULL, ENDPOINT_FLAG_NONE, NULL);
    occupancy_endpoint_id = endpoint::get_id(occupancy_endpoint);
    
    // Создаем кластер Occupancy Sensing
    cluster_t *occupancy_cluster = cluster::create(
        occupancy_endpoint,
        chip::app::Clusters::OccupancySensing::Id,
        CLUSTER_FLAG_SERVER
    );
    
    // Атрибуты для Occupancy Sensing
    attribute::create(
        occupancy_cluster,
        chip::app::Clusters::OccupancySensing::Attributes::Occupancy::Id,
        ATTRIBUTE_FLAG_NONE,
        esp_matter_uint8(0)
    );
    
    ESP_LOGI(TAG_MAIN, "Matter endpoints created: Motion=%d, Distance=%d, Occupancy=%d",
             motion_endpoint_id, distance_endpoint_id, occupancy_endpoint_id);
}

// Задача для чтения датчиков
void sensor_read_task(void *pvParameters) {
    ESP_LOGI(TAG_MAIN, "Sensor read task started");
    
    while (true) {
        // Чтение ультразвукового датчика каждые 5 секунд
        float distance = hc_sr04_measure(&ultrasonic_sensor);
        if (distance > 0) {
            distance_callback(distance);
        }
        
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

// Обработчик событий от датчиков
void app_driver_handle_sensor_event(sensor_event_t *event) {
    esp_matter_attr_val_t val = event->value;
    
    // Обновление атрибута в Matter
    esp_err_t err = attribute::update(
        event->endpoint_id,
        event->cluster_id,
        event->attribute_id,
        &val
    );
    
    if (err == ESP_OK) {
        ESP_LOGI(TAG_SENSOR, "Attribute updated: endpoint=%d, cluster=0x%08lx, attribute=0x%08lx",
                 event->endpoint_id, event->cluster_id, event->attribute_id);
    } else {
        ESP_LOGE(TAG_SENSOR, "Failed to update attribute: %d", err);
    }
}

extern "C" void app_main() {
    esp_err_t err = ESP_OK;
    
    ESP_LOGI(TAG_MAIN, "ESP-Matter Sensors Starting...");
    
    // Инициализация NVS
    nvs_flash_init();
    
    // Инициализация очереди событий
    sensor_event_queue = xQueueCreate(10, sizeof(sensor_event_t));
    
    // Инициализация датчиков
    hc_sr501_init(&pir_sensor, GPIO_NUM_4);
    hc_sr501_set_callback(&pir_sensor, pir_motion_callback);
    
    rcwl_0516_init(&microwave_sensor, GPIO_NUM_5);
    rcwl_0516_set_callback(&microwave_sensor, microwave_motion_callback);
    
    hc_sr04_init(&ultrasonic_sensor, GPIO_NUM_6, GPIO_NUM_7);
    hc_sr04_set_callback(&ultrasonic_sensor, distance_callback);
    
    // Инициализация Matter
    matter_config_t config = {
        .commissioner = false,
        .enable_server = true,
    };
    
    err = esp_matter::start(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MAIN, "Matter start failed: %d", err);
        return;
    }
    
    // Создание эндпоинтов
    create_matter_endpoints();
    
    // Запуск задачи чтения датчиков
    xTaskCreate(sensor_read_task, "sensor_task", 4096, NULL, 5, NULL);
    
    // Задача обработки событий датчиков
    xTaskCreate([](void *arg) {
        sensor_event_t event;
        while (true) {
            if (xQueueReceive(sensor_event_queue, &event, portMAX_DELAY) == pdTRUE) {
                app_driver_handle_sensor_event(&event);
            }
        }
    }, "event_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG_MAIN, "ESP-Matter Sensors Started Successfully");
    
    // Бесконечный цикл
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}