#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "ds18x20.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t data_pin;
    uint8_t rom_code[8];
    bool initialized;
} ds18b20_dev_t;

esp_err_t ds18b20_init(ds18b20_dev_t *dev, gpio_num_t data_pin);
esp_err_t ds18b20_read(ds18b20_dev_t *dev, int16_t *temperature);
esp_err_t ds18b20_reset(ds18b20_dev_t *dev);

#ifdef __cplusplus
}
#endif
