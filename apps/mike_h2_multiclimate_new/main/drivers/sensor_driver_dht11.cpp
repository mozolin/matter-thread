#include "sensor_driver_dht11.h"
#include "app_priv.h"

float temperature, humidity;

esp_err_t dht11_init(dht11_dev_t *dev, gpio_num_t data_pin)
{
    gpio_set_pull_mode((gpio_num_t)CONFIG_DHT11_GPIO, GPIO_PULLUP_ONLY);
    
    return ESP_OK;
}

esp_err_t dht11_read(dht11_dev_t *dev, int16_t *temperature, uint16_t *humidity)
{
    float hum, temp;
    //while (1)
    //{
        if (dht_read_float_data(DHT_TYPE_DHT11, (gpio_num_t)CONFIG_DHT11_GPIO, &hum, &temp) == ESP_OK)
            ESP_LOGW("|   DHT11", "Temp: %.2f °C, Hum: %.2f %%", temp, hum);
        else
            ESP_LOGE(">> DHT11", "Could not read data from sensor!");

        // If you read the sensor data too often, it will heat up
        // http://www.kandrsmith.org/RJS/Misc/Hygrometers/dht_sht_how_fast.html
        //vTaskDelay(pdMS_TO_TICKS(2000));
    //}
    

    *temperature = (int16_t)(temp * 100.0f);
    *humidity = (uint16_t)(hum * 100.0f);
    return ESP_OK;
}

esp_err_t dht11_reset(dht11_dev_t *dev)
{
    /*
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    */
    return ESP_OK;
}
