
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
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

#define TAG_H2 "Mike H2"

#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  led_indicator_handle_t led_handle;
#endif

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch (event->Type) {
  case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
    ESP_LOGI(TAG_H2, "~~~ Interface IP Address changed");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
    ESP_LOGI(TAG_H2, "~~~ Commissioning complete");
    break;

  case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
    ESP_LOGI(TAG_H2, "~~~ Commissioning failed, fail safe timer expired");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
    ESP_LOGI(TAG_H2, "~~~ Commissioning session started");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
    ESP_LOGI(TAG_H2, "~~~ Commissioning session stopped");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
    ESP_LOGI(TAG_H2, "~~~ Commissioning window opened");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
    ESP_LOGI(TAG_H2, "~~~ Commissioning window closed");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
    ESP_LOGI(TAG_H2, "~~~ Fabric removed successfully");
    if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
      chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        /* After removing last fabric, this example does not remove the Wi-Fi credentials
         * and still has IP connectivity so, only advertising on DNS-SD.
         */
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
        if(err != CHIP_NO_ERROR) {
          ESP_LOGE(TAG_H2, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
        }
      }
    }
    break;
  }

  case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
    ESP_LOGI(TAG_H2, "~~~ Fabric will be removed");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
    ESP_LOGI(TAG_H2, "~~~ Fabric is updated");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
    ESP_LOGI(TAG_H2, "~~~ Fabric is committed");
    break;

  case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
    ESP_LOGI(TAG_H2, "~~~ BLE deinitialized and memory reclaimed");
    break;

  default:
      break;
  }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGI(TAG_H2, "~~~ Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  return ESP_OK;
}

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;

  /* Initialize the ESP NVS layer */
  nvs_flash_init();

  /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
  node::config_t node_config;

  // node handle can be used to add/modify other endpoints.
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_H2, "~~~ Failed to create Matter node"));

	#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
	#endif

  /* Matter start */
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_H2, "~~~ Failed to start Matter, err:%d", err));

	#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
		#if CONFIG_OPENTHREAD_CLI
    	esp_matter::console::otcli_register_commands();
		#endif
  	esp_matter::console::init();
	#endif
	
	//ESP_LOGW(TAG_H2, "~~~ USE_DRIVER_LED_INDICATOR: %d", USE_DRIVER_LED_INDICATOR);

  #if USE_DRIVER_LED_INDICATOR
    //-- at least one LED must not be an RGB LED
    #if USE_ORDINARY_LED
      //--> UART RX/TX Blinking Simulator
      #if LED_MODE == 1
        #if DEBUG_MODE
          ESP_LOGW(TAG_H2, "~~~ BLINK: 1. Simple version with random intervals");
        #endif
        xTaskCreate(random_blink_task, "uart_sim", 4096, NULL, 1, NULL);
      #endif
    
      #if LED_MODE == 2
        #if DEBUG_MODE
          ESP_LOGW(TAG_H2, "~~~ BLINK: 2. Realistic version with UART patterns");
        #endif
        xTaskCreate(simulate_uart_activity, "uart_pattern", 4096, NULL, 1, NULL);
      #endif
    
      #if LED_MODE == 3
        #if DEBUG_MODE
           ESP_LOGW(TAG_H2, "~~~ BLINK: 3. Version with different activity modes");
         #endif
         xTaskCreate(uart_simulation_task, "uart_sim", 4096, NULL, 1, NULL);
      #endif
      //<-- UART RX/TX Blinking Simulator
    #endif //-- USE_ORDINARY_LED
  
    //-- at least one LED must be an RGB LED
    #if USE_RGB_LED
      //--> RGB LED indicator
      led_handle = configure_indicator();
      while(1) {
        //ESP_LOGW(TAG_H2, "~~~ BLINK RGB: BLINK_ONCE_RED");
        get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      //<-- RGB LED indicator
    #endif //-- USE_RGB_LED
  #endif //-- USE_DRIVER_LED_INDICATOR
	
}
