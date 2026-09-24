#pragma once

#include <esp_err.h>
#include <driver/i2c_master.h>
#include <driver/gpio.h>

#ifdef __cplusplus
extern "C" {
#endif

// Инициализация I2C шины (должна вызываться один раз)
esp_err_t i2c_bus_init(gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t port);

// Деинициализация I2C шины
void i2c_bus_deinit(void);

// Добавление устройства на шину
esp_err_t i2c_bus_add_device(uint8_t dev_addr, uint32_t clk_speed_hz, i2c_master_dev_handle_t *dev_handle);

// Получение глобального хендла шины (для обратной совместимости)
i2c_master_bus_handle_t i2c_bus_get_handle(void);

#ifdef __cplusplus
}
#endif