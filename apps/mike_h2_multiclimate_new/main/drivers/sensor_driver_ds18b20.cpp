#include "sensor_driver_ds18b20.h"
#include "app_priv.h"

esp_err_t ds18b20_init(ds18b20_dev_t *dev, gpio_num_t data_pin)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->data_pin = data_pin;

    // Инициализация onewire шины
    esp_err_t err = ds18x20_init(data_pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "DS18B20: ds18x20_init failed: %d", err);
        return err;
    }

    // Поиск устройств на шине
    size_t device_count = 0;
    err = ds18x20_scan(&dev->rom_code[0], 1, &device_count);
    if (err != ESP_OK || device_count == 0) {
        ESP_LOGE(TAG_MULTI_SENSOR, "DS18B20: no devices found");
        return ESP_ERR_NOT_FOUND;
    }

    dev->initialized = true;
    ESP_LOGI(TAG_MULTI_SENSOR, "DS18B20 initialized on GPIO %d", data_pin);
    return ESP_OK;
}

esp_err_t ds18b20_read_temperature(ds18b20_dev_t *dev, int16_t *temperature)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;

    float temp;
    esp_err_t err = ds18x20_measure_and_read(dev->rom_code, &temp);
    if (err != ESP_OK) return err;

    *temperature = (int16_t)(temp * 100.0f);  // 0.01°C
    return ESP_OK;
}

esp_err_t ds18b20_reset(ds18b20_dev_t *dev)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return ESP_OK;  // DS18B20 не требует reset, просто перечитать
}
