#include <cstring>
#include "sdkconfig.h"
#include "i2c_adv.h"

#include "soc/soc_caps.h"
#include "driver/i2c_types.h"




#if CONFIG_I2C_USE_IN_APP

  esp_err_t i2cdev_check_present(i2c_port_t port, gpio_num_t sda, gpio_num_t scl, uint8_t addr)
  {
    i2c_dev_t dev;
    esp_err_t err = ESP_OK;
    
    memset(&dev, 0, sizeof(i2c_dev_t));
    dev.port = port;
    dev.addr = addr;
    dev.cfg.sda_io_num = sda;
    dev.cfg.scl_io_num = scl;
    dev.cfg.sda_pullup_en = true;
    dev.cfg.scl_pullup_en = true;
    dev.cfg.master.clk_speed = 100000; // 100 кГц
    
    err = i2c_dev_check_present(&dev);
    i2c_dev_delete_mutex(&dev);
      
    return err;
  }

  typedef struct
  {
    uint8_t addr;
    const char *name;
  } i2c_known_dev_t;

  static const i2c_known_dev_t known_devs[] =
  {
    {0x3C, "SSD1306"},
    {0x3D, "SSD1306"},
    {0x44, "SHT4X"},
    {0x62, "SCD4X"},
    {0x76, "BME280"},
    {0x77, "BME680"},
  };

  #if CONFIG_CHECK_I2C_PORTS
    // Проверка одного порта
    void scan_port(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
    {
      esp_err_t err = ESP_OK;
      
      ESP_LOGE("", "Scanning PORT %d (SDA=%d, SCL=%d)...", port, sda, scl);
      
      // Перебор 7-битных адресов (0x08 ~ 0x77 — это диапазон, обычно используемый в стандартах)
      int found = 0;
      for (size_t i = 0; i < sizeof(known_devs)/sizeof(known_devs[0]); i++) {
        ESP_LOGW("", "Scanning 0x%02X (%s)", known_devs[i].addr, known_devs[i].name);
        vTaskDelay(pdMS_TO_TICKS(50));
        err = i2cdev_check_present(port, sda, scl, known_devs[i].addr);
        if(err == ESP_OK) {
          ESP_LOGI("", "  - Found device at 0x%02X", known_devs[i].addr);
          found++;
        }
        // при желании — небольшая пауза между пробами
        //vTaskDelay(pdMS_TO_TICKS(20));
      }
      if(found == 0) {
        ESP_LOGE("", "  - No devices found.");
      }
    }
  
    void check_i2c_ports()
    {
      ESP_LOGW("", "");
      ESP_LOGW("", "===============================================");
      // Для ESP32-H2 оба значения будут равны 2
      ESP_LOGW("", " Total number of ports I2C: %d", SOC_I2C_NUM);
      ESP_LOGW("", " Maximum number of ports (I2C_NUM_MAX): %d", I2C_NUM_MAX);
      ESP_LOGW("", "===============================================");
      ESP_LOGW("", "");
    
      // Предполагая, что на вашей ESP32-H2 используются следующие контакты (измените по необходимости)
      // Порт 0
      scan_port((i2c_port_t)CONFIG_I2C_0_PORT_NUM, (gpio_num_t)CONFIG_I2C_0_SDA_GPIO, (gpio_num_t)CONFIG_I2C_0_SCL_GPIO);
      // Порт 1 (если SOC_I2C_NUM == 2)
      if(SOC_I2C_NUM > 1) {
        scan_port((i2c_port_t)CONFIG_I2C_1_PORT_NUM, (gpio_num_t)CONFIG_I2C_1_SDA_GPIO, (gpio_num_t)CONFIG_I2C_1_SCL_GPIO);
      }
    
      ESP_LOGW("", "");
      ESP_LOGW("", "===============================================");
      ESP_LOGW("", "");
    }
  #endif // CONFIG_CHECK_I2C_PORTS

#endif // CONFIG_I2C_USE_IN_APP
