#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c_master.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

// BME280 Device Structure
typedef struct {
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    i2c_port_t i2c_port;
    uint8_t i2c_addr;
    i2c_master_dev_handle_t dev_handle;
    int16_t last_temperature;
    uint16_t last_humidity;
    int16_t last_pressure;
    uint64_t last_read_time;
    bool initialized;
} bme280_dev_t;

// Initialize BME280 sensor
esp_err_t bme280_init(bme280_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t i2c_port, uint8_t i2c_addr);

// Read all data from BME280
esp_err_t bme280_read_all(bme280_dev_t *dev, int16_t *temperature, uint16_t *humidity, int16_t *pressure);

// Read temperature only (in 0.01°C)
esp_err_t bme280_read_temperature(bme280_dev_t *dev, int16_t *temperature);

// Read humidity only (in 0.01%)
esp_err_t bme280_read_humidity(bme280_dev_t *dev, uint16_t *humidity);

// Read pressure only (in Pa)
esp_err_t bme280_read_pressure(bme280_dev_t *dev, int16_t *pressure);

// Reset BME280 sensor
esp_err_t bme280_reset(bme280_dev_t *dev);

#ifdef __cplusplus
}
#endif