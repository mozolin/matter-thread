#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "dht.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t dht11_init();
esp_err_t dht11_read(int16_t *temperature, uint16_t *humidity);
esp_err_t dht11_reset();

#ifdef __cplusplus
}
#endif
