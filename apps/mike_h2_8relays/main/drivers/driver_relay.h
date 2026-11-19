#include <cstdint>
#pragma once

#include <esp_err.h>
#include "driver/gpio.h"
#include <vector>


/**************
 *            *
 *   RELAYS   *
 *            *
 **************/
// Configuration for each relay (endpoint + GPIO pin)
struct RelayConfig {
  uint8_t endpoint;
  gpio_num_t gpio_pin;
};

//-- Plugs (Relays)
//-- List of all relays
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

//-- Default attribute values used during initialization
struct gpio_plug {
  gpio_num_t GPIO_PIN_VALUE;
};

struct plugin_endpoint {
  uint16_t endpoint_id;
  gpio_num_t plug;
};

extern plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
extern uint16_t configure_plugs;



extern void turn_off_other_relays(uint8_t excluded_endpoint);

extern void relay_init(void);

extern esp_err_t relay_set_on_off(uint8_t endpoint, bool state);

extern esp_err_t nvs_save_state(uint8_t endpoint, bool state);

extern bool nvs_load_state(uint8_t endpoint);

extern esp_err_t app_driver_sync_update(uint16_t endpoint_id, gpio_num_t gpio_pin, bool state, bool matter_update);

extern bool get_plug_state(uint8_t endpoint, bool state);

#if CONFIG_DEBUG_MODE
  extern void print_plugs_state(void);
#endif
