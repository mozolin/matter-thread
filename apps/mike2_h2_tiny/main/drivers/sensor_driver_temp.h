#pragma once

#include <esp_err.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t temp_init();
esp_err_t temp_read(int16_t *temperature);
esp_err_t temp_reset();

#ifdef __cplusplus
}
#endif
