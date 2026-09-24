#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include "bmp280.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bmp280_t dev;
    i2c_master_bus_handle_t bus_handle;
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    uint8_t i2c_addr;
    bool initialized;
} bme280_dev_t;

esp_err_t bme280_init(bme280_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin,
                      i2c_port_t i2c_bus, uint8_t i2c_addr);
esp_err_t bme280_read_all(bme280_dev_t *dev, int16_t *temperature, uint16_t *humidity, int16_t *pressure);
esp_err_t bme280_reset(bme280_dev_t *dev);

#ifdef __cplusplus
}
#endif
