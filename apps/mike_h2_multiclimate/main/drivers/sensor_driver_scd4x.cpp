#include "sensor_driver_scd4x.h"
#include "app_priv.h"

// Дескриптор устройства SCD4X
i2c_dev_t scd4x_dev = { 0 };
// Флаг инициализации
bool s_initialized = false;

/**
 * @brief Инициализация I2C и датчика SCD4X
 */
esp_err_t scd4x_init_app(void)
{
  esp_err_t err = ESP_OK;
  
  if(s_initialized) {
    #if DEBUG_MODE
      ESP_LOGW(TAG_MIKE_APP, "Драйвер SCD4X уже инициализирован");
    #endif
    return ESP_OK;
  }

  #if DEBUG_MODE
    ESP_LOGI(TAG_MIKE_APP, "Инициализация SCD4X: порт=%d, SDA=%d, SCL=%d",
           CONFIG_SCD4X_I2C_PORT, (gpio_num_t)CONFIG_SCD4X_SDA_GPIO, (gpio_num_t)CONFIG_SCD4X_SCL_GPIO);
  #endif

  #if !MOCK_SENSORS_BEHAVIOR
    err = scd4x_init_desc(&scd4x_dev, CONFIG_SCD4X_I2C_PORT, (gpio_num_t)CONFIG_SCD4X_SDA_GPIO, (gpio_num_t)CONFIG_SCD4X_SCL_GPIO);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка инициализации дескриптора SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }
    
    //-- Wake up sensor from sleep mode to idle mode
    err = scd4x_wake_up(&scd4x_dev);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка пробуждения SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }

    //-- Stop periodic measurement
    err = scd4x_stop_periodic_measurement(&scd4x_dev);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка остановки периодических измерений SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }

    //-- Reinitialize sensor
    err = scd4x_reinit(&scd4x_dev);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка переинициализации сенсора SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }
    
    uint16_t serial[3];
    //-- Read serial number
    err = scd4x_get_serial_number(&scd4x_dev, serial, serial + 1, serial + 2);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка чтения серийного номера SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }
    ESP_LOGI(TAG, "Sensor serial number: 0x%04x%04x%04x", serial[0], serial[1], serial[2]);
    
    //-- Start periodic measurement
    err = scd4x_start_periodic_measurement(&scd4x_dev);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Ошибка запуска периодических измерений SCD4X: %s", esp_err_to_name(err));
      #endif
      return err;
    }
    ESP_LOGI(TAG, "Periodic measurements started");
  #endif
  
  s_initialized = true;
  
  #if DEBUG_MODE
    ESP_LOGI("", "");
    ESP_LOGI("", "##############################");
    ESP_LOGI("", "#                            #");
    ESP_LOGI("", "#     SCD4X: initialized     #");
    ESP_LOGI("", "#                            #");
    ESP_LOGI("", "##############################");
    ESP_LOGI("", "");
  #endif

  return err;
}

/**
 * @brief Выполнить измерение и заполнить структуру результата
 */
esp_err_t scd4x_read_app(int16_t *temperature, uint16_t *humidity)
{
  esp_err_t err = ESP_OK;

  uint16_t co2;
  float temp, hum;
    
  #if !MOCK_SENSORS_BEHAVIOR
    if(!s_initialized) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Драйвер не инициализирован");
      #endif
      return ESP_ERR_INVALID_STATE;
    }

    esp_err_t res = scd4x_read_measurement(&scd4x_dev, &co2, &temp, &hum);
    if(res != ESP_OK)
    {
      ESP_LOGE(TAG, "Error reading results %d (%s)", res, esp_err_to_name(res));
      return err;
    }

    if(co2 == 0)
    {
      ESP_LOGW(TAG, "Invalid sample detected, skipping");
      return ESP_FAIL;
    }
  #else
    co2 = 1234;
    temp = 26.77;
    hum  = 41.33;
  #endif

  #if DEBUG_MODE
    ESP_LOGW("|  SCD4X", "CO2: %u ppm, Temp: %.2f °C, Hum: %.2f %%", co2, temp, hum);
  #else
    ESP_LOGD("|  SCD4X", "CO2: %u ppm, Temp: %.2f °C, Hum: %.2f %%", co2, temp, hum);
  #endif

  *temperature = (int16_t)(temp * 100.0f);
  *humidity = (uint16_t)(hum * 100.0f);

  return err;
}

/**
 * @brief Освобождение ресурсов
 */
esp_err_t scd4x_driver_deinit(void)
{
  if(!s_initialized) {
    return ESP_OK;
  }

  scd4x_free_desc(&scd4x_dev);
  
  s_initialized = false;
  #if DEBUG_MODE
    ESP_LOGI(TAG_MIKE_APP, "Драйвер SCD4X освобождён");
  #endif

  return ESP_OK;
}

esp_err_t scd4x_reset_app()
{
  return ESP_OK;
}
