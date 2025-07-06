
//#include <esp_log.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app/util/attribute-storage.h>
#include <app/clusters/on-off-server/on-off-server.h>

#include "driver_relay.h"
#include <app_priv.h>

#include "driver/gpio.h"
#include <vector>

#include "nvs_flash.h"
#include "nvs.h"

//-- List of all relays (expandable)
const std::vector<RelayConfig> relays = {
  {1, GPIO_NUM_3},
	{2, GPIO_NUM_5},
	{3, GPIO_NUM_2},
	{4, GPIO_NUM_1},
	{5, GPIO_NUM_0},
	{6, GPIO_NUM_12},
	{7, GPIO_NUM_11},
	{8, GPIO_NUM_10},
};

//-- Turn off ALL relays except the specified one
void turn_off_other_relays(uint8_t excluded_endpoint)
{
  for(const auto &relay : relays) {
    if(relay.endpoint != excluded_endpoint) {
      bool matter_update = true;
      app_driver_sync_update(relay.endpoint, relay.gpio_pin, false, matter_update);
    }
  }
}

//-- Initialize relay GPIOs and restore saved states
void relay_init()
{
  for(const auto &relay : relays) {
    bool state = nvs_load_state(relay.endpoint);
    gpio_set_level(relay.gpio_pin, state ? 1 : 0);
    //OnOffServer::Instance().setOnOffValue(relay.endpoint, state);
    //bool matter_update = false;
    //app_driver_sync_update(relay.endpoint, relay.gpio_pin, state, matter_update);
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

	get_plug_state(endpoint_id);
	
	return err;
}

bool get_plug_state(uint8_t endpoint)
{
  bool current_state;
  auto status = OnOffServer::Instance().getOnOffValue(endpoint, &current_state);
  
  if(status == chip::Protocols::InteractionModel::Status::Success) {
    ESP_LOGW(TAG, "~~~ Endpoint %d: %s", endpoint, current_state ? "ON" : "OFF");
    return current_state;
  }
  
  ESP_LOGE(TAG, "~~~ Error reading endpoint %d (status %d)", endpoint, static_cast<int>(status));
  return false;
}
