#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>

#include "sensor_driver_bme280.h"
#include "sensor_driver_bme680.h"
#include "sensor_driver_ds18b20.h"
#include "sensor_driver_dht11.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

// Sensor device structures
static bme280_dev_t bme280_sensor;
static bme680_dev_t bme680_sensor;
static ds18b20_dev_t ds18b20_sensor;
static dht11_dev_t dht11_sensor;

typedef struct {
    uint64_t last_reset_time;
    uint32_t reset_count;
    bool reset_in_progress;
} sensor_reset_tracker_t;

static sensor_reset_tracker_t sensor_reset_tracker[SENSOR_TYPE_MAX];

// Reset interval constants
#define RESET_INTERVAL_HOURS            24
#define MS_PER_HOUR                     (3600 * 1000)
#define RESET_INTERVAL_MS               (RESET_INTERVAL_HOURS * MS_PER_HOUR)

esp_err_t app_driver_sensor_init(const sensor_config_t* sensor_cfg)
{
    if (sensor_cfg == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor config is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t err = ESP_OK;

    switch (sensor_cfg->type) {
        case SENSOR_TYPE_BME280:
            err = bme280_init(&bme280_sensor, sensor_cfg->sda_pin, sensor_cfg->scl_pin, 
                              sensor_cfg->i2c_bus, sensor_cfg->i2c_addr);
            break;
        case SENSOR_TYPE_BME680:
            err = bme680_init(&bme680_sensor, sensor_cfg->sda_pin, sensor_cfg->scl_pin, 
                              sensor_cfg->i2c_bus, sensor_cfg->i2c_addr);
            break;
        case SENSOR_TYPE_DS18B20:
            err = ds18b20_init(&ds18b20_sensor, sensor_cfg->data_pin);
            break;
        case SENSOR_TYPE_DHT11:
            err = dht11_init(&dht11_sensor, sensor_cfg->data_pin);
            break;
        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown sensor type: %d", sensor_cfg->type);
            err = ESP_ERR_INVALID_ARG;
            break;
    }

    return err;
}

esp_err_t app_driver_read_sensor_data(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Invalid sensor index: %d", sensor_idx);
        return ESP_ERR_INVALID_ARG;
    }

    sensor_data_t *sensor = &sensors[sensor_idx];
    esp_err_t err = ESP_OK;

    switch (sensor->config.type) {
        case SENSOR_TYPE_BME280: {
            int16_t temperature;
            uint16_t humidity;
            int16_t pressure;
            
            err = bme280_read_all(&bme280_sensor, &temperature, &humidity, &pressure);
            if (err == ESP_OK) {
                sensor->last_temperature = temperature;
                sensor->last_humidity = humidity;
                sensor->last_pressure = pressure;
                sensor->last_read_time = esp_timer_get_time();
            }
            break;
        }
        
        case SENSOR_TYPE_BME680: {
            int16_t temperature;
            uint16_t humidity;
            int16_t pressure;
            uint32_t gas;
            
            err = bme680_read_all(&bme680_sensor, &temperature, &humidity, &pressure, &gas);
            if (err == ESP_OK) {
                sensor->last_temperature = temperature;
                sensor->last_humidity = humidity;
                sensor->last_pressure = pressure;
                sensor->last_gas_resistance = gas;
                sensor->last_read_time = esp_timer_get_time();
            }
            break;
        }
        
        case SENSOR_TYPE_DS18B20: {
            int16_t temperature;
            err = ds18b20_read_temperature(&ds18b20_sensor, &temperature);
            if (err == ESP_OK) {
                sensor->last_temperature = temperature;
                sensor->last_read_time = esp_timer_get_time();
            }
            break;
        }
        
        case SENSOR_TYPE_DHT11: {
            int16_t temperature;
            uint16_t humidity;
            err = dht11_read(&dht11_sensor, &temperature, &humidity);
            if (err == ESP_OK) {
                sensor->last_temperature = temperature;
                sensor->last_humidity = humidity;
                sensor->last_read_time = esp_timer_get_time();
            }
            break;
        }
        
        default:
            err = ESP_ERR_NOT_SUPPORTED;
            break;
    }

    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_SENSOR, "Failed to read sensor %d: %d", sensor_idx, err);
    }

    return err;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    // Sensors are read-only in this implementation
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
        case SENSOR_TYPE_BME280:
            err = bme280_reset(&bme280_sensor);
            break;
        case SENSOR_TYPE_BME680:
            err = bme680_reset(&bme680_sensor);
            break;
        case SENSOR_TYPE_DS18B20:
            err = ds18b20_reset(&ds18b20_sensor);
            break;
        case SENSOR_TYPE_DHT11:
            err = dht11_reset(&dht11_sensor);
            break;
        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown sensor type for reset: %d", sensor->config.type);
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

// Sensor polling task implementation
void sensor_polling_task(void *pvParameters)
{
    const TickType_t poll_period_ms = CONFIG_SENSOR_POLL_PERIOD_MS / portTICK_PERIOD_MS;
    
    // Initialize reset trackers
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
        
        // Check for periodic reset
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
        
        // Log statistics
        if (cycle_counter % STATS_LOG_INTERVAL == 0) {
            app_driver_log_sensor_statistics();
        }
        
        // Read all sensors
        for (int i = 0; i < configured_sensors; i++) {
            if (sensor_reset_tracker[i].reset_in_progress) {
                continue;
            }
            
            sensor_data_t *sensor = &sensors[i];
            uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
            
            esp_err_t read_err = app_driver_read_sensor_data(i);
            
            if (read_err == ESP_OK) {
                bool updated = false;
                
                // Update Matter attributes based on sensor type
                switch (sensor->config.type) {
                    case SENSOR_TYPE_BME280:
                    case SENSOR_TYPE_BME680: {
                        // Update Temperature
                        esp_matter_attr_val_t temp_val = esp_matter_int16((int16_t)sensor->last_temperature);
                        esp_err_t err = esp_matter::attribute::update(
                            endpoint_id,
                            TemperatureMeasurement::Id,
                            TemperatureMeasurement::Attributes::MeasuredValue::Id,
                            &temp_val
                        );
                        
                        if (err == ESP_OK) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            updated = true;
                        }
                        
                        // Update Humidity
                        esp_matter_attr_val_t hum_val = esp_matter_uint16((uint16_t)sensor->last_humidity);
                        err = esp_matter::attribute::update(
                            endpoint_id,
                            RelativeHumidityMeasurement::Id,
                            RelativeHumidityMeasurement::Attributes::MeasuredValue::Id,
                            &hum_val
                        );
                        
                        if (err == ESP_OK) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Humidity = %.2f%%", 
                                    i, sensor->last_humidity / 100.0f);
                        }
                        
                        // Update Pressure (if available)
                        if (sensor->last_pressure > 0) {
                            esp_matter_attr_val_t press_val = esp_matter_int16((int16_t)sensor->last_pressure);
                            err = esp_matter::attribute::update(
                                endpoint_id,
                                PressureMeasurement::Id,
                                PressureMeasurement::Attributes::MeasuredValue::Id,
                                &press_val
                            );
                            
                            if (err == ESP_OK) {
                                ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Pressure = %.2f hPa", 
                                        i, sensor->last_pressure / 100.0f);
                            }
                        }
                        break;
                    }
                    
                    case SENSOR_TYPE_DS18B20: {
                        esp_matter_attr_val_t temp_val = esp_matter_int16((int16_t)sensor->last_temperature);
                        esp_err_t err = esp_matter::attribute::update(
                            endpoint_id,
                            TemperatureMeasurement::Id,
                            TemperatureMeasurement::Attributes::MeasuredValue::Id,
                            &temp_val
                        );
                        
                        if (err == ESP_OK) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            updated = true;
                        }
                        break;
                    }
                    
                    case SENSOR_TYPE_DHT11: {
                        // Update Temperature
                        esp_matter_attr_val_t temp_val = esp_matter_int16((int16_t)sensor->last_temperature);
                        esp_err_t err = esp_matter::attribute::update(
                            endpoint_id,
                            TemperatureMeasurement::Id,
                            TemperatureMeasurement::Attributes::MeasuredValue::Id,
                            &temp_val
                        );
                        
                        if (err == ESP_OK) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                        }
                        
                        // Update Humidity
                        esp_matter_attr_val_t hum_val = esp_matter_uint16((uint16_t)sensor->last_humidity);
                        err = esp_matter::attribute::update(
                            endpoint_id,
                            RelativeHumidityMeasurement::Id,
                            RelativeHumidityMeasurement::Attributes::MeasuredValue::Id,
                            &hum_val
                        );
                        
                        if (err == ESP_OK) {
                            ESP_LOGD(TAG_MULTI_SENSOR, "Sensor %d: Humidity = %.2f%%", 
                                    i, sensor->last_humidity / 100.0f);
                        }
                        break;
                    }
                    
                    default:
                        break;
                }
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
    
    ESP_LOGI(TAG_MULTI_SENSOR, "Reset completed: %d success, %d failed", success_count, fail_count);
    return overall_err;
}

void app_driver_log_sensor_statistics(void)
{
    uint64_t current_time_ms = esp_timer_get_time() / 1000;
    
    ESP_LOGI(TAG_MULTI_SENSOR, "=== Sensor Statistics ===");
    ESP_LOGI(TAG_MULTI_SENSOR, "Total sensors: %d", configured_sensors);
    ESP_LOGI(TAG_MULTI_SENSOR, "Uptime: %llu minutes", current_time_ms / 60000);
    
    for (int i = 0; i < configured_sensors; i++) {
        sensor_data_t *sensor = &sensors[i];
        uint64_t ms_since_reset = current_time_ms - sensor_reset_tracker[i].last_reset_time;
        uint64_t hours_since_reset = ms_since_reset / 3600000;
        uint64_t minutes_since_reset = (ms_since_reset % 3600000) / 60000;
        
        const char* type_str = "Unknown";
        switch (sensor->config.type) {
            case SENSOR_TYPE_BME280: type_str = "BME280"; break;
            case SENSOR_TYPE_BME680: type_str = "BME680"; break;
            case SENSOR_TYPE_DS18B20: type_str = "DS18B20"; break;
            case SENSOR_TYPE_DHT11: type_str = "DHT11"; break;
            default: break;
        }
        
        ESP_LOGI(TAG_MULTI_SENSOR, 
                "Sensor %d: %s (%s), Resets=%lu, Last reset %llu:%02llu ago",
                i, sensor->config.name, type_str,
                sensor_reset_tracker[i].reset_count,
                hours_since_reset, minutes_since_reset);
    }
    ESP_LOGI(TAG_MULTI_SENSOR, "=========================");
}
