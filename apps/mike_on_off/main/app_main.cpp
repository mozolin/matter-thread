
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


#define CREATE_ALL_PLUGS(node) \
  for(const auto &relay : relays) { \
    struct gpio_plug plug; \
  	plug.GPIO_PIN_VALUE = (gpio_num_t)relay.gpio_pin; \
  	create_plug(&plug, node); \
  }
    
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;


constexpr auto k_timeout_seconds = 300;
uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

led_indicator_handle_t led_handle;

#if USE_SSD1306_DRIVER
	//-- SSD1306 device instance
	SSD1306_t ssd1306dev;
	//-- Is SSD1306 initialized?
	bool ssd1306_initialized = false;
#endif


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
  [BLINK_ONCE_RED] = red_once_blink,
  [BLINK_ONCE_GREEN] = green_once_blink,
  [BLINK_ONCE_BLUE] = blue_once_blink,
  [BLINK_ONCE_LIVE] = live_once_blink,
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
  		//ESP_LOGW(TAG_MIKE_APP, "~~~ ###!!!@@@ FOUND: %d", i);
  		idx = i;

  		if(start_delay > 0) {
  			led_indicator_start(led_handle, idx);
      	//vTaskDelay(pdMS_TO_TICKS(start_delay));
      	vTaskDelay(start_delay / portTICK_PERIOD_MS);

		  	led_indicator_stop(led_handle, idx);
		  	if(stop_delay > 0) {
      		//vTaskDelay(pdMS_TO_TICKS(stop_delay));
      		vTaskDelay(stop_delay / portTICK_PERIOD_MS);
      	}
      }

  		break;
  	}
  }

  return idx;
}

void init_indicator_task(void *pvParameter)
{
  
  uint32_t live_blink_time = 0;
  
  while(1) {
    //-- blink every "LIVE_BLINK_TIME_MS" milliseconds
    uint32_t blinked_duration = esp_log_timestamp() - live_blink_time;
    if(blinked_duration >= LIVE_BLINK_TIME_MS) {
    	get_led_indicator_blink_idx(BLINK_ONCE_LIVE, 75, 0);
    	live_blink_time = esp_log_timestamp();
    }

    vTaskDelay(pdMS_TO_TICKS(10));
  }
}

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch (event->Type) {
  case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Interface IP Address changed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning complete");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
    ESP_LOGE(TAG_MIKE_APP, "~~~ Commissioning failed, fail safe timer expired");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session started");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session stopped");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window opened");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window closed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Binding entry changed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
    ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric removed successfully");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
      chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        //-- After removing last fabric, this example does not remove the Wi-Fi credentials
        //-- and still has IP connectivity so, only advertising on DNS-SD.
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
        if(err != CHIP_NO_ERROR) {
          ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
          get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
        }
      }
    }
    break;
  }

  case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric will be removed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is updated");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
    ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is committed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
    break;

  case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
    ESP_LOGW(TAG_MIKE_APP, "~~~ BLE deinitialized and memory reclaimed");
    get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
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
  get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
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
        	ESP_LOGW(TAG_MIKE_APP, "~~~ Updated: endpoint:%d|cluster:%d|attribute:%d|state:%d", (int)endpoint_id, (int)cluster_id, (int)attribute_id, (int)state);
        	//-- Blink...
        	get_led_indicator_blink_idx(BLINK_ONCE_BLUE, 75, 0);
        } else {
        	ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to update attribute :%d|%d|%d|%d", (int)endpoint_id, (int)cluster_id, (int)attribute_id, (int)state);
        	get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
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
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter node cannot be NULL");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    return ESP_ERR_INVALID_ARG;
  }

  if(!plug) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Plug cannot be NULL");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    return ESP_ERR_INVALID_ARG;
  }

  //-- Check if plug's IO pin is already used by reset button
  if((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    return ESP_ERR_INVALID_STATE;
  }

  //-- Check for plug if already configured.
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
      return ESP_ERR_INVALID_STATE;
    }
  }

  on_off_plugin_unit::config_t plugin_unit_config;
  plugin_unit_config.on_off.on_off = false;
  endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

  if(!endpoint) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter endpoint creation failed");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    return ESP_FAIL;
  }

  //-- GPIO pin Initialization
  err = app_driver_plugin_unit_init(plug);

  if(err != ESP_OK) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to initialize plug");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
  }

  //-- Check for maximum plugs that can be configured.
  if(configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
    plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
    plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
    configure_plugs++;
  } else {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Maximum plugs configuration limit exceeded!");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
    return ESP_FAIL;
  }

  uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
  ESP_LOGW(TAG_MIKE_APP, "~~~ Plug created with endpoint_id %d", plug_endpoint_id);
  get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 75, 0);
  return err;
}



extern "C" void app_main()
{
  esp_err_t err = ESP_OK;
  
  //-- Start reboot button task
  xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, 5, NULL);
  
  //-- Init LED indicator
  led_handle = configure_indicator();
  //xTaskCreate(init_indicator_task, "init_indicator_task", 2048, NULL, 6, NULL);
  
  #if USE_SSD1306_DRIVER
	  //-- Init LCD SSD1306
  	err = ssd1306_init();
	  if(err != ESP_OK) {
	  	ESP_LOGW(TAG_MIKE_APP, "~~~ Error initialize SSD1306!");
	  	get_led_indicator_blink_idx(BLINK_ONCE_RED, 75, 0);
	  } else {
	  	ssd1306_initialized = true;
	  }
  #endif

  
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
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create Matter node"));

	CREATE_ALL_PLUGS(node);

  //-- Set OpenThread platform config
  esp_openthread_platform_config_t config = {
    .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
    .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
    .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
  };
  set_openthread_platform_config(&config);

  //-- Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to start Matter, err:%d", err));

	#if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
		#if CONFIG_OPENTHREAD_CLI
    	esp_matter::console::otcli_register_commands();
		#endif
    esp_matter::console::init();
	#endif

}
