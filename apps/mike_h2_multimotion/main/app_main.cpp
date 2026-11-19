#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_attribute.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#include "drivers/sensor_common.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "app_main_driver.h"
#include "i2c_adv.h"

#include "sensor_microwave.h"
#include "sensor_pir.h"
#include "sensor_ultrasonic.h"


#if CONFIG_BOOT_SWITCH_ENABLED
  #include "driver_boot_switch.h"
#endif

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
sensor_data_t sensors[CONFIG_NUM_SENSORS];


#if CONFIG_SHOW_DEVICE_LOG
  #include "plugin_device_structure.h"
#endif


static esp_err_t create_sensor_endpoint(sensor_config_t* sensor_cfg, node_t* node)
{
    esp_err_t err = ESP_OK;

    if (!node) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Matter node cannot be NULL");
        #endif
        return ESP_ERR_INVALID_ARG;
    }

    if (!sensor_cfg) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Sensor config cannot be NULL");
        #endif
        return ESP_ERR_INVALID_ARG;
    }

    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].sensor_type == sensor_cfg->type) {
            #if CONFIG_DEBUG_MODE
              ESP_LOGW(CONFIG_TAG_MIKE_APP, "Sensor type %d already configured", sensor_cfg->type);
            #endif
            return ESP_ERR_INVALID_STATE;
        }
    }

    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg->type) {
        case SENSOR_TYPE_PIR:
        case SENSOR_TYPE_MICROWAVE: {
            occupancy_sensor::config_t occupancy_config;
            occupancy_config.occupancy_sensing.occupancy = 0;
            
            if (sensor_cfg->type == SENSOR_TYPE_PIR) {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x00; // PIR
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 0);
            } else {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x01; // Ultrasonic
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 1);
            }
            
            occupancy_config.occupancy_sensing.features = 0x01;
            
            endpoint = occupancy_sensor::create(node, &occupancy_config, ENDPOINT_FLAG_NONE, sensor_cfg);
            break;
        }

        case SENSOR_TYPE_ULTRASONIC: {
            occupancy_sensor::config_t sensor_config;
            sensor_config.occupancy_sensing.occupancy = 0;
            sensor_config.occupancy_sensing.occupancy_sensor_type = 0x01; // Ultrasonic
            sensor_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 1);
            sensor_config.occupancy_sensing.features = 0x01;
            
            endpoint = occupancy_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, sensor_cfg);

            if (!endpoint) {
                #if CONFIG_DEBUG_MODE
                  ESP_LOGW(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for ultrasonic sensor");
                #endif
                return ESP_FAIL;
            }
            
            cluster::pressure_measurement::config_t pressure_config;
            
            const int16_t BASE_PRESSURE = 1000;
            const int16_t PRESSURE_PER_CM = 10;
            
            int16_t min_pressure = BASE_PRESSURE + (2 * PRESSURE_PER_CM);
            int16_t max_pressure = BASE_PRESSURE + (400 * PRESSURE_PER_CM);
            
            pressure_config.pressure_min_measured_value = nullable<int16_t>(min_pressure);
            pressure_config.pressure_max_measured_value = nullable<int16_t>(max_pressure);
            pressure_config.pressure_measured_value = BASE_PRESSURE + (100 * PRESSURE_PER_CM);
            
            esp_matter::cluster_t *pressure_cluster = 
                esp_matter::cluster::pressure_measurement::create(endpoint, &pressure_config, MATTER_CLUSTER_FLAG_SERVER);
            
            if (!pressure_cluster) {
                #if CONFIG_DEBUG_MODE
                  ESP_LOGW(CONFIG_TAG_MIKE_APP, "Failed to create Pressure Measurement cluster");
                #endif
            }
            break;
        }

        default:
            #if CONFIG_DEBUG_MODE
              ESP_LOGE(CONFIG_TAG_MIKE_APP, "Unknown sensor type: %d", sensor_cfg->type);
            #endif
            return ESP_ERR_INVALID_ARG;
    }

    if (!endpoint) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Matter endpoint creation failed for sensor type: %d", sensor_cfg->type);
        #endif
        return ESP_FAIL;
    }

    err = app_driver_sensor_init(sensor_cfg);
    if (err != ESP_OK) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize sensor type: %d", sensor_cfg->type);
        #endif
        return err;
    }

    if (configured_sensors < CONFIG_NUM_SENSORS) {
        sensor_mapping_list[configured_sensors].endpoint_id = endpoint::get_id(endpoint);
        sensor_mapping_list[configured_sensors].sensor_type = sensor_cfg->type;
        sensor_mapping_list[configured_sensors].primary_gpio = sensor_cfg->trigger_pin;
        sensor_mapping_list[configured_sensors].secondary_gpio = sensor_cfg->echo_pin;
        
        sensors[configured_sensors].config = *sensor_cfg;
        sensors[configured_sensors].last_state = false;
        sensors[configured_sensors].last_distance_cm = 0;
        sensors[configured_sensors].last_detection_time = 0;
        
        configured_sensors++;
        
        #if CONFIG_DEBUG_MODE
          ESP_LOGI(CONFIG_TAG_MIKE_APP, "Sensor %s created with endpoint_id %d", 
                sensor_cfg->name, endpoint::get_id(endpoint));
        #endif
    } else {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Maximum sensors configuration limit exceeded!");
        #endif
        return ESP_FAIL;
    }

    return err;
}

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;

  // 1. NVS — первым делом
  err = nvs_flash_init();
  if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    err = nvs_flash_erase();
    if(err != ESP_OK) {
    	return;
    }
  }
  err = nvs_flash_init();
  if(err != ESP_OK) {
  	#if CONFIG_DEBUG_MODE
  	  ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize NVS Flash: %d (%s)", err, esp_err_to_name(err));
  	#endif
  	return;
  }

  // 2. LED indicator
  #if CONFIG_USE_DRIVER_LED_INDICATOR
    led_indicator_init_main();
  #endif


  // 3. Reboot button
  #if CONFIG_REBOOT_BUTTON_ENABLED
    xTaskCreate(reboot_button_task, "reboot_button_task", 4096, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  #endif

  
  // 4. Console
  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    #if CONFIG_DEBUG_MODE
      ESP_LOGE("", "");
      ESP_LOGE("", "===========================================");
      ESP_LOGE("", " Free heap: %" PRIu32 ", Largest block: %d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      ESP_LOGE("", "===========================================");
      ESP_LOGE("", "");
    #endif
    err = esp_matter::console::init();
    if(err != ESP_OK) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Console init failed: %d", err);
      #endif
    }
  #endif


  // 5. Matter node + endpoints
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create Matter node"));
  
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!                                                              !!
//!!  Key endpoints must be created unconditionally -             !!
//!!  so as not to disrupt their order within the node structure  !!
//!!                                                              !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //-- HCSR501
  sensor_config_t pir_sensor = {
      .type = SENSOR_TYPE_PIR,
      .trigger_pin = (gpio_num_t)CONFIG_HCSR501_PIR_GPIO,
      .echo_pin = GPIO_NUM_NC,
      .endpoint_id = 0,
      .name = "PIR Sensor (HC-SR501)"
  };
  create_sensor_endpoint(&pir_sensor, node);

  //-- RCWL0516
  sensor_config_t microwave_sensor = {
      .type = SENSOR_TYPE_MICROWAVE,
      .trigger_pin = (gpio_num_t)CONFIG_RCWL0516_MICROWAVE_GPIO,
      .echo_pin = GPIO_NUM_NC,
      .endpoint_id = 0,
      .name = "Microwave Sensor (RCWL-0516)"
  };
  create_sensor_endpoint(&microwave_sensor, node);

  //-- HCSR04
  sensor_config_t ultrasonic_sensor = {
      .type = SENSOR_TYPE_ULTRASONIC,
      .trigger_pin = (gpio_num_t)CONFIG_HCSR04_TRIG_GPIO,
      .echo_pin = (gpio_num_t)CONFIG_HCSR04_ECHO_GPIO,
      .endpoint_id = 0,
      .name = "Ultrasonic Sensor (HC-SR04)"
  };
  create_sensor_endpoint(&ultrasonic_sensor, node);
  
  // 6. Boot Switch endpoint и On/Off эндпоинты
  #if CONFIG_BOOT_SWITCH_ENABLED
    create_boot_switch_endpoint(node);
    create_onoff_endpoints(node);
  #endif


  // 7. OpenThread config
  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif


  // 8. Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to start Matter, err:%d", err));

  
  // 9. Open commissioning windows after factory reset and reboot
  #if CONFIG_REBOOT_BUTTON_ENABLED
    open_commissioning_window_after_factory_reset();
  #endif

  
  // 10. Get FabricId & NodeId
  #if CONFIG_DEBUG_MODE
    if(node) {
      ESP_LOGW("", "");
      ESP_LOGW("", "=== FabricId & NodeId =====================");
      chip::FabricTable &fabricTable = chip::Server::GetInstance().GetFabricTable();
      for(auto iter = fabricTable.begin(); iter != fabricTable.end(); ++iter) {
        ESP_LOGW("", "FabricId 0x%llx NodeId 0x%llx", iter->GetFabricId(), iter->GetNodeId());
      }
      ESP_LOGW("", "===========================================");
      ESP_LOGW("", "");
    }
  #endif


  // 11. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  // 14. Запуск задачи опроса кнопки BOOT
  #if CONFIG_BOOT_SWITCH_ENABLED
    start_boot_switch_polling_task();
  #endif

  
  // 12. Выставляем OnOff статус
  // ====================================================
  // Явно выставляем OnOff = ON (true) для LED эндпоинта,
  // чтобы HA сразу видел разблокированное состояние.
  // ====================================================
  #if CONFIG_BOOT_SWITCH_ENABLED && CONFIG_USE_DRIVER_LED_INDICATOR
    set_led_onoff_attribute(true);
  #endif

  
  // 13. Polling task
  xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, CONFIG_SENSOR_POLL_TASK_PRIORITY, NULL);

  
  // 14. Log
  #if CONFIG_SHOW_DEVICE_LOG
    if(node) {
      vTaskDelay(pdMS_TO_TICKS(3000));
      log_device_structure(node);
    }
  #endif

}
