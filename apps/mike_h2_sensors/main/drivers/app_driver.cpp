/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>
#include "sensor_driver.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

// Global sensor device instances
static hcsr501_dev_t pir_sensor;
static rcwl0516_dev_t microwave_sensor;
static hcsr04_dev_t ultrasonic_sensor;

esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_cfg)
{
    if (sensor_cfg == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor config is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ESP_OK;

    switch (sensor_cfg->type) {
        case SENSOR_TYPE_PIR:
            err = hcsr501_init(&pir_sensor, sensor_cfg->trigger_pin);
            break;
        case SENSOR_TYPE_MICROWAVE:
            err = rcwl0516_init(&microwave_sensor, sensor_cfg->trigger_pin);
            break;
        case SENSOR_TYPE_ULTRASONIC:
            err = hcsr04_init(&ultrasonic_sensor, sensor_cfg->trigger_pin, sensor_cfg->echo_pin);
            break;
        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown sensor type: %d", sensor_cfg->type);
            err = ESP_ERR_INVALID_ARG;
            break;
    }

    return err;
}

bool app_driver_read_sensor_state(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Invalid sensor index: %d", sensor_idx);
        return false;
    }

    sensor_type_t type = sensors[sensor_idx].config.type;
    
    switch (type) {
        case SENSOR_TYPE_PIR:
            return hcsr501_read(&pir_sensor);
        case SENSOR_TYPE_MICROWAVE:
            return rcwl0516_read(&microwave_sensor);
        case SENSOR_TYPE_ULTRASONIC:
            // Ultrasonic doesn't have a binary state, use distance threshold
            return (hcsr04_measure_distance(&ultrasonic_sensor) < 50); // 50cm threshold
        default:
            return false;
    }
}

uint32_t app_driver_read_ultrasonic_distance(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Invalid sensor index: %d", sensor_idx);
        return 0;
    }

    if (sensors[sensor_idx].config.type != SENSOR_TYPE_ULTRASONIC) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor %d is not ultrasonic", sensor_idx);
        return 0;
    }

    return hcsr04_measure_distance(&ultrasonic_sensor);
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    // Sensors are read-only in this implementation
    // Updates come from sensor polling task, not from Matter
    return ESP_OK;
}

// Helper function to get sensor by endpoint ID
sensor_data_t* get_sensor_by_endpoint(uint16_t endpoint_id)
{
    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].endpoint_id == endpoint_id) {
            return &sensors[i];
        }
    }
    return NULL;
}

// Helper function to get endpoint by sensor type
uint16_t get_endpoint_by_sensor_type(sensor_type_t type)
{
    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].sensor_type == type) {
            return sensor_mapping_list[i].endpoint_id;
        }
    }
    return 0;
}

// Sensor polling task implementation
void sensor_polling_task(void *pvParameters)
{
    ESP_LOGI(TAG_MULTI_SENSOR, "Sensor polling task started");
    
    const TickType_t poll_period_ms = CONFIG_SENSOR_POLL_PERIOD_MS / portTICK_PERIOD_MS;
    
    while (true) {
        for (int i = 0; i < configured_sensors; i++) {
            sensor_data_t *sensor = &sensors[i];
            uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
            
            switch (sensor->config.type) {
                case SENSOR_TYPE_PIR:
                case SENSOR_TYPE_MICROWAVE: {
                    bool current_state = app_driver_read_sensor_state(i);
                    
                    if (current_state != sensor->last_state) {
                        sensor->last_state = current_state;
                        sensor->last_detection_time = esp_timer_get_time();
                        
                        // Update Matter attribute
                        esp_matter_attr_val_t val = esp_matter_bool(current_state);
                        esp_matter::attribute::update(endpoint_id, 
                                                      OccupancySensing::Id, 
                                                      OccupancySensing::Attributes::Occupancy::Id, 
                                                      &val);
                        
                        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Sensor %s: state changed to %d", 
                                sensor->config.name, current_state);
                    }
                    break;
                }
                case SENSOR_TYPE_ULTRASONIC: {
                    uint32_t current_distance = app_driver_read_ultrasonic_distance(i);
                    
                    if (abs((int)current_distance - (int)sensor->last_distance_cm) > 5) {
                        sensor->last_distance_cm = current_distance;
                        sensor->last_detection_time = esp_timer_get_time();
                        
                        // Update Matter attribute (using temperature sensor for distance)
                        // Convert cm to "temperature" (25 = 100cm baseline)
                        int16_t temp_value = 25 - (current_distance / 4);
                        if (temp_value < -50) temp_value = -50;
                        if (temp_value > 150) temp_value = 150;
                        
                        esp_matter_attr_val_t val = esp_matter_int16(temp_value);
                        esp_matter::attribute::update(endpoint_id, 
                                                      TemperatureMeasurement::Id, 
                                                      TemperatureMeasurement::Attributes::MeasuredValue::Id, 
                                                      &val);
                        
                        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Ultrasonic sensor: distance = %lu cm (temp value = %d)", 
                                current_distance, temp_value);
                    }
                    break;
                }
                default:
                    break;
            }
        }
        
        vTaskDelay(poll_period_ms);
    }
}