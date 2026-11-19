#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "bmp280.h"
#include "sensor_common.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t bme280_init_app();
esp_err_t bme280_read_app(int16_t *temperature, uint16_t *humidity, int16_t *pressure);
esp_err_t bme280_reset_app();

#ifdef __cplusplus
}
#endif
