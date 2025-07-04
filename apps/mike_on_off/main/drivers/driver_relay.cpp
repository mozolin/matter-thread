
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


static const char *TAG_RELAY = "Driver Relay";

// List of all relays (expandable)
/*static */const std::vector<RelayConfig> relays = {
  {1, GPIO_NUM_0},
	{2, GPIO_NUM_1},
	{3, GPIO_NUM_2},
	{4, GPIO_NUM_3},
	{5, GPIO_NUM_5},
	{6, GPIO_NUM_10},
	{7, GPIO_NUM_11},
	{8, GPIO_NUM_12},
};

// Turn off ALL relays except the specified one
void turn_off_other_relays(uint8_t excluded_endpoint)
{
  for(const auto &relay : relays) {
    if(relay.endpoint != excluded_endpoint) {
      ESP_LOGE("TURN OFF", "GPIO: %d set to 0", (int)relay.gpio_pin);
      app_driver_update_gpio_value(relay.gpio_pin, 0);
      save_relay_state(relay.endpoint, false);
      OnOffServer::Instance().setOnOffValue(relay.endpoint, 0, false);
    }
  }
}

// Initialize relay GPIOs and restore saved states
void relay_init()
{
  for(const auto &relay : relays) {
    bool state = load_relay_state(relay.endpoint);
    gpio_set_level(relay.gpio_pin, state ? 1 : 0);
    //OnOffServer::Instance().setOnOffValue(relay.endpoint, state);
  }
}

//-- Set relay state (with auto-turn-off for others)
esp_err_t relay_set_on_off(uint8_t endpoint, bool state)
{
  esp_err_t err = ESP_OK;

  for(const auto &relay : relays) {
    
    ESP_LOGE("TURN OFF", "EP = %d", (int)relay.endpoint);
    
    if(relay.endpoint == endpoint) {
      //-- If turning ON, ensure other relays are OFF
      if(state) {
        ESP_LOGE("TURN OFF", "*******************************");
        turn_off_other_relays(endpoint);
        ESP_LOGE("TURN OFF", "*******************************");
        vTaskDelay(pdMS_TO_TICKS(1000));
      }

      //-- update GPIO value
      err = app_driver_update_gpio_value(relay.gpio_pin, state ? 1 : 0);
      save_relay_state(endpoint, state);

      ESP_LOGE("TURN OFF", "state = %d", (int)state);

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
esp_err_t save_relay_state(uint8_t endpoint, bool state)
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
bool load_relay_state(uint8_t endpoint)
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
