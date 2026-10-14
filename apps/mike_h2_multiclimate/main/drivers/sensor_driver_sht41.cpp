#include "sensor_driver_sht41.h"
#include "app_priv.h"

// Дескриптор устройства SHT41
sht4x_t s_dev;
// Флаг инициализации
bool s_initialized = false;

/**
 * @brief Инициализация I2C и датчика SHT41
 */
esp_err_t sht41_init(void)
{
    esp_err_t err = ESP_OK;
    
    if(s_initialized) {
      ESP_LOGW(TAG_MULTI_CLIMATE, "Драйвер уже инициализирован");
      return ESP_OK;
    }

    ESP_LOGI(TAG_MULTI_CLIMATE, "Инициализация SHT41: порт=%d, SDA=%d, SCL=%d",
             CONFIG_SHT41_I2C_PORT, (gpio_num_t)CONFIG_SHT41_SDA_GPIO, (gpio_num_t)CONFIG_SHT41_SCL_GPIO);
    
    #if !MOCK_SENSORS_BEHAVIOR
      // Инициализация дескриптора устройства SHT41
      // Библиотека sht4x сама создаёт I2C мастер и добавляет устройство
      memset(&s_dev, 0, sizeof(sht4x_t));
      err = sht4x_init_desc(&s_dev, CONFIG_SHT41_I2C_PORT, (gpio_num_t)CONFIG_SHT41_SDA_GPIO, (gpio_num_t)CONFIG_SHT41_SCL_GPIO);
      if(err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Ошибка инициализации дескриптора SHT41: %s", esp_err_to_name(err));
        return err;
      }

      // Установка частоты I2C
      err = sht4x_init(&s_dev);
      if(err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Ошибка инициализации датчика SHT41: %s", esp_err_to_name(err));
        //sht4x_free_desc(&s_dev);
        return err;
      }
    #endif
    s_initialized = true;

    ESP_LOGI("", "");
    ESP_LOGI("", "##############################");
    ESP_LOGI("", "#                            #");
    ESP_LOGI("", "#     SHT41: initialized     #");
    ESP_LOGI("", "#                            #");
    ESP_LOGI("", "##############################");
    ESP_LOGI("", "");

    return err;
}

/**
 * @brief Выполнить измерение и заполнить структуру результата
 */
esp_err_t sht41_read_all(int16_t *temperature, uint16_t *humidity)
{
  esp_err_t err = ESP_OK;

  float temp, hum;
    
  #if !MOCK_SENSORS_BEHAVIOR
    if (!s_initialized) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Драйвер не инициализирован");
        return ESP_ERR_INVALID_STATE;
    }

    err = sht4x_measure(&s_dev, &temp, &hum);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Ошибка измерения SHT41: %s", esp_err_to_name(err));
        return err;
    }
  #else
    temp = 26.77;
    hum  = 41.33;
  #endif

  #if DEBUG_MODE
    ESP_LOGW("|  SHT41", "Temp: %.2f °C, Hum: %.2f %%", temp, hum);
  #else
    ESP_LOGD("|  SHT41", "Temp: %.2f °C, Hum: %.2f %%", temp, hum);
  #endif

  *temperature = (int16_t)(temp * 100.0f);
  *humidity = (uint16_t)(hum * 100.0f);

  return err;
}

/**
 * @brief Освобождение ресурсов
 */
esp_err_t sht41_driver_deinit(void)
{
    if (!s_initialized) {
        return ESP_OK;
    }

    sht4x_free_desc(&s_dev);
    s_initialized = false;
    ESP_LOGI(TAG_MULTI_CLIMATE, "Драйвер SHT41 освобождён");

    return ESP_OK;
}

esp_err_t sht41_reset()
{
    return ESP_OK;
}
