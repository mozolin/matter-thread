#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <driver/i2c.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

// BME280 Device Structure
typedef struct {
    gpio_num_t sda_pin;
    gpio_num_t scl_pin;
    i2c_port_t i2c_bus;
    uint8_t i2c_addr;
    int16_t last_temperature;  // in 0.01°C
    uint16_t last_humidity;     // in 0.01%
    int16_t last_pressure;      // in Pa
    uint64_t last_read_time;
    bool initialized;
    void* i2c_handle;  // I2C handle for communication
} bme280_dev_t;

// Initialize BME280 sensor
esp_err_t bme280_init(bme280_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t i2c_bus, uint8_t i2c_addr);

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
