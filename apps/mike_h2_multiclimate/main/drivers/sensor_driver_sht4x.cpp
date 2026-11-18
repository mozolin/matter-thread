#include "sensor_driver_sht4x.h"
#include "app_priv.h"

// Дескриптор устройства SHT4X
sht4x_t sht4x_dev;
// Флаг инициализации
bool sht4x_initialized = false;

esp_err_t sht4x_init_desc_adv(sht4x_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
{
  dev->i2c_dev.port = port;
  dev->i2c_dev.addr = SHT4X_I2C_ADDRESS;
  dev->i2c_dev.cfg.sda_io_num = sda_gpio;
  dev->i2c_dev.cfg.scl_io_num = scl_gpio;
  // internal Pull-ups
  dev->i2c_dev.cfg.sda_pullup_en = true;
  dev->i2c_dev.cfg.scl_pullup_en = true;
  #ifdef CONFIG_I2C_FREQ_HZ
    dev->i2c_dev.cfg.master.clk_speed = CONFIG_I2C_FREQ_HZ;
  #else
    dev->i2c_dev.cfg.master.clk_speed = SENSOR_I2C_FREQ_HZ;
  #endif

  return i2c_dev_create_mutex(&dev->i2c_dev);
}

/**
 * @brief Инициализация I2C и датчика SHT4X
 */
esp_err_t sht4x_init_app(void)
{
    esp_err_t err = ESP_OK;
    
    if(sht4x_initialized) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGW(CONFIG_TAG_MIKE_APP, "Драйвер уже инициализирован");
      #endif
      return ESP_OK;
    }

    #if CONFIG_DEBUG_MODE
      ESP_LOGI(CONFIG_TAG_MIKE_APP, "Инициализация SHT4X: порт=%d, SDA=%d, SCL=%d",
             (i2c_port_t)CONFIG_SHT4X_I2C_PORT, (gpio_num_t)CONFIG_SHT4X_SDA_GPIO, (gpio_num_t)CONFIG_SHT4X_SCL_GPIO);
    #endif
    
    #if !CONFIG_MOCK_SENSORS_BEHAVIOR
      // Инициализация дескриптора устройства SHT4X
      // Библиотека sht4x сама создаёт I2C мастер и добавляет устройство
      memset(&sht4x_dev, 0, sizeof(sht4x_t));
      //err = sht4x_init_desc(&sht4x_dev, (i2c_port_t)CONFIG_SHT4X_I2C_PORT, (gpio_num_t)CONFIG_SHT4X_SDA_GPIO, (gpio_num_t)CONFIG_SHT4X_SCL_GPIO);
      err = sht4x_init_desc_adv(&sht4x_dev, (i2c_port_t)CONFIG_SHT4X_I2C_PORT, (gpio_num_t)CONFIG_SHT4X_SDA_GPIO, (gpio_num_t)CONFIG_SHT4X_SCL_GPIO);
      if(err != ESP_OK) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка инициализации дескриптора SHT4X: %s", esp_err_to_name(err));
        #endif
        return err;
      }

      // Установка частоты I2C
      err = sht4x_init(&sht4x_dev);
      if(err != ESP_OK) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка инициализации датчика SHT4X: %s", esp_err_to_name(err));
        #endif
        //sht4x_free_desc(&sht4x_dev);
        return err;
      }
    #endif
    sht4x_initialized = true;

    #if CONFIG_DEBUG_MODE
      ESP_LOGI("", "");
      ESP_LOGI("", "##############################");
      ESP_LOGI("", "#                            #");
      ESP_LOGI("", "#     SHT4X: initialized     #");
      ESP_LOGI("", "#                            #");
      ESP_LOGI("", "##############################");
      ESP_LOGI("", "");
    #endif

    return err;
}

/**
 * @brief Выполнить измерение и заполнить структуру результата
 */
esp_err_t sht4x_read_app(int16_t *temperature, uint16_t *humidity)
{
  esp_err_t err = ESP_OK;

  float temp, hum;
    
  #if !CONFIG_MOCK_SENSORS_BEHAVIOR
    if (!sht4x_initialized) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Драйвер не инициализирован");
        #endif
        return ESP_ERR_INVALID_STATE;
    }

    err = sht4x_measure(&sht4x_dev, &temp, &hum);
    if (err != ESP_OK) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка измерения SHT4X: %s", esp_err_to_name(err));
        #endif
        return err;
    }
  #else
    temp = 26.77;
    hum  = 41.33;
  #endif

  #if CONFIG_DEBUG_MODE
    ESP_LOGW("|  SHT4X", "Temp: %.2f °C, Hum: %.2f %%", temp, hum);
  #else
    ESP_LOGD("|  SHT4X", "Temp: %.2f °C, Hum: %.2f %%", temp, hum);
  #endif

  *temperature = (int16_t)(temp * 100.0f);
  *humidity = (uint16_t)(hum * 100.0f);

  return err;
}

/**
 * @brief Освобождение ресурсов
 */
esp_err_t sht4x_driver_deinit(void)
{
    if (!sht4x_initialized) {
        return ESP_OK;
    }

    sht4x_free_desc(&sht4x_dev);
    sht4x_initialized = false;
    #if CONFIG_DEBUG_MODE
      ESP_LOGI(CONFIG_TAG_MIKE_APP, "Драйвер SHT4X освобождён");
    #endif

    return ESP_OK;
}

esp_err_t sht4x_reset_app()
{
    return ESP_OK;
}
