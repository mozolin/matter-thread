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
#include "i2cdev.h"
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include "sensor_driver_temp.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

//--> uses for CommissioningWindowOpen
constexpr auto k_timeout_seconds = CONFIG_COMMISSIONING_WINDOW_OPEN;
//<-- uses for CommissioningWindowOpen
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[SENSOR_TYPE_MAX];
sensor_data_t sensors[SENSOR_TYPE_MAX];


#if USE_DRIVER_LED_INDICATOR
  led_indicator_handle_t led_handle;
#endif

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    #if USE_DRIVER_LED_INDICATOR
                      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
                    #endif
                    ESP_LOGE(TAG_TINY2, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGW(TAG_TINY2, "~~~ Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  esp_err_t err = ESP_OK;

  if(type == PRE_UPDATE) {
    #if USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
    #endif
    ESP_LOGW(TAG_TINY2, "~~~ Attribute pre-update");
    app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
    err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
  }

  return err;
}

static esp_err_t create_sensor_endpoint(node_t* node)
{
    esp_err_t err = ESP_OK;

    if(!node) {
      return ESP_ERR_INVALID_ARG;
    }

    endpoint_t *endpoint = NULL;
    
    // Create temperature sensor endpoint
    ESP_LOGW("", "");
    ESP_LOGW("", "*****************************************");
    ESP_LOGW("", " Start creating endpoint for Temperature ");
    ESP_LOGW("", "*****************************************");
    ESP_LOGW("", "");
    temperature_sensor::config_t sensor_config;
    
    sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
    sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-5500); // -55.00°C
    sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500); // 125.00°C
    
    endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);

    if(!endpoint) {
      #if USE_DRIVER_LED_INDICATOR
        //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
      ESP_LOGE(TAG_TINY2, "~~~ Matter endpoint creation failed for sensor!");
      return ESP_FAIL;
    }

    // Initialize sensor hardware
    err = app_driver_sensor_init();
    if(err != ESP_OK) {
      #if USE_DRIVER_LED_INDICATOR
        //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
      ESP_LOGE(TAG_TINY2, "~~~ Failed to initialize sensor!");
      return err;
    }

    // Store mapping
    if (configured_sensors < SENSOR_TYPE_MAX) {
        sensor_mapping_list[configured_sensors].endpoint_id = endpoint::get_id(endpoint);
        sensors[configured_sensors].last_temperature = 0;
        
        configured_sensors++;
        
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ Sensor created with endpoint_id %d", endpoint::get_id(endpoint));
    } else {
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        #endif
        ESP_LOGE(TAG_TINY2, "~~~ Maximum sensors configuration limit exceeded!");
        return ESP_FAIL;
    }

    return err;
}

void set_basic_attributes_esp_matter()
{
    uint16_t endpoint_id = 0x0000;

    // Set NodeLabel
    char node_label[] = "Tiny2_ESP32H2";
    esp_matter_attr_val_t node_label_val = esp_matter_char_str(node_label, strlen(node_label));
    esp_err_t err = esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
        &node_label_val
    );
    
    if(err == ESP_OK) {
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
        #endif
        ESP_LOGW(TAG_TINY2, "~~~ NodeLabel set via ESP-Matter API");
    } else {
        #if USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        #endif
        ESP_LOGE(TAG_TINY2, "~~~ Failed to set NodeLabel: %d", err);
    }
}

//--- Задача: раз в секунду мигать, пока открыто окно сопряжения ---
static void commissioning_window_blink_task(void *arg)
{
  uint32_t window_sec = (uint32_t)(uintptr_t)arg;
  uint32_t elapsed = 0;

  ESP_LOGW("#1# CommWindow", "");
  ESP_LOGW("#1# CommWindow", "##############################");
  ESP_LOGW("#1# CommWindow", "Blink task started, window = %lu s", window_sec);
  ESP_LOGW("#1# CommWindow", "##############################");
  ESP_LOGW("#1# CommWindow", "");

  while(elapsed < window_sec) {
    // Проверяем, не закрылось ли окно раньше (например, устройство уже сопряжено)
    auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
    if(!mgr.IsCommissioningWindowOpen()) {
      ESP_LOGW("#2# CommWindow", "");
      ESP_LOGW("#2# CommWindow", "##############################");
      ESP_LOGW("#2# CommWindow", "Window closed early at %lu s, stop blink", elapsed);
      ESP_LOGW("#2# CommWindow", "##############################");
      ESP_LOGW("#2# CommWindow", "");
      break;
    }

    #if USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
    #endif

    vTaskDelay(pdMS_TO_TICKS(1000));
    elapsed++;
  }

  ESP_LOGW("#3# CommWindow", "");
  ESP_LOGW("#3# CommWindow", "##############################");
  ESP_LOGW("#3# CommWindow", "Blink task finished after %lu s", elapsed);
  ESP_LOGW("#3# CommWindow", "##############################");
  ESP_LOGW("#3# CommWindow", "");
  vTaskDelete(NULL);
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
  	ESP_LOGE(TAG_TINY2, "~~~ Failed to initialize NVS Flash: %d (%s)", err, esp_err_to_name(err));
  	return;
  }

  
  // 2. i2cdev — до I2C устройств
  err = i2cdev_init();
  if(err != ESP_OK) {
    ESP_LOGE(TAG_TINY2, "~~~ Failed to initialize i2cdev: %d (%s)", err, esp_err_to_name(err));
    // Можно продолжить, но I2C устройства не будут работать.
  }

  
  // 3. LED indicator
  #if USE_DRIVER_LED_INDICATOR
    led_handle = configure_indicator();
    if(!led_handle) {
      ESP_LOGE(TAG_TINY2, "~~~ Failed to configure LED indicator");
    } else {
      extern bool led_indicator_locked;   //-- определён в driver_led_indicator.cpp
      led_indicator_locked = false;

      led_indicator_start(led_handle, BLINK_OFF);

      /*
      ESP_LOGE("", "");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", " USE_DRIVER_LED_INDICATOR = Y");
      #if USE_RGB_LED
        ESP_LOGE("", " USE_RGB_LED (%d) = Y", LED_BLINK_GPIO);
      #endif
      ESP_LOGE("", " LED state at boot: OFF (locked)");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", "");
      */
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
    }
  #endif

  // 4. Reboot button
  #if CONFIG_REBOOT_BUTTON_ENABLED
    xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  #endif

    
  // 5. Console
  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    ESP_LOGE("", "");
    ESP_LOGE("", "===========================================");
    ESP_LOGE("", " Free heap: %" PRIu32 ", Largest block: %d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    ESP_LOGE("", "===========================================");
    ESP_LOGE("", "");
    err = esp_matter::console::init();
    if(err != ESP_OK) {
      ESP_LOGE(TAG_TINY2, "~~~ Console init failed: %d", err);
    }
  #endif


  // 6. Matter node + endpoint
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_TINY2, "~~~ Failed to create Matter node"));
  create_sensor_endpoint(node);

  
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
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_TINY2, "~~~ Failed to start Matter, err:%d", err));

  // 9. Open commissioning windows after factory reset and reboot
  #if CONFIG_REBOOT_BUTTON_ENABLED
    //--> uses for CommissioningWindowOpen
    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t context) {
      auto &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      // Check if the pairing window is already open, avoid opening it again
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(
          // Window opens for CONFIG_COMMISSIONING_WINDOW_OPEN seconds
          chip::System::Clock::Seconds16(CONFIG_COMMISSIONING_WINDOW_OPEN)
        );
        if(err != CHIP_NO_ERROR) {
          ESP_LOGE("", "");
          ESP_LOGE("", "-------------------------------------------------");
          ESP_LOGE("", "Failed to open commissioning window: %" CHIP_ERROR_FORMAT, err.Format());
          ESP_LOGE("", "-------------------------------------------------");
          ESP_LOGE("", "");
        } else {
          ESP_LOGW("", "");
          ESP_LOGW("", "------------------------------------------------------------");
          ESP_LOGW("", " Commissioning window opened after factory reset for %d s   ", CONFIG_COMMISSIONING_WINDOW_OPEN);
          ESP_LOGW("", "------------------------------------------------------------");
          ESP_LOGW("", "");
        }
      } else {
      	// Запускаем задачу мигания (в стеке 2 KB достаточно)
        xTaskCreate(commissioning_window_blink_task, "comm_blink", 2048, (void *)(uintptr_t)CONFIG_COMMISSIONING_WINDOW_OPEN, CONFIG_COMMISSIONING_WINDOW_PRIORITY, NULL);
      }
    }, 0);
    //<-- uses for CommissioningWindowOpen
  #endif
  
  // 10. Get FabricId & NodeId
  ESP_LOGW("", "");
  ESP_LOGW("", "===========================================");
  chip::FabricTable &fabricTable = chip::Server::GetInstance().GetFabricTable();
  for(auto iter = fabricTable.begin(); iter != fabricTable.end(); ++iter) {
    ESP_LOGW("", "FabricId 0x%llx NodeId 0x%llx", iter->GetFabricId(), iter->GetNodeId());
  }
  ESP_LOGW("", "===========================================");
  ESP_LOGW("", "");

  // 11. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();
  
  // 12. Polling task
  xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, CONFIG_SENSOR_POLL_TASK_PRIORITY, NULL);

  #if USE_DRIVER_LED_INDICATOR
    /*
    get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
    get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
    get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
    get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
    get_led_indicator_blink_idx(BLINK_ONCE_PURPLE, 60, 0);
    */
  #endif

}
