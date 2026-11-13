#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "ds18x20.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ds18b20_init_app();
esp_err_t ds18b20_read_app(int16_t *temperature, uint8_t *num_sensors);
esp_err_t ds18b20_reset_app();

#ifdef __cplusplus
}
#endif
