#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "ds18x20.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t ds18b20_init();
esp_err_t ds18b20_read(int16_t *temperature);
esp_err_t ds18b20_reset();

#ifdef __cplusplus
}
#endif
