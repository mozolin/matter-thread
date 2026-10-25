/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                                                                  !!
!! The ESP32-H2, like other ESP32 variants, can experience panics   !!
!! when using floating-point operations, especially in interrupt    !!
!! service routines (ISRs). This is because the ESP32-H2 doesn't    !!
!! have a hardware floating-point unit (FPU). When float operations !!
!! are performed outside of the main application context (like in   !!
!! an ISR), it can lead to unexpected behavior and crashes.         !!
!!                                                                  !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
//#include "driver/gpio.h"
//#include "soc/gpio_num.h"

#include <common_macros.h>
#include <app_priv.h>
//#include <app_reset.h>
#include "i2cdev.h"
#include <platform/ESP32/OpenthreadLauncher.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>


#define CREATE_ALL_GPIO_ONOFF_PLUGS(node) \
  for(const auto &relay : relays) { \
    struct gpio_plug plug; \
    plug.GPIO_PIN_VALUE = (gpio_num_t)relay.gpio_pin; \
    create_plug(&plug, node); \
  }
    
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;


//--> uses for CommissioningWindowOpen
constexpr auto k_timeout_seconds = CONFIG_COMMISSIONING_WINDOW_OPEN;
//<-- uses for CommissioningWindowOpen
uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

#if USE_DRIVER_LED_INDICATOR
  led_indicator_handle_t led_handle;
#endif

#if CONFIG_SSD1306_ENABLED
  //-- SSD1306 device instance
  SSD1306_t ssd1306dev;
  //-- Is SSD1306 initialized?
  bool ssd1306_initialized = false;
  //=== состояние дисплея (по умолчанию — ВЫКЛЮЧЕН) ===
  bool ssd1306_display_enabled = false;
  uint64_t ssd1306_last_on_time_ms = 0;

  //-- callback function specific to this application
  //-- (can be used within the SSD1306 driver)
  #ifdef CONFIG_SSD1306_MAIN_RUN_CB
    void ssd1306_main_run_cb(void)
    {
	    relay_init();
    }
  #endif

#endif

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch(event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Interface IP Address changed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning complete");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
      ESP_LOGE(TAG_MIKE_APP, "~~~ Commissioning failed, fail safe timer expired");
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session started");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session stopped");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window opened");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window closed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Binding entry changed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric removed successfully");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
        if(!commissionMgr.IsCommissioningWindowOpen()) {
          //-- After removing last fabric, this example does not remove the Wi-Fi credentials
          //-- and still has IP connectivity so, only advertising on DNS-SD.
          CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
          if(err != CHIP_NO_ERROR) {
            ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
            //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
          }
        }
      }
      break;
    }
    
    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric will be removed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is updated");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is committed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
      ESP_LOGW(TAG_MIKE_APP, "~~~ BLE deinitialized and memory reclaimed");
      //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    default:
      break;
  }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGW(TAG_MIKE_APP, "~~~ Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
  return ESP_OK;
}

//-- This callback is called for every attribute update. The callback implementation shall
//-- handle the desired attributes and return an appropriate error code. If the attribute
//-- is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  ESP_LOGE(TAG_MIKE_APP, "~~~ app_attribute_update_cb():type=%d,ep=0x%04" PRIX16 ",cluster=0x%08" PRIX32 ",attr=0x%08" PRIX32, (int)type, endpoint_id, cluster_id, attribute_id);
  
  esp_err_t err = ESP_OK;

  switch(type) {
    case PRE_UPDATE: {
    	ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::PRE_UPDATE");
        //-- Driver update
        bool state = val->val.b;
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
        if(err == ESP_OK) {
          ESP_LOGW(TAG_MIKE_APP, "~~~ Updated: endpoint:0x%04" PRIX16 "|cluster:0x%08" PRIX32 "|attribute:0x%08" PRIX32 "|state:%d", endpoint_id, cluster_id, attribute_id, (int)state);
          //-- Blink...
          //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
        } else {
          ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to update attribute: 0x%04" PRIX16 "|0x%08" PRIX32 "|0x%08" PRIX32 "|%d", endpoint_id, cluster_id, attribute_id, (int)state);
          //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        }
      }
      break;
  
    case POST_UPDATE:
      ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::POST_UPDATE");
      break;
  
    case READ:
      ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::READ");
      #if ADD_CUSTOM_CLUSTERS
	      //-- Callback for reading custom attributes
  	    read_custom_attribute_cb(type, endpoint_id, cluster_id, attribute_id, val, priv_data);
      #endif
      break;
  
    case WRITE:
    	ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::WRITE");
      break;
  }
  
  return err;
}

//-- Creates plug-endpoint mapping for each GPIO pin configured.
static esp_err_t create_plug(gpio_plug* plug, node_t* node)
{
  esp_err_t err = ESP_OK;

  if(!node) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter node cannot be NULL");
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_ARG;
  }

  if(!plug) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Plug cannot be NULL");
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_ARG;
  }

  //-- Check if plug's IO pin is already used by reset button
  if((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_STATE;
  }

  //-- Check for plug if already configured.
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      return ESP_ERR_INVALID_STATE;
    }
  }

  on_off_plugin_unit::config_t plugin_unit_config;
  plugin_unit_config.on_off.on_off = false;
  endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

  if(!endpoint) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter endpoint creation failed");
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_FAIL;
  }

  //-- GPIO pin Initialization
  err = app_driver_plugin_unit_init(plug);

  if(err != ESP_OK) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to initialize plug");
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
  }

  //-- Check for maximum plugs that can be configured.
  if(configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
    plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
    plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
    configure_plugs++;
  } else {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Maximum plugs configuration limit exceeded!");
    //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_FAIL;
  }

  uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
  ESP_LOGW(TAG_MIKE_APP, "~~~ Plug created with endpoint_id %d", plug_endpoint_id);
  //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
  return err;
}

void set_basic_attributes_esp_matter()
{
  uint16_t endpoint_id = 0x0000;

  #if UART_MONITOR_DEBUG
	  ESP_LOGW("", "");
  	ESP_LOGW("", "##################################");
	  ESP_LOGW("", "#");
  #endif
  
  //-- Set NodeLabel
  char node_label[] = CONFIG_CUSTOM_DEVICE_NODE_LABEL;
  esp_matter_attr_val_t node_label_val = esp_matter_char_str(node_label, strlen(node_label));
  esp_err_t err = esp_matter::attribute::update(
    endpoint_id,
    chip::app::Clusters::BasicInformation::Id,
    chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
    &node_label_val
  );
	if(err == ESP_OK) {
	  #if UART_MONITOR_DEBUG
	    ESP_LOGW(TAG_MIKE_APP, "~~~ NodeLabel set via ESP-Matter API");
	  #endif
	} else {
	  #if UART_MONITOR_DEBUG
	    ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to set NodeLabel: %d", err);
	  #endif
	}
  
  
  
  //-- Set Location (unfortunately, it is not yet included in the cluster definition)
  char location[] = CONFIG_CUSTOM_DEVICE_LOCATION;
  esp_matter_attr_val_t location_val = esp_matter_char_str(location, strlen(location));
  /*
  //-- !!! TRYING TO GET ITS VALUE !!!
  cluster_t *cluster = cluster::get(endpoint_id, chip::app::Clusters::BasicInformation::Id);
  if(!cluster) {
  	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to get cluster");
  } else {
  	ESP_LOGI(TAG_MIKE_APP, "~~~ Custom cluster 0x%08" PRIX32 " initialized at endpoint 0x%04" PRIX16, chip::app::Clusters::BasicInformation::Id, endpoint_id);
  	
  	attribute_t *attribute = attribute::get(endpoint_id, chip::app::Clusters::BasicInformation::Id, chip::app::Clusters::BasicInformation::Attributes::Location::Id);
  	if(!attribute) {
  		ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to get Location attribute!");
		} else {
			ESP_LOGW(TAG_MIKE_APP, "~~~ Location attribute received!");
		}
  	
  	attribute_t *location_attr = cluster::basic_information::attribute::create_location(cluster, location, strlen(location));
  	if(!location_attr) {
  		ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create Location attribute!");
		}
  }
  */
  err = esp_matter::attribute::update(
    endpoint_id,
    chip::app::Clusters::BasicInformation::Id,
    chip::app::Clusters::BasicInformation::Attributes::Location::Id,
    &location_val
  );
	if(err == ESP_OK) {
    #if UART_MONITOR_DEBUG
  		ESP_LOGW(TAG_MIKE_APP, "~~~ Location set via ESP-Matter API");
  	#endif
	} else {
	  #if UART_MONITOR_DEBUG
	  	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to set Location: %d", err);
	  #endif
	}

	#if UART_MONITOR_DEBUG  
	  ESP_LOGW("", "#");
  	ESP_LOGW("", "##################################");
	  ESP_LOGW("", "");
  #endif

}

void print_attribute_ids()
{
  ESP_LOGW("", "");
  ESP_LOGW("", "##################################");
  ESP_LOGW("", "#");
  ESP_LOGW("", "#   NodeLabel Attribute ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id);
  ESP_LOGW("", "#   Location Attribute ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Attributes::Location::Id);
  ESP_LOGW("", "#   Basic Information Cluster ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Id);
  ESP_LOGW("", "#");
  ESP_LOGW("", "##################################");
  ESP_LOGW("", "");
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
  	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to initialize NVS Flash: %d (%s)", err, esp_err_to_name(err));
  	return;
  }
  
  // 2. i2cdev — до I2C устройств
  err = i2cdev_init();
  if(err != ESP_OK) {
    #if DEBUG_MODE
      ESP_LOGE(TAG_MIKE_APP, "Failed to initialize i2cdev: %d (%s)", err, esp_err_to_name(err));
    #endif
    // Можно продолжить, но I2C устройства не будут работать.
  }
  
  // 3. LED indicator
  #if USE_DRIVER_LED_INDICATOR
    led_handle = configure_indicator();
    if(!led_handle) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Failed to configure LED indicator");
      #endif
    } else {
      //=== при старте LED выключен ===
      //-- Синхронизируем глобальный флаг с driver_led_indicator.cpp:
      //-- при загрузке функция get_led_indicator_blink_idx заблокирована,
      //-- поэтому любые попытки мигнуть будут игнорироваться.
      extern bool led_indicator_locked;   //-- определён в driver_led_indicator.cpp
      //led_indicator_locked = true;
      led_indicator_locked = false;

      //-- Гасим RGB-ленту через штатный шаг BLINK_OFF.
      //-- ВАЖНО: используем именно BLINK_OFF, а не BLINK_MAX —
      //-- BLINK_MAX это размер массива led_mode[], а не валидный индекс.
      led_indicator_start(led_handle, BLINK_OFF);

      ESP_LOGE("", "");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", " USE_DRIVER_LED_INDICATOR = Y");
      #if USE_RGB_LED
        ESP_LOGE("", " USE_RGB_LED (%d) = Y", LED_BLINK_GPIO);
      #endif
      ESP_LOGE("", " LED state at boot: OFF (locked)");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", "");
    }
  #endif


  // 4. SSD1306
  #if CONFIG_SSD1306_ENABLED
    //-- Init LCD SSD1306
    err = ssd1306_i2cdev_init();
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "~~~ Error initialize SSD1306!");
      #endif
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
    } else {
      ssd1306_initialized = true;
      #if DEBUG_MODE
        ESP_LOGW(TAG_MIKE_APP, "~~~ SSD1306 Initialized!");
      #endif
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      #endif
    }
    //=== ИЗМЕНЕНО: при загрузке экран ВЫКЛЮЧЕН ===
    ssd1306_display_enabled = false;        // стало: экран выключен
    ssd1306_clear_screen(&ssd1306dev, false);
    ssd1306_contrast(&ssd1306dev, 0x00);    // программное выключение панели
    //=== /ИЗМЕНЕНО ===
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
      ESP_LOGE(TAG_MIKE_APP, "~~~ Console init failed: %d", err);
    }
  #endif

  
  // 7. Matter node + endpoints
  //-- Create a Matter node and add the mandatory Root Node device type on endpoint 0
  node::config_t node_config;
  //-- node handle can be used to add/modify other endpoints.
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create Matter node"));
  //-- Creates plug-endpoint mapping for all GPIO pins configured.
  CREATE_ALL_GPIO_ONOFF_PLUGS(node);


  // 8. OpenThread config
  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    //-- Set OpenThread platform config
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif

  // 9. Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to start Matter, err:%d", err));


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


  // 10. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  if(err == ESP_OK) {
    relay_init();
  }

  //-- Setting BasicInformationCluster attributes
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  #if LIVE_BLINK_TIME_MS > 0
	  //-- Start init indicator task
  	xTaskCreate(init_indicator_task, "init_indicator_task", 2048, NULL, 6, NULL);
  #endif

}
