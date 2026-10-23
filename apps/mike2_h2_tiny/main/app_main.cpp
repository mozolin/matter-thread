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
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>


using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;

/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
/*
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  led_indicator_handle_t led_handle;
#endif
*/
#if USE_DRIVER_LED_INDICATOR
  led_indicator_handle_t led_handle;
#endif

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGW(TAG_TINY2, "~~~ Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGW(TAG_TINY2, "~~~ Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGW(TAG_TINY2, "~~~ Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG_TINY2, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGW(TAG_TINY2, "~~~ Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGW(TAG_TINY2, "~~~ Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGW(TAG_TINY2, "~~~ Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
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
    ESP_LOGW(TAG_TINY2, "~~~ Attribute pre-update");
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
        ESP_LOGW(TAG_TINY2, "~~~ NodeLabel set via ESP-Matter API");
    } else {
        ESP_LOGE(TAG_TINY2, "~~~ Failed to set NodeLabel: %d", err);
    }
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
      //=== ДОБАВЛЕНО: при старте LED выключен ===
      //-- Синхронизируем глобальный флаг с driver_led_indicator.cpp:
      //-- при загрузке функция get_led_indicator_blink_idx заблокирована,
      //-- поэтому любые попытки мигнуть будут игнорироваться.
      extern bool led_indicator_locked;   //-- определён в driver_led_indicator.cpp
      led_indicator_locked = true;

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
      //=== /ДОБАВЛЕНО ===
    }
  #endif

  
  // 4. SSD1306
  #if CONFIG_SSD1306_ENABLED
    //-- Init LCD SSD1306
    err = ssd1306_init();
    if(err != ESP_OK) {
      ESP_LOGE(TAG_TINY2, "~~~ ~~~ Error initialize SSD1306!");
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
    } else {
      ssd1306_initialized = true;
      ESP_LOGW(TAG_TINY2, "~~~ SSD1306 Initialized!");
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      #endif
    }
    //=== ИЗМЕНЕНО: при загрузке экран ВЫКЛЮЧЕН ===
    //ssd1306_show_title();                 // было: сразу показываем заголовок
    ssd1306_display_enabled = false;        // стало: экран выключен
    ssd1306_clear_screen(&ssd1306dev, false);
    //ssd1306_display_off(&ssd1306dev);       // гасим панель
    ssd1306_contrast(&ssd1306dev, 0x00);    // программное выключение панели
    //=== /ИЗМЕНЕНО ===
  #endif

  
  // 5. Reboot button
  #if CONFIG_REBOOT_BUTTON_ENABLED
    xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  #endif

    
  // 6. Console
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


  // 7. Matter node + endpoints
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_TINY2, "~~~ Failed to create Matter node"));

  
  // 8. OpenThread config
  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif

  
  // 9. Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_TINY2, "~~~ Failed to start Matter, err:%d", err));


  // 10. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();
  
}
