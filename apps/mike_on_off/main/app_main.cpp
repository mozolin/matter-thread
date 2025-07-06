
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include "driver/gpio.h"
#include "soc/gpio_num.h"

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#include <platform/ESP32/OpenthreadLauncher.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>


#define CREATE_PLUG(node, plug_id) \
    struct gpio_plug plug##plug_id; \
    plug##plug_id.GPIO_PIN_VALUE = (gpio_num_t) CONFIG_GPIO_PLUG_##plug_id; \
    create_plug(&plug##plug_id, node);

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;
uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

led_indicator_handle_t led_handle;

/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
blink_step_t const *led_mode[] = {
  [BLINK_ON_YELLOW] = yellow_on,
  [BLINK_ON_ORANGE] = orange_on,
  [BLINK_DOUBLE_RED] = double_red_blink,
  [BLINK_TRIPLE_GREEN] = triple_green_blink,
  [BLINK_LONG_BLUE] = blue_long_blink,
  [BLINK_WHITE_BREATHE_SLOW] = breath_white_slow_blink,
  [BLINK_WHITE_BREATHE_FAST] = breath_white_fast_blink,
  [BLINK_BLUE_BREATH] = breath_blue_blink,
  [BLINK_COLOR_HSV_RING] = color_hsv_ring_blink,
  [BLINK_COLOR_RGB_RING] = color_rgb_ring_blink,
  #if LED_NUMBERS > 1
    [BLINK_FLOWING] = flowing_blink,
  #endif
  [BLINK_MAX] = NULL,
};

uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay)
{
	uint8_t idx = 255;
	
	int size = sizeof(led_mode)/sizeof(led_mode[0]);

	auto item = led_mode[blink_type];
  for(int i=0; i<size; i++) {
  	if(led_mode[i] == item) {
  		//ESP_LOGW(TAG, "~~~ ###!!!@@@ FOUND: %d", i);
  		idx = i;

  		if(start_delay > 0) {
  			led_indicator_start(led_handle, idx);
      	vTaskDelay(pdMS_TO_TICKS(start_delay));

      	//led_indicator_set_on_off(led_handle, true);

		  	led_indicator_stop(led_handle, idx);
      	//vTaskDelay(pdMS_TO_TICKS(stop_delay));
      }

  		break;
  	}
  }

  return idx;
}


static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch (event->Type) {
  case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
    ESP_LOGW(TAG, "~~~ Interface IP Address changed");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
    ESP_LOGW(TAG, "~~~ Commissioning complete");
    break;

  case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
    ESP_LOGW(TAG, "~~~ Commissioning failed, fail safe timer expired");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
    ESP_LOGW(TAG, "~~~ Commissioning session started");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
    ESP_LOGW(TAG, "~~~ Commissioning session stopped");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
    ESP_LOGW(TAG, "~~~ Commissioning window opened");
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
    ESP_LOGW(TAG, "~~~ Commissioning window closed");
    break;

  case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster:
    ESP_LOGW(TAG, "~~~ Binding entry changed");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
    ESP_LOGW(TAG, "~~~ Fabric removed successfully");
    if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
      chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        //-- After removing last fabric, this example does not remove the Wi-Fi credentials
        //-- and still has IP connectivity so, only advertising on DNS-SD.
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
        if(err != CHIP_NO_ERROR) {
          ESP_LOGE(TAG, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
        }
      }
    }
    break;
  }

  case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
    ESP_LOGW(TAG, "~~~ Fabric will be removed");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
    ESP_LOGW(TAG, "~~~ Fabric is updated");
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
    ESP_LOGW(TAG, "~~~ Fabric is committed");
    break;

  case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
    ESP_LOGW(TAG, "~~~ BLE deinitialized and memory reclaimed");
    break;

  default:
    break;
  }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGW(TAG, "~~~ Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  return ESP_OK;
}

//-- This callback is called for every attribute update. The callback implementation shall
//-- handle the desired attributes and return an appropriate error code. If the attribute
//-- is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  esp_err_t err = ESP_OK;

  switch(type) {
  	case PRE_UPDATE: {
  	  	//-- Driver update
        bool state = val->val.b;
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
        if(err == ESP_OK) {
        	ESP_LOGW(TAG, "~~~ Attribute updated: %d|%d|%d|%d", (int)endpoint_id, (int)cluster_id, (int)attribute_id, (int)state);
        	//-- Blink...
        	get_led_indicator_blink_idx(BLINK_LONG_BLUE, 300, 0);
        } else {
        	ESP_LOGE(TAG, "~~~ Failed to update attribute :%d|%d|%d|%d", (int)endpoint_id, (int)cluster_id, (int)attribute_id, (int)state);
        }
  		}
			break;
  
  	case POST_UPDATE:
  		break;
  
  	case READ:
  		break;
  
  	case WRITE:
  		break;
  }
  
  return err;
}

//-- Creates plug-endpoint mapping for each GPIO pin configured.
static esp_err_t create_plug(gpio_plug* plug, node_t* node)
{
  esp_err_t err = ESP_OK;

  if(!node) {
    ESP_LOGE(TAG, "~~~ Matter node cannot be NULL");
    return ESP_ERR_INVALID_ARG;
  }

  if(!plug) {
    ESP_LOGE(TAG, "~~~ Plug cannot be NULL");
    return ESP_ERR_INVALID_ARG;
  }

  //-- Check if plug's IO pin is already used by reset button
  if((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
    ESP_LOGE(TAG, "~~~ Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
    return ESP_ERR_INVALID_STATE;
  }

  //-- Check for plug if already configured.
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
      ESP_LOGE(TAG, "~~~ Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
      return ESP_ERR_INVALID_STATE;
    }
  }

  on_off_plugin_unit::config_t plugin_unit_config;
  plugin_unit_config.on_off.on_off = false;
  endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

  if(!endpoint) {
    ESP_LOGE(TAG, "~~~ Matter endpoint creation failed");
    return ESP_FAIL;
  }

  //-- GPIO pin Initialization
  err = app_driver_plugin_unit_init(plug);

  if(err != ESP_OK) {
    ESP_LOGE(TAG, "~~~ Failed to initialize plug");
  }

  //-- Check for maximum plugs that can be configured.
  if(configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
    plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
    plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
    configure_plugs++;
  } else {
    ESP_LOGE(TAG, "~~~ Maximum plugs configuration limit exceeded!!!");
    return ESP_FAIL;
  }

  uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
  ESP_LOGW(TAG, "~~~ Plug created with endpoint_id %d", plug_endpoint_id);
  return err;
}



extern "C" void app_main()
{
  //-- Start reboot button task
  xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, 5, NULL);

  esp_err_t err = ESP_OK;

  //-- Initialize the ESP NVS (Non-Volatile Storage) layer
  err = nvs_flash_init();
  if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);

  //-- Create a Matter node and add the mandatory Root Node device type on endpoint 0
  node::config_t node_config;

  //-- node handle can be used to add/modify other endpoints.
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "~~~ Failed to create Matter node"));

	#ifdef CONFIG_GPIO_PLUG_1
    CREATE_PLUG(node, 1)
	#endif

	#ifdef CONFIG_GPIO_PLUG_2
    CREATE_PLUG(node, 2)
	#endif

	#ifdef CONFIG_GPIO_PLUG_3
    CREATE_PLUG(node, 3)
	#endif

	#ifdef CONFIG_GPIO_PLUG_4
    CREATE_PLUG(node, 4)
	#endif

	#ifdef CONFIG_GPIO_PLUG_5
    CREATE_PLUG(node, 5)
	#endif

	#ifdef CONFIG_GPIO_PLUG_6
    CREATE_PLUG(node, 6)
	#endif

	#ifdef CONFIG_GPIO_PLUG_7
    CREATE_PLUG(node, 7)
	#endif

	#ifdef CONFIG_GPIO_PLUG_8
    CREATE_PLUG(node, 8)
	#endif

	#ifdef CONFIG_GPIO_PLUG_9
    CREATE_PLUG(node, 9)
	#endif

	#ifdef CONFIG_GPIO_PLUG_10
    CREATE_PLUG(node, 10)
	#endif

	#ifdef CONFIG_GPIO_PLUG_11
    CREATE_PLUG(node, 11)
	#endif

	#ifdef CONFIG_GPIO_PLUG_12
    CREATE_PLUG(node, 12)
	#endif

	#ifdef CONFIG_GPIO_PLUG_13
    CREATE_PLUG(node, 13)
	#endif

	#ifdef CONFIG_GPIO_PLUG_14
    CREATE_PLUG(node, 14)
	#endif

	#ifdef CONFIG_GPIO_PLUG_15
    CREATE_PLUG(node, 15)
	#endif

	#ifdef CONFIG_GPIO_PLUG_16
    CREATE_PLUG(node, 16)
	#endif

  //-- Set OpenThread platform config
  esp_openthread_platform_config_t config = {
    .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
    .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
    .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
  };
  set_openthread_platform_config(&config);

  //-- Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "~~~ Failed to start Matter, err:%d", err));

	#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
		#if CONFIG_OPENTHREAD_CLI
    	esp_matter::console::otcli_register_commands();
		#endif
    esp_matter::console::init();
	#endif

  /*********************
   *                   *
   *   LED INDICATOR   *
   *                   *
   *********************/
  led_handle = configure_indicator();

  //uint8_t idx = get_led_indicator_blink_idx(BLINK_LONG_BLUE, 2000, 0);
  //ESP_LOGE(TAG, "~~~ ###!!!@@@ FOUND: %d", idx);
  
  
  /*
  !!! CLOSED !!!
  led_indicator_handle_t led_handle = configure_indicator();
  
  while(1) {
    ESP_LOGW(TAG, "~~~ ************************************");
    ESP_LOGW(TAG, "~~~ *                                  *");
    ESP_LOGW(TAG, "~~~ *   Start blinking LED indicator   *");
    ESP_LOGW(TAG, "~~~ *                                  *");
    ESP_LOGW(TAG, "~~~ ************************************");

    for(int i = 0; i < BLINK_MAX; i++) {
      led_indicator_start(led_handle, i);
      ESP_LOGW(TAG, "~~~ start blink: %d", i);
      vTaskDelay(pdMS_TO_TICKS(4000));
      
      led_indicator_stop(led_handle, i);
      ESP_LOGW(TAG, "~~~ stop blink: %d", i);
      vTaskDelay(pdMS_TO_TICKS(1000));
    }
  }
  */

}
