#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include "bme680.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bme680_init();
esp_err_t bme680_read_all(int16_t *temperature, uint16_t *humidity,
                          int16_t *pressure, uint32_t *gas_resistance);
esp_err_t bme680_reset();

#ifdef __cplusplus
}
#endif
