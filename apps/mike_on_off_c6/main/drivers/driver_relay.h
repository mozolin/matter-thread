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

extern void turn_off_other_relays(uint8_t excluded_endpoint);

extern void relay_init(void);

extern esp_err_t relay_set_on_off(uint8_t endpoint, bool state);

extern esp_err_t nvs_save_state(uint8_t endpoint, bool state);

extern bool nvs_load_state(uint8_t endpoint);

extern esp_err_t app_driver_sync_update(uint16_t endpoint_id, gpio_num_t gpio_pin, bool state, bool matter_update);

extern bool get_plug_state(uint8_t endpoint, bool logs);

extern void print_plugs_state(void);
