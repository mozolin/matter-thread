#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "scd4x.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t scd4x_init_app();
esp_err_t scd4x_read_app(int16_t *co2, int16_t *temperature, uint16_t *humidity);
esp_err_t scd4x_driver_deinit(void);
esp_err_t scd4x_reset_app();

#ifdef __cplusplus
}
#endif
