#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include "dht.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t data_pin;
    bool initialized;
} dht11_dev_t;

esp_err_t dht11_init(dht11_dev_t *dev, gpio_num_t data_pin);
esp_err_t dht11_read(dht11_dev_t *dev, int16_t *temperature, uint16_t *humidity);
esp_err_t dht11_reset(dht11_dev_t *dev);

#ifdef __cplusplus
}
#endif
