#pragma once

#include <esp_err.h>
#include <driver/gpio.h>

#if CONFIG_BME280_ENABLED
  #include "bmp280.h"
#endif
#if CONFIG_BME680_ENABLED
  #include "bme680.h"
#endif
#if CONFIG_SHT4X_ENABLED
  #include "sht4x.h"
#endif


#ifdef __cplusplus
extern "C" {
#endif


#if CONFIG_BME280_ENABLED
  extern esp_err_t bmp280_init_desc_adv(bmp280_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
#endif
#if CONFIG_BME680_ENABLED
  extern esp_err_t bme680_init_desc_adv(bme680_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
#endif
#if CONFIG_SHT4X_ENABLED
  extern esp_err_t sht4x_init_desc_adv(sht4x_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
#endif
#if CONFIG_SCD4X_ENABLED
  extern esp_err_t scd4x_init_desc_adv(sht4x_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio);
#endif

extern void scan_port(i2c_port_t port, gpio_num_t sda, gpio_num_t scl);
extern void check_i2c_ports(void);


#ifdef __cplusplus
}
#endif
