#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>

#include "sensor_driver_bme280.h"
#include "sensor_driver_bme680.h"
#include "sensor_driver_ds18b20.h"
#if CONFIG_DHT11_ENABLED
  #include "sensor_driver_dht11.h"
#endif

using namespace chip::app::Clusters;
using namespace esp_matter;

typedef struct {
    uint64_t last_reset_time;
    uint32_t reset_count;
    bool reset_in_progress;
} sensor_reset_tracker_t;

i2c_master_bus_handle_t bus_handle;

static sensor_reset_tracker_t sensor_reset_tracker[SENSOR_TYPE_MAX];

// Reset interval constants
#define RESET_INTERVAL_HOURS            24
#define MS_PER_HOUR                     (3600 * 1000)
#define RESET_INTERVAL_MS               (RESET_INTERVAL_HOURS * MS_PER_HOUR)

esp_err_t app_driver_sensor_init(sensor_type_t sensor_cfg)
{
    esp_err_t err = ESP_OK;

    switch (sensor_cfg) {
        case SENSOR_TYPE_BME280:
            err = bme280_init();
            break;
        case SENSOR_TYPE_BME680:
            err = bme680_init();
            break;
        case SENSOR_TYPE_DS18B20:
            err = ds18b20_init();
            break;
        #if CONFIG_DHT11_ENABLED
        case SENSOR_TYPE_DHT11:
            err = dht11_init();
            break;
        #endif
        default:
            ESP_LOGE(TAG_MULTI_CLIMATE, "Unknown sensor type: %d", sensor_cfg);
            err = ESP_ERR_INVALID_ARG;
            break;
    }

    return err;
}

esp_err_t app_driver_read_sensor_data(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Invalid sensor index: %d", sensor_idx);
        return ESP_ERR_INVALID_ARG;
    }

    sensor_data_t *sensor = &sensors[sensor_idx];
    esp_err_t err = ESP_OK;

    switch (sensor->config) {
        case SENSOR_TYPE_BME280: {
            int16_t temperature;
            uint16_t humidity;
            int16_t pressure;
            
            err = bme280_read_all(&temperature, &humidity, &pressure);
            if (err == ESP_OK) {
                sensor->last_temperature = temperature;
                sensor->last_humidity = humidity;
                sensor->last_pressure = pressure;
                sensor->last_read_time = esp_timer_get_time();

                #if CONFIG_SSD1306_ENABLED
                  ssd1306_show_sensor_data(3, temperature, humidity, pressure, 0);
                #endif
            }
            break;
        }
        
        case SENSOR_TYPE_BME680: {
            int16_t temperature;
            uint16_t humidity;
            int16_t pressure;
            uint32_t gas;
            
            err = bme680_read_all(&temperature, &humidity, &pressure, &gas);
            if(err == ESP_OK) {
              sensor->last_temperature = temperature;
              sensor->last_humidity = humidity;
              sensor->last_pressure = pressure;
              sensor->last_gas_resistance = gas;
              sensor->last_read_time = esp_timer_get_time();

              #if CONFIG_SSD1306_ENABLED
                ssd1306_show_sensor_data(4, temperature, humidity, pressure, gas);
              #endif
            }
            break;
        }
        
        case SENSOR_TYPE_DS18B20: {
            int16_t temperature;
            err = ds18b20_read(&temperature);
            if(err == ESP_OK) {
              sensor->last_temperature = temperature;
              sensor->last_read_time = esp_timer_get_time();

              #if CONFIG_SSD1306_ENABLED
                ssd1306_show_sensor_data(5, temperature, 0, 0, 0);
              #endif
            }
            break;
        }
        
        #if CONFIG_DHT11_ENABLED
        case SENSOR_TYPE_DHT11: {
            int16_t temperature;
            uint16_t humidity;
            err = dht11_read(&temperature, &humidity);
            if(err == ESP_OK) {
              sensor->last_temperature = temperature;
              sensor->last_humidity = humidity;
              sensor->last_read_time = esp_timer_get_time();

              #if CONFIG_SSD1306_ENABLED
                ssd1306_show_sensor_data(6, temperature, humidity, 0, 0);
              #endif
            }
            break;
        }
        #endif

        default:
            err = ESP_ERR_NOT_SUPPORTED;
            break;
    }

    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_CLIMATE, "Failed to read sensor %d: %d", sensor_idx, err);
    }

    return err;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
    // Sensors are read-only in this implementation
    return ESP_OK;
}

static esp_err_t app_driver_sensor_soft_reset(uint8_t sensor_idx)
{
    if (sensor_idx >= configured_sensors) {
        return ESP_ERR_INVALID_ARG;
    }
    
    sensor_data_t *sensor = &sensors[sensor_idx];
    ESP_LOGW(TAG_MULTI_CLIMATE, "Performing soft reset for sensor %d (%d)", 
            sensor_idx, sensor->config);
    
    esp_err_t err = ESP_OK;
    
    switch (sensor->config) {
        case SENSOR_TYPE_BME280:
            err = bme280_reset();
            break;
        case SENSOR_TYPE_BME680:
            err = bme680_reset();
            break;
        case SENSOR_TYPE_DS18B20:
            err = ds18b20_reset();
            break;
        #if CONFIG_DHT11_ENABLED
        case SENSOR_TYPE_DHT11:
            err = dht11_reset();
            break;
        #endif
        default:
            ESP_LOGE(TAG_MULTI_CLIMATE, "Unknown sensor type for reset: %d", sensor->config);
            err = ESP_ERR_NOT_SUPPORTED;
            break;
    }
    
    if (err == ESP_OK) {
        sensor_reset_tracker[sensor_idx].reset_count++;
        sensor_reset_tracker[sensor_idx].last_reset_time = esp_timer_get_time() / 1000;
        ESP_LOGI(TAG_MULTI_CLIMATE, "Sensor %d reset successful (total resets: %lu)", 
                sensor_idx, sensor_reset_tracker[sensor_idx].reset_count);
    } else {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Failed to reset sensor %d: %d", sensor_idx, err);
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
    //const uint32_t STATS_LOG_INTERVAL = 5000;
    
    ESP_LOGI(TAG_MULTI_CLIMATE, "Sensor polling task started with %d sensors", configured_sensors);
    
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
                    ESP_LOGI(TAG_MULTI_CLIMATE, 
                            "Scheduled reset for sensor %d (%d): %llu ms since last reset", 
                            i, sensors[i].config, time_since_last_reset);
                    
                    sensor_reset_tracker[i].reset_in_progress = true;
                    esp_err_t reset_err = app_driver_sensor_soft_reset(i);
                    sensor_reset_tracker[i].reset_in_progress = false;
                    
                    if (reset_err == ESP_OK) {
                        ESP_LOGI(TAG_MULTI_CLIMATE, "Scheduled reset completed for sensor %d", i);
                    }
                    
                    vTaskDelay(pdMS_TO_TICKS(200));
                }
            }
        }
        
        // Log statistics
        //if (cycle_counter % STATS_LOG_INTERVAL == 0) {
            app_driver_log_sensor_statistics();
        //}
        
        // Read all sensors
        for (int i = 0; i < configured_sensors; i++) {
            if (sensor_reset_tracker[i].reset_in_progress) {
                continue;
            }
            
            sensor_data_t *sensor = &sensors[i];
            uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
            
            esp_err_t read_err = app_driver_read_sensor_data(i);
            
            if (read_err == ESP_OK) {
                // Update Matter attributes based on sensor type
                switch (sensor->config) {
                    case SENSOR_TYPE_BME280: {
                        // Update Temperature
                        esp_matter_attr_val_t temp_val = esp_matter_int16((int16_t)sensor->last_temperature);
                        esp_err_t err = esp_matter::attribute::update(
                            endpoint_id,
                            TemperatureMeasurement::Id,
                            TemperatureMeasurement::Attributes::MeasuredValue::Id,
                            &temp_val
                        );
                        
                        if (err == ESP_OK) {
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #endif
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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #endif
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
                                #if DO_DEBUG
                                    ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Pressure = %.2f hPa", i, sensor->last_pressure);
                                #else
                                    ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Pressure = %.2f hPa", i, sensor->last_pressure);
                                #endif
                            }
                        }
                        break;
                    }

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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #endif
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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #endif
                        }
                        
                        // Update Pressure (if available)
                        //if (sensor->last_pressure > 0) {
                            esp_matter_attr_val_t press_val = esp_matter_int16((int16_t)sensor->last_pressure);
                            err = esp_matter::attribute::update(
                                endpoint_id,
                                PressureMeasurement::Id,
                                PressureMeasurement::Attributes::MeasuredValue::Id,
                                &press_val
                            );
                            
                            if (err == ESP_OK) {
                                #if DO_DEBUG
                                    ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Pressure = %.2f hPa", 
                                        i, sensor->last_pressure / 100.0f);
                                #else
                                    ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Pressure = %.2f hPa", 
                                        i, sensor->last_pressure / 100.0f);
                                #endif
                            }
                        //}
                        
                        // Update Gas Resistance
                        esp_matter_attr_val_t gas_val = esp_matter_float((float)sensor->last_gas_resistance);
                        err = esp_matter::attribute::update(
                            endpoint_id,
                            TotalVolatileOrganicCompoundsConcentrationMeasurement::Id,
                            TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id,
                            &gas_val
                        );
                        
                        if (err == ESP_OK) {
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Gas Resistance = %.0f Ohm", 
                                    i, sensor->last_gas_resistance);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Gas Resistance = %.0f Ohm", 
                                    i, sensor->last_gas_resistance);
                            #endif
                        } else {
                        	#if DO_DEBUG
                        		ESP_LOGE("", "");
                        		ESP_LOGE("", "*******************************");
                        		ESP_LOGE("", " BME680 Gas Resistance: WRONG!");
                        		ESP_LOGE("", "*******************************");
                        		ESP_LOGE("", "");
                        	#endif
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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #endif
                        }
                        break;
                    }
                    
                    #if CONFIG_DHT11_ENABLED
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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                                    i, sensor->last_temperature / 100.0f);
                            #endif
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
                            #if DO_DEBUG
                                ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #else
                                ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Humidity = %.2f %%", 
                                    i, sensor->last_humidity / 100.0f);
                            #endif
                        }
                        break;
                    }
                    #endif

                    default:
                        break;
                }
            }
        }
        
        vTaskDelay(poll_period_ms);
    }
}

void app_driver_log_sensor_statistics(void)
{
    uint64_t current_time_ms = esp_timer_get_time() / 1000;
    

    ESP_LOGW("", "");
    ESP_LOGW("", "=== SENSOR STATISTICS =====================================================");
    ESP_LOGW("", "");
    ESP_LOGW("", "Total sensors: %d", configured_sensors);
    ESP_LOGW("", "Uptime: %llu minutes", current_time_ms / 60000);
    ESP_LOGW("", "");
    ESP_LOGW("", "===========================================================================");
    ESP_LOGW("", "");
    
    for(int i = 0; i < configured_sensors; i++) {
        sensor_data_t *sensor = &sensors[i];
        
        /*
        ESP_LOGW("", 
                "Sensor %d: %s (%s), Resets=%lu, Last reset %llu:%02llu ago",
                i, sensor->config.name, type_str,
                sensor_reset_tracker[i].reset_count,
                hours_since_reset, minutes_since_reset);
        */
        switch(sensor->config) {
            case SENSOR_TYPE_BME280: {
                ESP_LOGW("", "BME280  (%d) | Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f hPa",
                i,
                sensor->last_temperature / 100.0f,
                sensor->last_humidity / 100.0f,
                sensor->last_pressure);
                break;
            }
            case SENSOR_TYPE_BME680: {
                ESP_LOGW("", "BME680  (%d) | Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f hPa, Gas: %d Ohm",
                i,
                sensor->last_temperature / 100.0f,
                sensor->last_humidity / 100.0f,
                sensor->last_pressure,
                (uint16_t)sensor->last_gas_resistance);
                break;
            }
            case SENSOR_TYPE_DS18B20: {
                ESP_LOGW("", "DS18B20 (%d) | Temp: %.2f °C",
                i,
                sensor->last_temperature / 100.0f);
                break;
            }
            #if CONFIG_DHT11_ENABLED
            case SENSOR_TYPE_DHT11: {
                ESP_LOGW("", "DHT11   (%d) | Temp: %.2f °C, Hum: %.2f %%",
                i,
                sensor->last_temperature / 100.0f,
                sensor->last_humidity / 100.0f);
                break;
            }
            #endif
            default: {
                break;
            }
        }
    }
    ESP_LOGW("", "");
    ESP_LOGW("", "===========================================================================");
    ESP_LOGW("", "");
}

// Инициализация I2C шины
void i2c_master_init(void)
{
  i2c_master_bus_config_t bus_cfg = {
    .i2c_port = I2C_NUM_1,
    .sda_io_num = (gpio_num_t)CONFIG_BME280_SDA_GPIO,
    .scl_io_num = (gpio_num_t)CONFIG_BME280_SCL_GPIO,
    .clk_source = I2C_CLK_SRC_DEFAULT,
    .glitch_ignore_cnt = 7,
    .flags = {
      .enable_internal_pullup = true,  // Включаем внутренние подтяжки
    },
  };
  
  ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &bus_handle));
  ESP_LOGI(TAG_MULTI_CLIMATE, "I2C master bus initialized");
}

// Добавление устройства на шину
i2c_master_dev_handle_t add_device(uint16_t address, const char *name)
{
  i2c_device_config_t dev_cfg = {
    .dev_addr_length = I2C_ADDR_BIT_LEN_7,
    .device_address = address,
    .scl_speed_hz = I2C_MASTER_BUS_FREQ_HZ,
  };
  
  i2c_master_dev_handle_t dev_handle;
  esp_err_t ret = i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);
  if(ret != ESP_OK) {
    ESP_LOGE(TAG_MULTI_CLIMATE, "Failed to add %s (0x%02X): %s", name, address, esp_err_to_name(ret));
    return NULL;
  }
  ESP_LOGI(TAG_MULTI_CLIMATE, "%s added at 0x%02X", name, address);
  return dev_handle;
}

// Проверка наличия устройства на шине
bool probe_device(uint16_t address)
{
  return i2c_master_probe(bus_handle, address, 100) == ESP_OK;
}
