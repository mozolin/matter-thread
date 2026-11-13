#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "bme680.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bme680_init_app();
esp_err_t bme680_read_app(int16_t *temperature, uint16_t *humidity,
                          int16_t *pressure, uint32_t *gas_resistance);
esp_err_t bme680_reset_app();

#ifdef __cplusplus
}
#endif
