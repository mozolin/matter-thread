#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include "bmp280.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bme280_init();
esp_err_t bme280_read_all(int16_t *temperature, uint16_t *humidity, int16_t *pressure);
esp_err_t bme280_reset();

#ifdef __cplusplus
}
#endif
