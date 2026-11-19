#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>
#include "sensor_common.h"

#include "app_main_driver.h"

#include "sensor_pir.h"
#include "sensor_microwave.h"
#include "sensor_ultrasonic.h"

#include "driver_led_esp32h2.h"
#if CONFIG_USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
#endif


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
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Sensor config is NULL");
      #endif
      return ESP_ERR_INVALID_ARG;
  }

  esp_err_t err = ESP_OK;

  switch (sensor_cfg->type) {
    #if CONFIG_HCSR501_ENABLED
    case SENSOR_TYPE_PIR:
      err = hcsr501_init_app(&pir_sensor, sensor_cfg->trigger_pin);
      break;
    #endif

    #if CONFIG_RCWL0516_ENABLED
    case SENSOR_TYPE_MICROWAVE:
      err = rcwl0516_init_app(&microwave_sensor, sensor_cfg->trigger_pin);
      break;
    #endif

    #if CONFIG_HCSR04_ENABLED
    case SENSOR_TYPE_ULTRASONIC:
      err = hcsr04_init_app(&ultrasonic_sensor, sensor_cfg->trigger_pin, sensor_cfg->echo_pin);
      //err = hcsr04_init1();
      break;
    #endif

    default:
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Unknown or disabled sensor type: %d", sensor_cfg->type);
      #endif
      err = ESP_ERR_INVALID_ARG;
      break;
  }

  return err;
}

static esp_err_t app_driver_sensor_soft_reset(uint8_t sensor_idx)
{
  if(sensor_idx >= configured_sensors) {
    return ESP_ERR_INVALID_ARG;
  }
  
  sensor_data_t *sensor = &sensors[sensor_idx];
  #if CONFIG_DEBUG_MODE
    ESP_LOGW(CONFIG_TAG_MIKE_APP, "Performing soft reset for sensor %d (%s)", 
          sensor_idx, sensor->config.name);
  #endif
  
  esp_err_t err = ESP_OK;
  
  switch (sensor->config.type) {
    #if CONFIG_HCSR501_ENABLED
    case SENSOR_TYPE_PIR:
      err = hcsr501_init_app(&pir_sensor, sensor->config.trigger_pin);
      break;
    #endif

    #if CONFIG_RCWL0516_ENABLED
    case SENSOR_TYPE_MICROWAVE:
      err = rcwl0516_init_app(&microwave_sensor, sensor->config.trigger_pin);
      break;
    #endif

    #if CONFIG_HCSR04_ENABLED
    /*
    case SENSOR_TYPE_ULTRASONIC:
        err = hcsr04_init_app(&ultrasonic_sensor, sensor->config.trigger_pin, sensor->config.echo_pin);
        //err = hcsr04_init();
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
    */
    #endif

    default:
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Unknown or disabled sensor type for reset: %d", sensor->config.type);
      #endif
      err = ESP_ERR_NOT_SUPPORTED;
      break;
  }
    
  if(err == ESP_OK) {
    sensor_reset_tracker[sensor_idx].reset_count++;
    sensor_reset_tracker[sensor_idx].last_reset_time = esp_timer_get_time() / 1000;
    #if CONFIG_DEBUG_MODE
      ESP_LOGI(CONFIG_TAG_MIKE_APP, "Sensor %d reset successful (total resets: %lu)", 
            sensor_idx, sensor_reset_tracker[sensor_idx].reset_count);
    #endif
  } else {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to reset sensor %d: %d", sensor_idx, err);
    #endif
  }
  
  return err;
}

void sensor_polling_task(void *pvParameters)
{
  //const TickType_t poll_period_ms = 1;
  const TickType_t poll_period_ms = CONFIG_SENSOR_POLL_PERIOD_MS / portTICK_PERIOD_MS;
  
  for(int i = 0; i < configured_sensors; i++) {
    sensor_reset_tracker[i].last_reset_time = esp_timer_get_time() / 1000;
    sensor_reset_tracker[i].reset_count = 0;
    sensor_reset_tracker[i].reset_in_progress = false;
  }
  
  uint32_t cycle_counter = 0;
  const uint32_t RESET_CHECK_INTERVAL = 100;
  
  #if CONFIG_DEBUG_MODE
    ESP_LOGI(CONFIG_TAG_MIKE_APP, "Sensor polling task started with %d sensors", configured_sensors);
  #endif
  
  while(true)
  {
    cycle_counter++;
    
    if(cycle_counter % RESET_CHECK_INTERVAL == 0) {
      uint64_t current_time_ms = esp_timer_get_time() / 1000;
      
      for(int i = 0; i < configured_sensors; i++) {
        if(sensor_reset_tracker[i].reset_in_progress) {
          continue;
        }
        
        uint64_t time_since_last_reset = current_time_ms - sensor_reset_tracker[i].last_reset_time;
        
        if(time_since_last_reset >= RESET_INTERVAL_MS) {
          #if CONFIG_DEBUG_MODE
            ESP_LOGI(CONFIG_TAG_MIKE_APP, 
                  "Scheduled reset for sensor %d (%s): %llu ms since last reset", 
                  i, sensors[i].config.name, time_since_last_reset);
          #endif
          
          sensor_reset_tracker[i].reset_in_progress = true;
          esp_err_t reset_err = app_driver_sensor_soft_reset(i);
          sensor_reset_tracker[i].reset_in_progress = false;
          
          if(reset_err == ESP_OK) {
            #if CONFIG_DEBUG_MODE
              ESP_LOGI(CONFIG_TAG_MIKE_APP, "Scheduled reset completed for sensor %d", i);
            #endif
          }
          
          vTaskDelay(pdMS_TO_TICKS(200));
        }
      }
    }
    
    // Log statistics
    #if CONFIG_DEBUG_MODE || CONFIG_ESP_EMULATOR_MODE
      app_driver_log_sensor_statistics();
    #endif

    for (int i = 0; i < configured_sensors; i++) {
      if (sensor_reset_tracker[i].reset_in_progress) {
          #if CONFIG_DEBUG_MODE
            ESP_LOGD(CONFIG_TAG_MIKE_APP, "Skipping sensor %d (reset in progress)", i);
          #endif
          continue;
      }
      
      sensor_data_t *sensor = &sensors[i];
      uint16_t endpoint_id = sensor_mapping_list[i].endpoint_id;
      
      switch(sensor->config.type) {
        #if CONFIG_HCSR501_ENABLED
        case SENSOR_TYPE_PIR: {
          bool current_state = hcsr501_read_app(&pir_sensor);

          static uint64_t last_pir_change = 0;
          uint64_t now = esp_timer_get_time();
          
          if (current_state != sensor->last_state) {
            if ((now - last_pir_change) < 2000000) {
              #if CONFIG_DEBUG_MODE
                ESP_LOGD(CONFIG_TAG_MIKE_APP, "PIR: Ignoring bounce");
              #endif
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
              #if CONFIG_DEBUG_MODE
                ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ HC-SR501 Sensor: motion %s (occupancy = %d)", 
                      current_state ? "detected" : "ended", occupancy_value);
              #endif
            } else {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to update HC-SR501 occupancy attribute: %d", update_err);
              #endif
            }
          }

          #if CONFIG_USE_DRIVER_LED_INDICATOR
            get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
          #endif

          break;
        }
        #endif

        #if CONFIG_RCWL0516_ENABLED
        case SENSOR_TYPE_MICROWAVE: {
          bool current_state = rcwl0516_read_app(&microwave_sensor);
          
          if (current_state != sensor->last_state) {
            sensor->last_state = current_state;
            sensor->last_detection_time = esp_timer_get_time();
            
            esp_matter_attr_val_t val = esp_matter_bool(current_state);
            esp_matter::attribute::update(endpoint_id, 
                                          OccupancySensing::Id, 
                                          OccupancySensing::Attributes::Occupancy::Id, 
                                          &val);
            
            #if CONFIG_DEBUG_MODE
              ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ RCWL-0516: state changed to %d", current_state);
            #endif
          }

          #if CONFIG_USE_DRIVER_LED_INDICATOR
            get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
          #endif

          break;
        }
        #endif

        #if CONFIG_HCSR04_ENABLED
        case SENSOR_TYPE_ULTRASONIC: {
          uint32_t current_distance = hcsr04_read_app(&ultrasonic_sensor);
          
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
              #if CONFIG_DEBUG_MODE
                ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ HC-SR04: Distance = %lu cm", current_distance);
              #endif
              
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
                  #if CONFIG_DEBUG_MODE
                    ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ HC-SR04: Motion %s (Δ %ld cm)", 
                          motion_detected ? "detected" : "ended",
                          last_distance_cm > 0 ? 
                              abs((int32_t)current_distance - (int32_t)last_distance_cm) : 0);
                  #endif
                } else {
                  #if CONFIG_DEBUG_MODE
                    ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to update occupancy: %d", occupancy_err);
                  #endif
                }
                
                sensor->last_state = motion_detected;
              }
            } else {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to update pressure attribute: %d", update_err);
              #endif
            }
          }

          #if CONFIG_USE_DRIVER_LED_INDICATOR
            get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
          #endif

          break;
        }
        #endif

        default: {
                break;
        }
      } // switch
    }
    
    vTaskDelay(poll_period_ms);
  }
}


#if CONFIG_DEBUG_MODE || CONFIG_ESP_EMULATOR_MODE
void app_driver_log_sensor_statistics(void)
{
  uint64_t current_time_ms = esp_timer_get_time() / 1000;
  
  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ === Sensor Statistics ===");
  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Total sensors: %d", configured_sensors);
  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Uptime: %llu minutes", current_time_ms / 60000);
  
  for(int i = 0; i < configured_sensors; i++) {
    sensor_data_t *sensor = &sensors[i];
    uint64_t ms_since_reset = current_time_ms - sensor_reset_tracker[i].last_reset_time;
    uint64_t hours_since_reset = ms_since_reset / 3600000;
    uint64_t minutes_since_reset = (ms_since_reset % 3600000) / 60000;
    
    const char* type_str = "Unknown";
    switch (sensor->config.type) {
      case SENSOR_TYPE_PIR: {
        type_str = "PIR";
        break;
      }

      case SENSOR_TYPE_MICROWAVE: {
        type_str = "Microwave";
        break;
      }
      
      case SENSOR_TYPE_ULTRASONIC: {
        type_str = "Ultrasonic";
        break;
      }
      default: {
        break;
      }
    }
    
    ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Sensor %d: %s (%s), Resets=%lu, Last reset %llu:%02llu ago",
      i, sensor->config.name, type_str,
      sensor_reset_tracker[i].reset_count,
      hours_since_reset, minutes_since_reset);
  }
  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ =========================");
}
#endif
