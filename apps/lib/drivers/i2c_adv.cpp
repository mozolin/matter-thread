#include "app_priv.h"
#include "i2c_adv.h"

#include "soc/soc_caps.h"
#include "driver/i2c_types.h"


#define SENSOR_I2C_FREQ_HZ 1000000 // Up to 3.4MHz, but esp-idf only supports 1MHz


#if CONFIG_BME280_ENABLED
  esp_err_t bmp280_init_desc_adv(bmp280_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
  {
    //CHECK_ARG(dev);

    if(addr != BMP280_I2C_ADDRESS_0 && addr != BMP280_I2C_ADDRESS_1)
    {
      #if DEBUG_MODE
        ESP_LOGE("BME280", "Invalid I2C address");
      #endif
      return ESP_ERR_INVALID_ARG;
    }

    dev->i2c_dev.port = port;
    dev->i2c_dev.addr = addr;
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
#endif

#if CONFIG_BME680_ENABLED
  esp_err_t bme680_init_desc_adv(bme680_t *dev, uint8_t addr, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
  {
    //CHECK_ARG(dev);

    if(addr != BME680_I2C_ADDR_0 &&  addr != BME680_I2C_ADDR_1)
    {
      #if DEBUG_MODE
        ESP_LOGE("BME680", "Invalid I2C address");
      #endif
      return ESP_ERR_INVALID_ARG;
    }

    dev->i2c_dev.port = port;
    dev->i2c_dev.addr = addr;
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
#endif


#if CONFIG_SHT4X_ENABLED
  esp_err_t sht4x_init_desc_adv(sht4x_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
  {
    //CHECK_ARG(dev);

    dev->i2c_dev.port = port;
    dev->i2c_dev.addr = SHT4X_I2C_ADDRESS;
    dev->i2c_dev.cfg.sda_io_num = sda_gpio;
    dev->i2c_dev.cfg.scl_io_num = scl_gpio;
    #ifdef CONFIG_I2C_FREQ_HZ
      dev->i2c_dev.cfg.master.clk_speed = CONFIG_I2C_FREQ_HZ;
    #else
      dev->i2c_dev.cfg.master.clk_speed = SENSOR_I2C_FREQ_HZ;
    #endif

    return i2c_dev_create_mutex(&dev->i2c_dev);
}
#endif


void check_i2c_ports()
{
  // Для ESP32-H2 оба значения будут равны 2
  ESP_LOGI("I2C_INFO", "Общее число портов I2C: %d", SOC_I2C_NUM);
  ESP_LOGI("I2C_INFO", "Максимум портов (I2C_NUM_MAX): %d", I2C_NUM_MAX);
}
