#include "sensor_driver_bme680.h"
#include "app_priv.h"

bme680_t sensor;
TickType_t last_wakeup;
uint32_t duration;

esp_err_t bme680_init()
{
    esp_err_t err = ESP_OK;
    
    #if !MOCK_SENSORS_BEHAVIOR
      memset(&sensor, 0, sizeof(bme680_t));
    
      err = bme680_init_desc(&sensor, BME680_I2C_ADDR_1, CONFIG_BME680_I2C_PORT, (gpio_num_t)CONFIG_BME680_SDA_GPIO, (gpio_num_t)CONFIG_BME680_SCL_GPIO);
      if(err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "bme680_init failed: %s", esp_err_to_name(err));
        return err;
      }
    
      // init the sensor
      err = bme680_init_sensor(&sensor);
      if(err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "bme680_init failed: %s", esp_err_to_name(err));
        return err;
      }
    
      // Changes the oversampling rates to 4x oversampling for temperature
      // and 2x oversampling for humidity. Pressure measurement is skipped.
      //bme680_set_oversampling_rates(&sensor, BME680_OSR_4X, BME680_OSR_NONE, BME680_OSR_2X);
      bme680_set_oversampling_rates(&sensor, BME680_OSR_4X, BME680_OSR_4X, BME680_OSR_2X);
    
      // Change the IIR filter size for temperature and pressure to 7.
      bme680_set_filter_size(&sensor, BME680_IIR_SIZE_7);
    
      // Change the heater profile 0 to 200 degree Celsius for 100 ms.
      bme680_set_heater_profile(&sensor, 0, 200, 100);
      bme680_use_heater_profile(&sensor, 0);
    
      // Set ambient temperature to 10 degree Celsius
      bme680_set_ambient_temperature(&sensor, 10);
    
      // as long as sensor configuration isn't changed, duration is constant
      bme680_get_measurement_duration(&sensor, &duration);
    #endif
    
    last_wakeup = xTaskGetTickCount();

    ESP_LOGI("", "");
    ESP_LOGI("", "###############################");
    ESP_LOGI("", "#                             #");
    ESP_LOGI("", "#     BME680: initialized     #");
    ESP_LOGI("", "#                             #");
    ESP_LOGI("", "###############################");
    ESP_LOGI("", "");
    
    return err;
}

esp_err_t bme680_read_all(int16_t *temperature, uint16_t *humidity,
                          int16_t *pressure, uint32_t *gas_resistance)
{
    bme680_values_float_t values;
    
    #if !MOCK_SENSORS_BEHAVIOR
      // trigger the sensor to start one TPHG measurement cycle
      if (bme680_force_measurement(&sensor) == ESP_OK)
      {
          // passive waiting until measurement results are available
          vTaskDelay(duration);
      
          // get the results and do something with them
          if (bme680_get_results_float(&sensor, &values) == ESP_OK) {
              #if DEBUG_MODE
                  ESP_LOGW("|  BME680", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f Pa, Gas: %.2f Ohm",
                     values.temperature, values.humidity, values.pressure * 100, values.gas_resistance);
              #else
                  ESP_LOGD("|  BME680", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f Pa, Gas: %.2f Ohm",
                     values.temperature, values.humidity, values.pressure * 100, values.gas_resistance);
              #endif
          }
      }
    #else
      // passive waiting until measurement results are available
      vTaskDelay(duration);
      values.temperature    = 26.48;
      values.humidity       = 47.95;
      values.pressure       = 991.00;
      values.gas_resistance = 40275.00;

      #if DEBUG_MODE
        ESP_LOGW("|  BME680", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f Pa, Gas: %.2f Ohm",
           values.temperature, values.humidity, values.pressure * 100, values.gas_resistance);
      #else
        ESP_LOGD("|  BME680", "Temp: %.2f °C, Hum: %.2f %%, Pres: %.2f Pa, Gas: %.2f Ohm",
           values.temperature, values.humidity, values.pressure * 100, values.gas_resistance);
      #endif

    #endif

    *temperature = (int16_t)(values.temperature * 100.0f);
    *humidity = (uint16_t)(values.humidity * 100.0f);
    *pressure = (int16_t)(values.pressure);
    *gas_resistance = (uint32_t)values.gas_resistance;
    return ESP_OK;
}

esp_err_t bme680_reset()
{
    /*
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return bme680_init(&dev->dev);
    */
    return ESP_OK;
}
