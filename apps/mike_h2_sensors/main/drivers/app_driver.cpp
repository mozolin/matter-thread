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

//-- HC-SR501: Passive Infrared (PIR) Motion Sensor
static hcsr501_dev_t pir_sensor;
//-- RCWL-0516: Microwave Doppler Radar Motion Sensor
static rcwl0516_dev_t microwave_sensor;
//-- HC-SR04: Ultrasonic Distance Sensor
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
    //ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Sensor polling task started");
    
    const TickType_t poll_period_ms = CONFIG_SENSOR_POLL_PERIOD_MS / portTICK_PERIOD_MS;
    
    while (true) {
        for (int i = 0; i < configured_sensors; i++) {
            sensor_data_t *sensor = &sensors[i];
            uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
            
            switch (sensor->config.type) {
                case SENSOR_TYPE_PIR: {
                    // Read PIR (HC-SR501) sensor state
                    bool current_state = hcsr501_read(&pir_sensor);

                    //ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501 state: %d (was: %d)", (int)current_state, (int)sensor->last_state);
                    
                    if (current_state != sensor->last_state) {
                        sensor->last_state = current_state;
                        sensor->last_detection_time = esp_timer_get_time();
                        
                        // Update Matter attribute - convert bool to uint8_t for occupancy
                        uint8_t occupancy_value = current_state ? OCCUPANCY_OCCUPIED : OCCUPANCY_UNOCCUPIED;
                        esp_matter_attr_val_t val = esp_matter_uint8(occupancy_value);
                        
                        esp_err_t update_err = esp_matter::attribute::update(endpoint_id, 
                                                      OccupancySensing::Id, 
                                                      OccupancySensing::Attributes::Occupancy::Id, 
                                                      &val);
                        
                        if (update_err == ESP_OK) {
                            ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR501 Sensor: motion %s (occupancy = %d)", 
                                    current_state ? "detected" : "ended", occupancy_value);
                        } else {
                            ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update HC-SR501 occupancy attribute: %d", update_err);
                        }
                    }
                    break;
                }
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
                        
                        /*
                        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ RCWL-0516: state changed to %d", 
                                sensor->config.name, current_state);
                        */
                        ESP_LOGI(TAG_MULTI_SENSOR, "RCWL-0516: state changed to %d", current_state);
                    }
                    break;
                }
                
                case SENSOR_TYPE_ULTRASONIC: {
                    // Read ultrasonic sensor distance
                    uint32_t current_distance = hcsr04_measure_distance(&ultrasonic_sensor);
                    
                    // Validate distance (HC-SR04 range: 2-400 cm)
                    if (current_distance < 2 || current_distance > 400) {
                        ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Invalid distance reading: %lu cm", current_distance);
                        break;
                    }
                    
                    // Convert distance to pressure value for Matter Pressure Measurement cluster
                    // Formula: pressure = 1000 + (distance_cm * 10)
                    // Where: 
                    // - 1000 = base pressure (represents 0 cm)
                    // - 10 = scaling factor (1 cm = 10 pressure units)
                    // Result: 2-400 cm -> 1020-5000 pressure units
                    const int16_t BASE_PRESSURE = 1000;    // 100.0 kPa at 0 cm
                    const int16_t PRESSURE_PER_CM = 10;    // 1.0 kPa per 10 cm
                    
                    int16_t pressure_value = BASE_PRESSURE + (current_distance * PRESSURE_PER_CM);
                    
                    // Update if distance changed significantly (> 1cm) or 2 seconds passed
                    bool should_update = false;
                    uint64_t current_time = esp_timer_get_time();
                    
                    // Calculate previous pressure value for comparison
                    int16_t last_pressure_value = BASE_PRESSURE + (sensor->last_distance_cm * PRESSURE_PER_CM);
                    int16_t pressure_change = abs(pressure_value - last_pressure_value);
                    
                    if (pressure_change > (PRESSURE_PER_CM * 2)) {  // > 2 cm change
                        should_update = true;
                        ESP_LOGD(TAG_MULTI_SENSOR, "HC-SR04: Significant change detected: %d pressure units", 
                                pressure_change);
                    } 
                    else if ((current_time - sensor->last_detection_time) > 2000000) { // 2 seconds
                        should_update = true;
                        ESP_LOGD(TAG_MULTI_SENSOR, "HC-SR04: Time-based update (2s interval)");
                    }
                    
                    if (should_update) {
                        sensor->last_distance_cm = current_distance;
                        sensor->last_detection_time = current_time;
                        
                        // Update Pressure Measurement cluster
                        //esp_matter_attr_val_t pressure_val = esp_matter_int16(pressure_value);
                        esp_matter_attr_val_t pressure_val = esp_matter_int16(current_distance * 10);
                        
                        esp_err_t update_err = esp_matter::attribute::update(
                            endpoint_id, 
                            PressureMeasurement::Id, 
                            PressureMeasurement::Attributes::MeasuredValue::Id, 
                            &pressure_val
                        );
                        
                        if (update_err == ESP_OK) {
                            ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Distance = %lu cm -> Pressure = %d (0.1 kPa)", 
                                    current_distance, pressure_value);
                            
                            // Optional: Also update occupancy if distance indicates presence
                            bool motion_detected = (current_distance < 50); // Object within 50cm
                            uint8_t occupancy_value = motion_detected ? 0x01 : 0x00;

                            if (motion_detected != sensor->last_state) {
                                esp_matter_attr_val_t occupancy_val = esp_matter_uint8(occupancy_value);
                                
                                esp_err_t occupancy_err = esp_matter::attribute::update(
                                    endpoint_id, 
                                    OccupancySensing::Id, 
                                    OccupancySensing::Attributes::Occupancy::Id, 
                                    &occupancy_val
                                );
                                
                                if (occupancy_err == ESP_OK) {
                                    ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Motion %s (distance = %lu cm)", 
                                            motion_detected ? "detected" : "ended", current_distance);
                                } else {
                                    ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update occupancy: %d", occupancy_err);
                                }
                                
                                sensor->last_state = motion_detected;
                            }
                        } else {
                            ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update pressure attribute: %d", update_err);
                        }
                    }
                    break;
                }
                
                /*
                case SENSOR_TYPE_ULTRASONIC: {
                    // Read ultrasonic sensor distance
                    uint32_t current_distance = hcsr04_measure_distance(&ultrasonic_sensor);
                    
                    // Validate distance (HC-SR04 range: 2-400 cm)
                    if (current_distance < 2 || current_distance > 400) {
                        ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Invalid distance reading: %lu cm", current_distance);
                        break;
                    }
                    
                    // Convert distance to pressure value
                    const int16_t BASE_PRESSURE = 1000;
                    const int16_t PRESSURE_PER_CM = 10;
                    int16_t pressure_value = BASE_PRESSURE + (current_distance * PRESSURE_PER_CM);
                    
                    // Check occupancy based on distance threshold
                    bool motion_detected = (current_distance < 50); // Object within 50cm
                    uint8_t occupancy_value = motion_detected ? 0x01 : 0x00;
                    
                    // Update conditions
                    bool should_update_pressure = false;
                    bool should_update_occupancy = false;
                    uint64_t current_time = esp_timer_get_time();
                    
                    // Check pressure change (> 2cm change)
                    int16_t last_pressure_value = BASE_PRESSURE + (sensor->last_distance_cm * PRESSURE_PER_CM);
                    int16_t pressure_change = abs(pressure_value - last_pressure_value);
                    
                    if (pressure_change > (PRESSURE_PER_CM * 2)) {
                        should_update_pressure = true;
                    }
                    
                    // Check occupancy change
                    if (motion_detected != sensor->last_state) {
                        should_update_occupancy = true;
                    }
                    
                    // Time-based update (every 2 seconds)
                    if ((current_time - sensor->last_detection_time) > 2000000) {
                        should_update_pressure = true;
                    }
                    
                    // Update if needed
                    if (should_update_pressure || should_update_occupancy) {
                        sensor->last_distance_cm = current_distance;
                        sensor->last_state = motion_detected;
                        sensor->last_detection_time = current_time;
                        
                        // 1. Update Pressure Measurement cluster
                        if (should_update_pressure) {
                            esp_matter_attr_val_t pressure_val = esp_matter_int16(pressure_value);
                            
                            esp_err_t pressure_err = esp_matter::attribute::update(
                                endpoint_id, 
                                PressureMeasurement::Id, 
                                PressureMeasurement::Attributes::MeasuredValue::Id, 
                                &pressure_val
                            );
                            
                            if (pressure_err == ESP_OK) {
                                ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Distance = %lu cm -> Pressure = %d", 
                                        current_distance, pressure_value);
                            } else {
                                ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update pressure: %d", pressure_err);
                            }
                        }
                        
                        // 2. Update Occupancy Sensing cluster
                        if (should_update_occupancy) {
                            esp_matter_attr_val_t occupancy_val = esp_matter_uint8(occupancy_value);
                            
                            esp_err_t occupancy_err = esp_matter::attribute::update(
                                endpoint_id, 
                                OccupancySensing::Id, 
                                OccupancySensing::Attributes::Occupancy::Id, 
                                &occupancy_val
                            );
                            
                            if (occupancy_err == ESP_OK) {
                                ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Motion %s (distance = %lu cm)", 
                                        motion_detected ? "detected" : "ended", current_distance);
                            } else {
                                ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update occupancy: %d", occupancy_err);
                            }
                        }
                    }
                    break;
                }
                */

                default:
                    break;
            }
        }
        
        vTaskDelay(poll_period_ms);
    }
}