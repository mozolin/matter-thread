#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>
#include "sensor_common.h"
#include "sensor_pir.h"
#include "sensor_microwave.h"
#include "sensor_ultrasonic.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

// Статические экземпляры драйверов датчиков
static hcsr501_dev_t pir_sensor;
static rcwl0516_dev_t microwave_sensor;
static hcsr04_dev_t ultrasonic_sensor;

typedef struct {
    uint64_t last_reset_time;
    uint32_t reset_count;
    bool reset_in_progress;
} sensor_reset_tracker_t;

static sensor_reset_tracker_t sensor_reset_tracker[SENSOR_TYPE_MAX];

// Константы
#define RESET_INTERVAL_HOURS            1
#define MS_PER_HOUR                     (3600 * 1000)
#define RESET_INTERVAL_MS               (RESET_INTERVAL_HOURS * MS_PER_HOUR)

// Константы для детектирования движения HC-SR04
#define MOTION_DETECTION_THRESHOLD_CM   5
#define MIN_DISTANCE_FOR_MOTION_CM      400
#define UPDATE_DISTANCE_CHANGED_BY_CM   1

esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_cfg)
{
    if (sensor_cfg == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor config is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ESP_OK;

    switch (sensor_cfg->type) {
#if CONFIG_HCSR501_ENABLED
        case SENSOR_TYPE_PIR:
            err = hcsr501_init(&pir_sensor, sensor_cfg->trigger_pin);
            break;
#endif

#if CONFIG_RCWL0516_ENABLED
        case SENSOR_TYPE_MICROWAVE:
            err = rcwl0516_init(&microwave_sensor, sensor_cfg->trigger_pin);
            break;
#endif

#if CONFIG_HCSR04_ENABLED
        case SENSOR_TYPE_ULTRASONIC:
            err = hcsr04_init(&ultrasonic_sensor, sensor_cfg->trigger_pin, sensor_cfg->echo_pin);
            break;
#endif

        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown or disabled sensor type: %d", sensor_cfg->type);
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
#if CONFIG_HCSR501_ENABLED
        case SENSOR_TYPE_PIR:
            return hcsr501_read(&pir_sensor);
#endif

#if CONFIG_RCWL0516_ENABLED
        case SENSOR_TYPE_MICROWAVE:
            return rcwl0516_read(&microwave_sensor);
#endif

#if CONFIG_HCSR04_ENABLED
        case SENSOR_TYPE_ULTRASONIC:
            return (hcsr04_measure_distance(&ultrasonic_sensor) > 0);
#endif

        default:
            return false;
    }
}

uint32_t app_driver_read_ultrasonic_distance(uint8_t sensor_idx)
{
#if CONFIG_HCSR04_ENABLED
    if (sensor_idx >= configured_sensors) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Invalid sensor index: %d", sensor_idx);
        return 0;
    }

    if (sensors[sensor_idx].config.type != SENSOR_TYPE_ULTRASONIC) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor %d is not ultrasonic", sensor_idx);
        return 0;
    }

    return hcsr04_measure_distance(&ultrasonic_sensor);
#else
    ESP_LOGE(TAG_MULTI_SENSOR, "Ultrasonic sensor is disabled");
    return 0;
#endif
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    return ESP_OK;
}

sensor_data_t* get_sensor_by_endpoint(uint16_t endpoint_id)
{
    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].endpoint_id == endpoint_id) {
            return &sensors[i];
        }
    }
    return NULL;
}

uint16_t get_endpoint_by_sensor_type(sensor_type_t type)
{
    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].sensor_type == type) {
            return sensor_mapping_list[i].endpoint_id;
        }
    }
    return 0;
}

esp_err_t app_driver_sensor_reinit(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sensor_config_t *cfg = &sensors[sensor_idx].config;
    ESP_LOGW(TAG_MULTI_SENSOR, "Reinitializing sensor %d", sensor_idx);
    
    switch (cfg->type) {
#if CONFIG_HCSR501_ENABLED
        case SENSOR_TYPE_PIR:
            return hcsr501_init(&pir_sensor, cfg->trigger_pin);
#endif

#if CONFIG_RCWL0516_ENABLED
        case SENSOR_TYPE_MICROWAVE:
            return rcwl0516_init(&microwave_sensor, cfg->trigger_pin);
#endif

#if CONFIG_HCSR04_ENABLED
        case SENSOR_TYPE_ULTRASONIC:
            return hcsr04_init(&ultrasonic_sensor, cfg->trigger_pin, cfg->echo_pin);
#endif

        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown or disabled sensor type: %d", cfg->type);
            return ESP_ERR_NOT_SUPPORTED;
    }
}

static esp_err_t app_driver_sensor_soft_reset(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sensor_data_t *sensor = &sensors[sensor_idx];
    ESP_LOGW(TAG_MULTI_SENSOR, "Performing soft reset for sensor %d (%s)", 
            sensor_idx, sensor->config.name);
    
    esp_err_t err = ESP_OK;
    
    switch (sensor->config.type) {
#if CONFIG_HCSR501_ENABLED
        case SENSOR_TYPE_PIR:
            err = hcsr501_init(&pir_sensor, sensor->config.trigger_pin);
            break;
#endif

#if CONFIG_RCWL0516_ENABLED
        case SENSOR_TYPE_MICROWAVE:
            err = rcwl0516_init(&microwave_sensor, sensor->config.trigger_pin);
            break;
#endif

#if CONFIG_HCSR04_ENABLED
        case SENSOR_TYPE_ULTRASONIC:
            err = hcsr04_init(&ultrasonic_sensor, 
                             sensor->config.trigger_pin, 
                             sensor->config.echo_pin);
            if (err == ESP_OK) {
                vTaskDelay(pdMS_TO_TICKS(100));
                for (int i = 0; i < 3; i++) {
                    gpio_set_level(sensor->config.trigger_pin, 1);
                    esp_rom_delay_us(10);
                    gpio_set_level(sensor->config.trigger_pin, 0);
                    vTaskDelay(pdMS_TO_TICKS(10));
                }
            }
            break;
#endif

        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown or disabled sensor type for reset: %d", sensor->config.type);
            err = ESP_ERR_NOT_SUPPORTED;
            break;
    }
    
    if (err == ESP_OK) {
        sensor_reset_tracker[sensor_idx].reset_count++;
        sensor_reset_tracker[sensor_idx].last_reset_time = esp_timer_get_time() / 1000;
        ESP_LOGI(TAG_MULTI_SENSOR, "Sensor %d reset successful (total resets: %lu)", 
                sensor_idx, sensor_reset_tracker[sensor_idx].reset_count);
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to reset sensor %d: %d", sensor_idx, err);
    }
    
    return err;
}

void sensor_polling_task(void *pvParameters)
{
    const TickType_t poll_period_ms = 1;
    
    for (int i = 0; i < configured_sensors; i++) {
        sensor_reset_tracker[i].last_reset_time = esp_timer_get_time() / 1000;
        sensor_reset_tracker[i].reset_count = 0;
        sensor_reset_tracker[i].reset_in_progress = false;
    }
    
    uint32_t cycle_counter = 0;
    const uint32_t RESET_CHECK_INTERVAL = 100;
    const uint32_t STATS_LOG_INTERVAL = 5000;
    
    ESP_LOGI(TAG_MULTI_SENSOR, "Sensor polling task started with %d sensors", configured_sensors);
    
    while (true) {
        
        cycle_counter++;
        
        if (cycle_counter % RESET_CHECK_INTERVAL == 0) {
            uint64_t current_time_ms = esp_timer_get_time() / 1000;
            
            for (int i = 0; i < configured_sensors; i++) {
                if (sensor_reset_tracker[i].reset_in_progress) {
                    continue;
                }
                
                uint64_t time_since_last_reset = current_time_ms - sensor_reset_tracker[i].last_reset_time;
                
                if (time_since_last_reset >= RESET_INTERVAL_MS) {
                    ESP_LOGI(TAG_MULTI_SENSOR, 
                            "Scheduled reset for sensor %d (%s): %llu ms since last reset", 
                            i, sensors[i].config.name, time_since_last_reset);
                    
                    sensor_reset_tracker[i].reset_in_progress = true;
                    esp_err_t reset_err = app_driver_sensor_soft_reset(i);
                    sensor_reset_tracker[i].reset_in_progress = false;
                    
                    if (reset_err == ESP_OK) {
                        ESP_LOGI(TAG_MULTI_SENSOR, "Scheduled reset completed for sensor %d", i);
                    }
                    
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
            }
        }
        
        if (cycle_counter % STATS_LOG_INTERVAL == 0) {
            app_driver_log_sensor_statistics();
        }
        
        for (int i = 0; i < configured_sensors; i++) {
            if (sensor_reset_tracker[i].reset_in_progress) {
                ESP_LOGD(TAG_MULTI_SENSOR, "Skipping sensor %d (reset in progress)", i);
                continue;
            }
            
            sensor_data_t *sensor = &sensors[i];
            uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
            
            switch (sensor->config.type) {
#if CONFIG_HCSR501_ENABLED
                case SENSOR_TYPE_PIR: {
                    bool current_state = hcsr501_read(&pir_sensor);

                    static uint64_t last_pir_change = 0;
                    uint64_t now = esp_timer_get_time();
                    
                    if (current_state != sensor->last_state) {
                        if ((now - last_pir_change) < 2000000) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "PIR: Ignoring bounce");
                            break;
                        }
                        last_pir_change = now;
                        
                        sensor->last_state = current_state;
                        sensor->last_detection_time = esp_timer_get_time();
                        
                        uint8_t occupancy_value = current_state ? OCCUPANCY_OCCUPIED : OCCUPANCY_UNOCCUPIED;
                        esp_matter_attr_val_t val = esp_matter_uint8(occupancy_value);
                        
                        esp_err_t update_err = esp_matter::attribute::update(endpoint_id, 
                                                      OccupancySensing::Id, 
                                                      OccupancySensing::Attributes::Occupancy::Id, 
                                                      &val);
                        
                        if (update_err == ESP_OK) {
                            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501 Sensor: motion %s (occupancy = %d)", 
                                    current_state ? "detected" : "ended", occupancy_value);
                        } else {
                            ESP_LOGE(TAG_MULTI_SENSOR, "Failed to update HC-SR501 occupancy attribute: %d", update_err);
                        }
                    }
                    break;
                }
#endif

#if CONFIG_RCWL0516_ENABLED
                case SENSOR_TYPE_MICROWAVE: {
                    bool current_state = rcwl0516_read(&microwave_sensor);
                    
                    if (current_state != sensor->last_state) {
                        sensor->last_state = current_state;
                        sensor->last_detection_time = esp_timer_get_time();
                        
                        esp_matter_attr_val_t val = esp_matter_bool(current_state);
                        esp_matter::attribute::update(endpoint_id, 
                                                      OccupancySensing::Id, 
                                                      OccupancySensing::Attributes::Occupancy::Id, 
                                                      &val);
                        
                        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ RCWL-0516: state changed to %d", current_state);
                    }
                    break;
                }
#endif

#if CONFIG_HCSR04_ENABLED
                case SENSOR_TYPE_ULTRASONIC: {
                    uint32_t current_distance = hcsr04_measure_distance(&ultrasonic_sensor);
                    
                    if (current_distance < 2 || current_distance > 400) {
                        ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Invalid distance reading: %lu cm", current_distance);
                        break;
                    }

                    bool motion_detected = false;
                    bool should_update_pressure = false;
                    bool should_update_occupancy = false;
                    uint64_t current_time = esp_timer_get_time();

                    uint32_t last_distance_cm = sensor->last_distance_cm;
                    
                    if (last_distance_cm > 0) {
                        int32_t distance_change = abs((int32_t)current_distance - (int32_t)last_distance_cm);
                        
                        if (distance_change >= MOTION_DETECTION_THRESHOLD_CM && 
                            current_distance <= MIN_DISTANCE_FOR_MOTION_CM) {
                            motion_detected = true;
                        }
                        
                        if (distance_change > UPDATE_DISTANCE_CHANGED_BY_CM || 
                            (current_time - sensor->last_detection_time) > 2000000) {
                            should_update_pressure = true;
                        }
                    } else {
                        should_update_pressure = true;
                    }
                    
                    if (motion_detected != sensor->last_state) {
                        should_update_occupancy = true;
                    }
                    
                    if (should_update_pressure || should_update_occupancy) {
                        
                        sensor->last_distance_cm = current_distance;
                        sensor->last_detection_time = current_time;

                        esp_matter_attr_val_t pressure_val = esp_matter_int16(current_distance * 10);
                        
                        esp_err_t update_err = esp_matter::attribute::update(
                            endpoint_id, 
                            PressureMeasurement::Id, 
                            PressureMeasurement::Attributes::MeasuredValue::Id, 
                            &pressure_val
                        );

                        if (update_err == ESP_OK) {
                            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04: Distance = %lu cm", current_distance);
                            
                            if (should_update_occupancy) {
                                uint8_t occupancy_value = motion_detected ? OCCUPANCY_OCCUPIED : OCCUPANCY_UNOCCUPIED;
                                esp_matter_attr_val_t occupancy_val = esp_matter_uint8(occupancy_value);

                                esp_err_t occupancy_err = esp_matter::attribute::update(
                                    endpoint_id, 
                                    OccupancySensing::Id, 
                                    OccupancySensing::Attributes::Occupancy::Id, 
                                    &occupancy_val
                                );
                                
                                if (occupancy_err == ESP_OK) {
                                    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04: Motion %s (Δ %ld cm)", 
                                            motion_detected ? "detected" : "ended",
                                            last_distance_cm > 0 ? 
                                                abs((int32_t)current_distance - (int32_t)last_distance_cm) : 0);
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
#endif

                default:
                    break;
            }
        }
        
        vTaskDelay(poll_period_ms);
    }
}

esp_err_t app_driver_reset_sensor_by_type(sensor_type_t type)
{
    esp_err_t overall_err = ESP_OK;
    uint8_t reset_count = 0;
    
    for (int i = 0; i < configured_sensors; i++) {
        if (sensors[i].config.type == type) {
            ESP_LOGI(TAG_MULTI_SENSOR, "Resetting sensor %d of type %d", i, type);
            esp_err_t err = app_driver_sensor_soft_reset(i);
            if (err != ESP_OK) {
                overall_err = err;
                ESP_LOGE(TAG_MULTI_SENSOR, "Failed to reset sensor %d: %d", i, err);
            } else {
                reset_count++;
            }
            vTaskDelay(pdMS_TO_TICKS(100));
        }
    }
    
    ESP_LOGI(TAG_MULTI_SENSOR, "Reset %d sensors of type %d", reset_count, type);
    return overall_err;
}

esp_err_t app_driver_reset_all_sensors(void)
{
    ESP_LOGI(TAG_MULTI_SENSOR, "=== Resetting all %d sensors ===", configured_sensors);
    
    esp_err_t overall_err = ESP_OK;
    uint8_t success_count = 0;
    uint8_t fail_count = 0;
    
    for (int i = 0; i < configured_sensors; i++) {
        ESP_LOGI(TAG_MULTI_SENSOR, "Resetting sensor %d/%d: %s", 
                i + 1, configured_sensors, sensors[i].config.name);
        
        esp_err_t err = app_driver_sensor_soft_reset(i);
        if (err != ESP_OK) {
            overall_err = err;
            fail_count++;
            ESP_LOGE(TAG_MULTI_SENSOR, "Failed to reset sensor %d: %d", i, err);
        } else {
            success_count++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
    if (overall_err == ESP_OK) {
        ESP_LOGI(TAG_MULTI_SENSOR, "All %d sensors reset successfully", success_count);
    } else {
        ESP_LOGW(TAG_MULTI_SENSOR, "Reset completed: %d success, %d failed", 
                success_count, fail_count);
    }
    
    ESP_LOGI(TAG_MULTI_SENSOR, "=================================");
    return overall_err;
}

void app_driver_log_sensor_statistics(void)
{
    uint64_t current_time_ms = esp_timer_get_time() / 1000;
    
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ === Sensor Statistics ===");
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Total sensors: %d", configured_sensors);
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Uptime: %llu minutes", current_time_ms / 60000);
    
    for (int i = 0; i < configured_sensors; i++) {
        sensor_data_t *sensor = &sensors[i];
        uint64_t ms_since_reset = current_time_ms - sensor_reset_tracker[i].last_reset_time;
        uint64_t hours_since_reset = ms_since_reset / 3600000;
        uint64_t minutes_since_reset = (ms_since_reset % 3600000) / 60000;
        
        const char* type_str = "Unknown";
        switch (sensor->config.type) {
            case SENSOR_TYPE_PIR: type_str = "PIR"; break;
            case SENSOR_TYPE_MICROWAVE: type_str = "Microwave"; break;
            case SENSOR_TYPE_ULTRASONIC: type_str = "Ultrasonic"; break;
            default: break;
        }
        
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Sensor %d: %s (%s), Resets=%lu, Last reset %llu:%02llu ago",
                i, sensor->config.name, type_str,
                sensor_reset_tracker[i].reset_count,
                hours_since_reset, minutes_since_reset);
    }
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ =========================");
}
