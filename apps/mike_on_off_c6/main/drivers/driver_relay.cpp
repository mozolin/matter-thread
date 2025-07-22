
//#include <esp_log.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/util/attribute-storage.h>
#include <app/clusters/on-off-server/on-off-server.h>

#include <app_priv.h>
#include "driver_relay.h"

#include "driver/gpio.h"
#include <vector>

#include "nvs_flash.h"
#include "nvs.h"


//-- Turn off ALL relays except the specified one
void turn_off_other_relays(uint8_t excluded_endpoint)
{
  for(const auto &relay : relays) {
    if(relay.endpoint != excluded_endpoint) {
      bool matter_update = true;
      app_driver_sync_update(relay.endpoint, relay.gpio_pin, false, matter_update);
      #if USE_SSD1306_DRIVER
	      //-- SSD1306: show plug status
  	    show_plug_status(relay.endpoint, false);
      #endif
    }
  }
}

//-- Initialize relay GPIOs and restore saved states
void relay_init()
{
  for(const auto &relay : relays) {
    bool state = nvs_load_state(relay.endpoint);
    #if RELAY_INVERSE_LEVEL
    	gpio_set_level(relay.gpio_pin, state ? 0 : 1);
    #else
      gpio_set_level(relay.gpio_pin, state ? 1 : 0);
    #endif
    //OnOffServer::Instance().setOnOffValue(relay.endpoint, state);
    //bool matter_update = false;
    //app_driver_sync_update(relay.endpoint, relay.gpio_pin, state, matter_update);
    #if USE_SSD1306_DRIVER
	    //-- SSD1306: show plug status
  		show_plug_status(relay.endpoint, state);
  	#endif
  }
}

//-- Set relay state (with auto-turn-off for others)
esp_err_t relay_set_on_off(uint8_t endpoint, bool state)
{
  esp_err_t err = ESP_OK;

  for(const auto &relay : relays) {
    if(relay.endpoint == endpoint) {
      //-- If turning ON, ensure other relays are OFF
      if(state) {
        turn_off_other_relays(endpoint);
        vTaskDelay(pdMS_TO_TICKS(100));
      }

      //-- update GPIO value
      bool matter_update = false;
      app_driver_sync_update(endpoint, relay.gpio_pin, state, matter_update);

      break;
    }
  }

  if(err == ESP_OK) {
  	#if USE_SSD1306_DRIVER
	  	//-- SSD1306: show plug status
  		show_plug_status(endpoint, state);
  	#endif
  }

  return err;
}


/*********************
 *                   *
 *   NVS SAVE/LOAD   *
 *                   *
 *********************/
//-- Save the state of a relay to NVS (Non-Volatile Storage)
esp_err_t nvs_save_state(uint8_t endpoint, bool state)
{
  nvs_handle_t handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) return err;

  char key[10];
  snprintf(key, sizeof(key), "relay_%d", endpoint);
  err = nvs_set_u8(handle, key, state ? 1 : 0);
  nvs_close(handle);
  return err;
}

//-- Load the saved state of a relay from NVS
bool nvs_load_state(uint8_t endpoint)
{
  nvs_handle_t handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) return false;

  char key[10];
  snprintf(key, sizeof(key), "relay_%d", endpoint);
  uint8_t state = 0;
  nvs_get_u8(handle, key, &state);
  nvs_close(handle);
  return state == 1;
}

esp_err_t app_driver_sync_update(uint16_t endpoint_id, gpio_num_t gpio_pin, bool state, bool matter_update)
{
	esp_err_t err = ESP_OK;

	//-- Update the physical GPIO
	err = app_driver_update_gpio_value(gpio_pin, state ? 1 : 0);
	//-- Save the state to NVS
  nvs_save_state(endpoint_id, state);
  
  //-- Updating state via Matter
  if(matter_update) {
  	OnOffServer::Instance().setOnOffValue(endpoint_id, 0, state);
	}

	get_plug_state(endpoint_id, true);
	
	return err;
}

bool get_plug_state(uint8_t endpoint, bool logs)
{
  bool current_state;
  auto status = OnOffServer::Instance().getOnOffValue(endpoint, &current_state);
  if(status == chip::Protocols::InteractionModel::Status::Success) {
    if(logs) {
    	ESP_LOGW(TAG_MIKE_APP, "~~~ Endpoint %d: %s", endpoint, current_state ? "ON" : "OFF");
    }
    return current_state;
  }
  if(logs) {
  	ESP_LOGE(TAG_MIKE_APP, "~~~ Error reading endpoint %d (status %d)", endpoint, static_cast<int>(status));
  }
  return false;
}

void print_plugs_state()
{
  bool state;
  char str_ep[sizeof(int) * 8 + 1], str_gpio[sizeof(int) * 8 + 1];
  
  ESP_LOGW("", "");
  ESP_LOGW("", "-------------------------");
  ESP_LOGW("", "|   EP | GPIO |  State  |");
  ESP_LOGW("", "-------------------------");
	for(const auto &relay : relays) {
    //state = get_plug_state(relay.endpoint, false);
    //state = gpio_get_level(relay.gpio_pin);
    //state = nvs_load_state(relay.endpoint);

    int ep = relay.endpoint;
    int gpio = relay.gpio_pin;
    
  	OnOffServer::Instance().getOnOffValue(ep, &state);
    
    if(ep < 10) {
    	sprintf(str_ep, " %d", ep);
    } else {
    	sprintf(str_ep, "%d", ep);
    }
    if(gpio < 10) {
    	sprintf(str_gpio, " %d", gpio);
    } else {
    	sprintf(str_gpio, "%d", gpio);
    }
    
    ESP_LOGW("", "|   %s |  %s  |   %s   |", str_ep, str_gpio, state ? "ON " : "OFF");
  }
  ESP_LOGW("", "-------------------------");
  ESP_LOGW("", "");
}
