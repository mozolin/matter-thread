#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

// DHT11 Device Structure
typedef struct {
    gpio_num_t data_pin;
    int16_t last_temperature;  // in 0.01°C (actually DHT11 is 1°C resolution)
    uint16_t last_humidity;     // in 0.01% (actually DHT11 is 1% resolution)
    uint64_t last_read_time;
    bool initialized;
} dht11_dev_t;

// Initialize DHT11 sensor
esp_err_t dht11_init(dht11_dev_t *dev, gpio_num_t data_pin);

// Read temperature and humidity from DHT11
esp_err_t dht11_read(dht11_dev_t *dev, int16_t *temperature, uint16_t *humidity);

// Read temperature only from DHT11
esp_err_t dht11_read_temperature(dht11_dev_t *dev, int16_t *temperature);

// Read humidity only from DHT11
esp_err_t dht11_read_humidity(dht11_dev_t *dev, uint16_t *humidity);

// Reset DHT11 sensor
esp_err_t dht11_reset(dht11_dev_t *dev);

#ifdef __cplusplus
}
#endif
