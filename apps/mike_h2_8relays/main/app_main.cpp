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

#include "app_main_driver.h"
#include "i2c_adv.h"

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


uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

#if CONFIG_SHOW_DEVICE_LOG
  #include "plugin_device_structure.h"
#endif


//-- Creates plug-endpoint mapping for each GPIO pin configured.
static esp_err_t create_plug(gpio_plug* plug, node_t* node)
{
  esp_err_t err = ESP_OK;

  if(!node) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Matter node cannot be NULL");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_ERR_INVALID_ARG;
  }

  if(!plug) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Plug cannot be NULL");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_ERR_INVALID_ARG;
  }

  //-- Check if plug's IO pin is already used by reset button
  if((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_ERR_INVALID_STATE;
  }

  //-- Check for plug if already configured.
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
      #endif
      #if CONFIG_USE_DRIVER_LED_INDICATOR
        //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
      return ESP_ERR_INVALID_STATE;
    }
  }

  on_off_plugin_unit::config_t plugin_unit_config;
  plugin_unit_config.on_off.on_off = false;
  endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

  if(!endpoint) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Matter endpoint creation failed");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_FAIL;
  }

  //-- GPIO pin Initialization
  err = app_driver_plugin_unit_init(plug);

  if(err != ESP_OK) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Failed to initialize plug");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
  }

  //-- Check for maximum plugs that can be configured.
  if(configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
    plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
    plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
    configure_plugs++;
  } else {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Maximum plugs configuration limit exceeded!");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_FAIL;
  }

  uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
  #if CONFIG_DEBUG_MODE
    ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Plug created with endpoint_id %d", plug_endpoint_id);
  #endif
  #if CONFIG_USE_DRIVER_LED_INDICATOR
    //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
  #endif
  return err;
}


void print_attribute_ids()
{
  #if CONFIG_DEBUG_MODE
    ESP_LOGW("", "");
    ESP_LOGW("", "##################################");
    ESP_LOGW("", "#");
    ESP_LOGW("", "#   NodeLabel Attribute ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id);
    ESP_LOGW("", "#   Location Attribute ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Attributes::Location::Id);
    ESP_LOGW("", "#   Basic Information Cluster ID: 0x%04" PRIX32, chip::app::Clusters::BasicInformation::Id);
    ESP_LOGW("", "#");
    ESP_LOGW("", "##################################");
    ESP_LOGW("", "");
  #endif
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

  
  // 2. i2cdev — до I2C устройств
  err = i2cdev_init();
  if(err != ESP_OK) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize i2cdev: %d (%s)", err, esp_err_to_name(err));
    #endif
    // Можно продолжить, но I2C устройства не будут работать.
  }

  
  // 3. LED indicator
  #if CONFIG_USE_DRIVER_LED_INDICATOR
    led_indicator_init_main();
  #endif

  
  // 4. SSD1306
  #if CONFIG_SSD1306_ENABLED
    //-- Init LCD SSD1306
    ssd1306_init_app_main();
  #endif

  
  // 5. Reboot button
  #if CONFIG_REBOOT_BUTTON_ENABLED
    xTaskCreate(reboot_button_task, "reboot_button_task", 4096, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  #endif

    
  // 6. Console
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


  // 7. Matter node + endpoints
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_main_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create Matter node"));

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!                                                              !!
//!!  Key endpoints must be created unconditionally -             !!
//!!  so as not to disrupt their order within the node structure  !!
//!!                                                              !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  //-- Creates plug-endpoint mapping for all GPIO pins configured.
  CREATE_ALL_GPIO_ONOFF_PLUGS(node);


  // 8. Boot Switch endpoint и On/Off эндпоинты
  #if CONFIG_BOOT_SWITCH_ENABLED
    create_boot_switch_endpoint(node);
    create_onoff_endpoints(node);
  #endif

  
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
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Failed to start Matter, err:%d", err));


  // 11. Open commissioning windows after factory reset and reboot
  #if CONFIG_REBOOT_BUTTON_ENABLED
    open_commissioning_window_after_factory_reset();
  #endif
  

  // 12. Get FabricId & NodeId
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


  // 10. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  // 14. Запуск задачи опроса кнопки BOOT
  #if CONFIG_BOOT_SWITCH_ENABLED
    start_boot_switch_polling_task();
  #endif

  
  // 15. Выставляем OnOff статус
  // ====================================================
  // Явно выставляем OnOff = ON (true) для LED эндпоинта,
  // чтобы HA сразу видел разблокированное состояние.
  // ====================================================
  #if CONFIG_BOOT_SWITCH_ENABLED && CONFIG_USE_DRIVER_LED_INDICATOR
    set_led_onoff_attribute(true);
  #endif

  
  // Relays initialization
  if(err == ESP_OK) {
    relay_init();
  }


  // 17. Log
  #if CONFIG_SHOW_DEVICE_LOG
    if(node) {
      vTaskDelay(pdMS_TO_TICKS(3000));
      log_device_structure(node);
    }
  #endif


  // 18. Check I2C ports
  #if CONFIG_CHECK_I2C_PORTS
    check_i2c_ports();
  #endif


}
