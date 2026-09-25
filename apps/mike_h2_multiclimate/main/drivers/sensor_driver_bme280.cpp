#include "sensor_driver_bme280.h"
#include "app_priv.h"

bmp280_t dev;
bool bme280p;

esp_err_t bme280_init()
{
    esp_err_t err = ESP_OK;
    
    ESP_LOGI(TAG_MULTI_SENSOR, "BMP280: found %s", bme280p ? "BME280" : "BMP280");
    
    bmp280_params_t params;
    bmp280_init_default_params(&params);
    
    memset(&dev, 0, sizeof(bmp280_t));

    err = bmp280_init_desc(&dev, BMP280_I2C_ADDRESS_0, CONFIG_BME280_I2C_PORT, (gpio_num_t)CONFIG_BME280_SDA_GPIO, (gpio_num_t)CONFIG_BME280_SCL_GPIO);
    if(err != ESP_OK) {
      ESP_LOGE(TAG_MULTI_SENSOR, "bmp280_init failed: %s", esp_err_to_name(err));
      return err;
    }
    err = bmp280_init(&dev, &params);
    if(err != ESP_OK) {
      ESP_LOGE(TAG_MULTI_SENSOR, "bmp280_init failed: %s", esp_err_to_name(err));
      return err;
    }

    bme280p = dev.id == BME280_CHIP_ID;
    ESP_LOGI(TAG_MULTI_SENSOR, "BMP280: found %s", bme280p ? "BME280" : "BMP280");

    return err;
}

esp_err_t bme280_read_all(bme280_dev_t *dev1, int16_t *temperature, uint16_t *humidity, int16_t *pressure)
{
    float temp, hum, press;

    /*
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(500));
    */
        if (bmp280_read_float(&dev, &temp, &press, &hum) != ESP_OK)
        {
            ESP_LOGE(TAG_MULTI_SENSOR, "Temperature/pressure reading failed!");
            //continue;
            return ESP_OK;
        }

        ESP_LOGD("|  BME280", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f hPa", temp, hum, press);
        /*
        if (bme280p) {
            ESP_LOGI(TAG_MULTI_SENSOR, "Humidity: %.2f", hum);
        } else {
            
        }
        */
    /*
    }
    */

    // Matter ожидает 0.01°C, 0.01%, Па
    *temperature = (int16_t)(temp * 100.0f);
    *humidity = (uint16_t)(hum * 100.0f);
    *pressure = (int16_t)press;  // Па
    return ESP_OK;
}

esp_err_t bme280_reset(bme280_dev_t *dev1)
{
    /*
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return bmp280_init(&dev->dev);
    */
    return ESP_OK;
}
