#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "sht4x.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sht4x_init();
esp_err_t sht4x_read_all(int16_t *temperature, uint16_t *humidity);
esp_err_t sht4x_driver_deinit(void);
esp_err_t sht4x_reset();

#ifdef __cplusplus
}
#endif
