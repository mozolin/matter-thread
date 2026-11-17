
#include "app_main_driver.h"
#include "driver_boot_switch.h"
#include "driver_ssd1306_common.h"


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
//#include <app_priv.h>
#include "sdkconfig.h"
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

//--> uses for CommissioningWindowOpen
constexpr auto k_timeout_seconds = CONFIG_COMMISSIONING_WINDOW_OPEN;
//<-- uses for CommissioningWindowOpen


esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  // ============================================================
  //  Обработка On/Off для экрана, ленты и ребута из HA
  // ============================================================
  #if CONFIG_BOOT_SWITCH_ENABLED
    //-- Проверяем, не наш ли это эндпоинт
    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        //-- Экран
        #if CONFIG_SSD1306_ENABLED
        if (endpoint_id == s_ep_screen_onoff) {
            bool turn_on = val->val.b;
            if (ssd1306_initialized) {
                ssd1306_set_display_enabled(turn_on);
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "HA -> Screen %s", turn_on ? "ON" : "OFF");
                #endif
            }
            return ESP_OK;
        }
        #endif

        //-- LED лента (блокировка)
        #if USE_DRIVER_LED_INDICATOR
        if (endpoint_id == s_ep_led_onoff) {
            bool turn_on = val->val.b;
            //-- turn_on = true -> разблокировать (led_indicator_locked = false)
            //-- turn_on = false -> заблокировать (led_indicator_locked = true)
            extern bool led_indicator_locked;
            led_indicator_locked = !turn_on;
            #if DEBUG_MODE
              ESP_LOGI(TAG_MIKE_APP, "HA -> LED %s (locked=%d)", turn_on ? "ON" : "OFF", led_indicator_locked);
            #endif
            return ESP_OK;
        }
        #endif

        //-- Перезагрузка (On = немедленный reboot)
        if (endpoint_id == s_ep_reboot_onoff) {
            if (val->val.b) {
                #if DEBUG_MODE
                  ESP_LOGW(TAG_MIKE_APP, "HA -> Reboot requested, restarting in 1 sec...");
                #endif
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
            return ESP_OK;
        }
    }
  #endif
    
  // Sensors are read-only in this implementation
  return ESP_OK;
}


void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch (event->Type) {
  case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Interface IP Address changed");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning complete");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning failed, fail safe timer expired");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning session started");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning session stopped");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning window opened");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Commissioning window closed");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Fabric removed successfully");
    #endif
    if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
      chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
        if(err != CHIP_NO_ERROR) {
          #if DEBUG_MODE
            ESP_LOGE(TAG_MIKE_APP, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
          #endif
        } else {
        	ESP_LOGW("", "");
        	ESP_LOGW("", "----------------------------");
        	ESP_LOGW("", "Commissioning window opened!");
        	ESP_LOGW("", "----------------------------");
        	ESP_LOGW("", "");
        }
      }
    }
    break;
  }

  case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Fabric will be removed");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Fabric is updated");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Fabric is committed");
    #endif
    break;

  case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "BLE deinitialized and memory reclaimed");
    #endif
    break;

  default:
    break;
  }
}

esp_err_t app_identification_cb(esp_matter::identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  #if DEBUG_MODE
    ESP_LOGI(TAG_MIKE_APP, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  #endif
  return ESP_OK;
}

esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  esp_err_t err = ESP_OK;

  #if !ESP_EMULATOR_MODE
    if(type == PRE_UPDATE) {
      app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
      err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }
  #endif

  return err;
}

//--- Задача: раз в секунду мигать, пока открыто окно сопряжения ---
void commissioning_window_blink_task(void *arg)
{
  uint32_t window_sec = (uint32_t)(uintptr_t)arg;
  uint32_t elapsed = 0;

  #if DEBUG_MODE
    ESP_LOGW("#1# CommWindow", "");
    ESP_LOGW("#1# CommWindow", "##############################");
    ESP_LOGW("#1# CommWindow", "Blink task started, window = %lu s", window_sec);
    ESP_LOGW("#1# CommWindow", "##############################");
    ESP_LOGW("#1# CommWindow", "");
  #endif

  while(elapsed < window_sec) {
    // Проверяем, не закрылось ли окно раньше (например, устройство уже сопряжено)
    auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
    if(!mgr.IsCommissioningWindowOpen()) {
      #if DEBUG_MODE
        ESP_LOGW("#2# CommWindow", "");
        ESP_LOGW("#2# CommWindow", "##############################");
        ESP_LOGW("#2# CommWindow", "Window closed early at %lu s, stop blink", elapsed);
        ESP_LOGW("#2# CommWindow", "##############################");
        ESP_LOGW("#2# CommWindow", "");
      #endif
      break;
    }

    #if USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
    #endif

    vTaskDelay(pdMS_TO_TICKS(1000));
    elapsed++;
  }

  #if DEBUG_MODE
    ESP_LOGW("#3# CommWindow", "");
    ESP_LOGW("#3# CommWindow", "##############################");
    ESP_LOGW("#3# CommWindow", "Blink task finished after %lu s", elapsed);
    ESP_LOGW("#3# CommWindow", "##############################");
    ESP_LOGW("#3# CommWindow", "");
  #endif
  vTaskDelete(NULL);
}

#if CONFIG_REBOOT_BUTTON_ENABLED
  // 9. Open commissioning windows after factory reset and reboot
  void open_commissioning_window_after_factory_reset()
  {  
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
          #if DEBUG_MODE
            ESP_LOGE("", "");
            ESP_LOGE("", "-------------------------------------------------");
            ESP_LOGE("", "Failed to open commissioning window: %" CHIP_ERROR_FORMAT, err.Format());
            ESP_LOGE("", "-------------------------------------------------");
            ESP_LOGE("", "");
          #endif
        } else {
          #if DEBUG_MODE
            ESP_LOGW("", "");
            ESP_LOGW("", "------------------------------------------------------------");
            ESP_LOGW("", " Commissioning window opened after factory reset for %d s   ", CONFIG_COMMISSIONING_WINDOW_OPEN);
            ESP_LOGW("", "------------------------------------------------------------");
            ESP_LOGW("", "");
          #endif
        }
      } else {
      	// Запускаем задачу мигания (в стеке 2 KB достаточно)
        xTaskCreate(commissioning_window_blink_task, "comm_blink", 2048, (void *)(uintptr_t)CONFIG_COMMISSIONING_WINDOW_OPEN, CONFIG_COMMISSIONING_WINDOW_PRIORITY, NULL);
      }
    }, 0);
    //<-- uses for CommissioningWindowOpen
  }
#endif

void set_basic_attributes_esp_matter()
{
  #ifdef CONFIG_DEVICE_NODE_LABEL  
    uint16_t endpoint_id = 0x0000;

    // Set NodeLabel
    // BasicInformationCluster (ClusterId 40, 0x0028), Endpoint 0
    // AttributeId 5: NodeLabel
    char node_label[] = CONFIG_DEVICE_NODE_LABEL;
    esp_matter_attr_val_t node_label_val = esp_matter_char_str(node_label, strlen(node_label));
    esp_err_t err = esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
        &node_label_val
    );
    
    if(err == ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGI(TAG_MIKE_APP, "NodeLabel set via ESP-Matter API");
      #endif
    } else {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Failed to set NodeLabel: %d", err);
      #endif
    }
  #endif  
}
