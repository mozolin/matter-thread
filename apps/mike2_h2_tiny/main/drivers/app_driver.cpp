#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>


#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
#endif

#include "sensor_driver_temp.h"

using namespace chip::app::Clusters;
using namespace esp_matter;

esp_err_t app_driver_sensor_init()
{
  return temp_init();
}

esp_err_t app_driver_read_sensor_data()
{
    esp_err_t err = ESP_OK;

    sensor_data_t *sensor = &sensors[0];

    int16_t temperature;
    err = temp_read(&temperature);
    if(err == ESP_OK) {
      sensor->last_temperature = temperature;
      //sensor->last_read_time = esp_timer_get_time();
    }

    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_CLIMATE, "Failed to read sensor: %d", err);
    } else {
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
      #endif
    }

    return err;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  // Sensors are read-only in this implementation
  return ESP_OK;
}


// Sensor polling task implementation
void sensor_polling_task(void *pvParameters)
{
    printf("sensor_polling_task...\n");
    const TickType_t poll_period_ms = CONFIG_SENSOR_POLL_PERIOD_MS / portTICK_PERIOD_MS;
    
    ESP_LOGI(TAG_MULTI_CLIMATE, "Sensor polling task started with %d sensors", configured_sensors);
    
    while (true) {
        
        sensor_data_t *sensor = &sensors;
        uint16_t endpoint_id = sensor_mapping_list[0].endpoint_id;
        
        esp_err_t read_err = app_driver_read_sensor_data();
        
        if (read_err == ESP_OK) {
            // Update Matter attributes based on sensor type
                
            esp_matter_attr_val_t temp_val = esp_matter_int16((int16_t)sensor->last_temperature);
            esp_err_t err = esp_matter::attribute::update(
                endpoint_id,
                TemperatureMeasurement::Id,
                TemperatureMeasurement::Attributes::MeasuredValue::Id,
                &temp_val
            );
            
            if (err == ESP_OK) {
                #if DEBUG_MODE
                    ESP_LOGW(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                        i, sensor->last_temperature / 100.0f);
                #else
                    ESP_LOGD(TAG_MULTI_CLIMATE, "Sensor %d: Temperature = %.2f°C", 
                        i, sensor->last_temperature / 100.0f);
                #endif
            }
        }
        
        vTaskDelay(poll_period_ms);
    }
}
