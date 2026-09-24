#pragma once

#include <esp_err.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#ifdef __cplusplus
extern "C" {
#endif

// DS18B20 Device Structure
typedef struct {
    gpio_num_t data_pin;
    int16_t last_temperature;  // in 0.01°C
    uint64_t last_read_time;
    bool initialized;
    uint8_t rom_code[8];
} ds18b20_dev_t;

// Initialize DS18B20 sensor
esp_err_t ds18b20_init(ds18b20_dev_t *dev, gpio_num_t data_pin);

// Read temperature from DS18B20 (returns in 0.01°C)
esp_err_t ds18b20_read_temperature(ds18b20_dev_t *dev, int16_t *temperature);

// Reset DS18B20 sensor
esp_err_t ds18b20_reset(ds18b20_dev_t *dev);

#ifdef __cplusplus
}
#endif