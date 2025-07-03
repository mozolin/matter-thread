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

extern bool OnOffAttributeCallback(chip::EndpointId endpoint, chip::AttributeId attributeId, uint8_t *value, uint16_t *readLength);

extern void RegisterOnOffCallback(chip::EndpointId endpoint);

extern const std::vector<RelayConfig> relays;

extern void turn_off_other_relays(uint8_t excluded_endpoint);

extern void relay_init(void);

extern esp_err_t relay_set_on_off(uint8_t endpoint, bool state);

extern esp_err_t save_relay_state(uint8_t endpoint, bool state);

extern bool load_relay_state(uint8_t endpoint);
