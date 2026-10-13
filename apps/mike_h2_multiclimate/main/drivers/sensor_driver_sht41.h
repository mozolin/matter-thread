#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
//#include <driver/i2c_master.h>
#include "sht4x.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t sht41_init();
esp_err_t sht41_read_all(int16_t *temperature, uint16_t *humidity);
esp_err_t sht41_driver_deinit(void)
esp_err_t sht41_reset();

#ifdef __cplusplus
}
#endif
