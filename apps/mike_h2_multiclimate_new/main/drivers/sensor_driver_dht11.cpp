#include "sensor_driver_dht11.h"
#include "app_priv.h"

esp_err_t dht11_init(dht11_dev_t *dev, gpio_num_t data_pin)
{
    if (!dev) return ESP_ERR_INVALID_ARG;
    dev->data_pin = data_pin;
    dev->initialized = true;

    ESP_LOGI(TAG_MULTI_SENSOR, "DHT11 initialized on GPIO %d", data_pin);
    return ESP_OK;
}

esp_err_t dht11_read(dht11_dev_t *dev, int16_t *temperature, uint16_t *humidity)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;

    float temp, hum;
    esp_err_t err = dht_read_float_data(DHT_TYPE_DHT11, dev->data_pin, &hum, &temp);
    if (err != ESP_OK) return err;

    *temperature = (int16_t)(temp * 100.0f);
    *humidity = (uint16_t)(hum * 100.0f);
    return ESP_OK;
}

esp_err_t dht11_reset(dht11_dev_t *dev)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return ESP_OK;
}
