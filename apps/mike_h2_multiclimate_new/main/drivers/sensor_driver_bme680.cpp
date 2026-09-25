#include "sensor_driver_bme680.h"
#include "app_priv.h"

bme680_t sensor;
TickType_t last_wakeup;
uint32_t duration;

esp_err_t bme680_init(bme680_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin,
                      i2c_port_t i2c_bus, uint8_t i2c_addr)
{
    esp_err_t err = ESP_OK;
    
    
    memset(&sensor, 0, sizeof(bme680_t));

    //ESP_ERROR_CHECK(bme680_init_desc(&sensor, BME680_I2C_ADDR_1, CONFIG_BME680_I2C_PORT, (gpio_num_t)CONFIG_BME680_SDA_GPIO, (gpio_num_t)CONFIG_BME680_SCL_GPIO));
    err = bme680_init_desc(&sensor, BME680_I2C_ADDR_1, CONFIG_BME680_I2C_PORT, (gpio_num_t)CONFIG_BME680_SDA_GPIO, (gpio_num_t)CONFIG_BME680_SCL_GPIO);
    if(err != ESP_OK) {
      ESP_LOGE(TAG_MULTI_SENSOR, "bme680_init failed: %s", esp_err_to_name(err));
      return err;
    }

    // init the sensor
    //ESP_ERROR_CHECK(bme680_init_sensor(&sensor));
    err = bme680_init_sensor(&sensor);
    if(err != ESP_OK) {
      ESP_LOGE(TAG_MULTI_SENSOR, "bme680_init failed: %s", esp_err_to_name(err));
      return err;
    }

    // Changes the oversampling rates to 4x oversampling for temperature
    // and 2x oversampling for humidity. Pressure measurement is skipped.
    bme680_set_oversampling_rates(&sensor, BME680_OSR_4X, BME680_OSR_NONE, BME680_OSR_2X);

    // Change the IIR filter size for temperature and pressure to 7.
    bme680_set_filter_size(&sensor, BME680_IIR_SIZE_7);

    // Change the heater profile 0 to 200 degree Celsius for 100 ms.
    bme680_set_heater_profile(&sensor, 0, 200, 100);
    bme680_use_heater_profile(&sensor, 0);

    // Set ambient temperature to 10 degree Celsius
    bme680_set_ambient_temperature(&sensor, 10);

    // as long as sensor configuration isn't changed, duration is constant
    bme680_get_measurement_duration(&sensor, &duration);
    
    last_wakeup = xTaskGetTickCount();
    
    return err;
}

esp_err_t bme680_read_all(bme680_dev_t *dev, int16_t *temperature, uint16_t *humidity,
                          int16_t *pressure, uint32_t *gas_resistance)
{
    bme680_values_float_t values;
    /*
    while (1)
    {
    */
        // trigger the sensor to start one TPHG measurement cycle
        if (bme680_force_measurement(&sensor) == ESP_OK)
        {
            // passive waiting until measurement results are available
            vTaskDelay(duration);

            // get the results and do something with them
            if (bme680_get_results_float(&sensor, &values) == ESP_OK)
                ESP_LOGW("|  BME680", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f hPa, Gas: %.2f Ohm",
                       values.temperature, values.humidity, values.pressure, values.gas_resistance);
        }
    /*
        // passive waiting until 1 second is over
        vTaskDelayUntil(&last_wakeup, pdMS_TO_TICKS(1000));
    }
    */
    
    /*
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;

    bme680_values_float_t values;
    esp_err_t err = bme680_measure_float(&dev->dev, &values);
    if (err != ESP_OK) return err;
    */


    *temperature = (int16_t)(values.temperature * 100.0f);
    *humidity = (uint16_t)(values.humidity * 100.0f);
    *pressure = (int16_t)values.pressure;
    *gas_resistance = (uint32_t)values.gas_resistance;
    return ESP_OK;
}

esp_err_t bme680_reset(bme680_dev_t *dev)
{
    /*
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return bme680_init(&dev->dev);
    */
    return ESP_OK;
}
