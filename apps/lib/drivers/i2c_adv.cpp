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
  esp_err_t sht4x_init_desc(sht4x_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
  {
    //CHECK_ARG(dev);

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
#endif

#if CONFIG_SCD4X_ENABLED
  esp_err_t scd4x_init_desc(i2c_dev_t *dev, i2c_port_t port, gpio_num_t sda_gpio, gpio_num_t scl_gpio)
  {
    //CHECK_ARG(dev);

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

    return i2c_dev_create_mutex(dev);
  }
#endif


//#include "i2cdev.h"
//#include "driver/i2c_master.h"
//#include "esp_log.h"

static const char *TAG_I2C_SCAN = "I2C_SCAN";

// Проверка одного порта
void scan_port(i2c_port_t port, gpio_num_t sda, gpio_num_t scl)
{
  // 1. Сначала используем i2cdev для инициализации "фиктивного устройства" на этом порту,
  //    чтобы библиотека создала и управляла базовой шиной
  i2c_dev_t dummy = {
    .port = port,
    .addr = 0x00, // Фиктивное, не используется
    .cfg = {
      .sda_io_num = sda,
      .scl_io_num = scl,
      // internal Pull-ups
      .sda_pullup_en = true,
      .scl_pullup_en = true,
      .master.clk_speed = 100000 // При сканировании можно использовать более низкую скорость для совместимости
    }
  };
  
  esp_err_t err = i2c_dev_create_mutex(&dummy);
  if(err != ESP_OK) {
    ESP_LOGE(TAG_I2C_SCAN, "Port %d: failed to init i2cdev (%s)", port, esp_err_to_name(err));
    return;
  }

  // 2. Получение дескриптора шины, управляемого i2cdev
  i2c_master_bus_handle_t bus_handle;
  err = i2cdev_get_shared_handle(port, (void **)&bus_handle);
  if(err != ESP_OK) {
    ESP_LOGE(TAG_I2C_SCAN, "Port %d: failed to get bus handle", port);
    i2c_dev_delete_mutex(&dummy);
    return;
  }

  ESP_LOGW(TAG_I2C_SCAN, "Scanning port %d (SDA=%d, SCL=%d)...", port, sda, scl);
  
  // 3. Перебор 7-битных адресов (0x08 ~ 0x77 — это диапазон, обычно используемый в стандартах)
  int found = 0;
  for(uint8_t addr = 0x08; addr <= 0x77; addr++) {
    // i2c_master_probe вернёт ESP_OK, если получит ACK
    if(i2c_master_probe(bus_handle, addr, 50) == ESP_OK) {
      ESP_LOGW(TAG_I2C_SCAN, "  Found device at 0x%02X", addr);
      found++;
    }
  }
  if(found == 0) {
    ESP_LOGW(TAG_I2C_SCAN, "  No devices found.");
  }

  // 4. Освобождение фиктивного устройства (базовая шина будет автоматически освобождена, когда не останется устройств)
  i2c_dev_delete_mutex(&dummy);
}

void check_i2c_ports()
{
  ESP_LOGW(TAG_I2C_SCAN, "");
  ESP_LOGW(TAG_I2C_SCAN, "===============================================");
  // Для ESP32-H2 оба значения будут равны 2
  ESP_LOGW(TAG_I2C_SCAN, " Total number of ports I2C: %d", SOC_I2C_NUM);
  ESP_LOGW(TAG_I2C_SCAN, " Maximum number of ports (I2C_NUM_MAX): %d", I2C_NUM_MAX);
  ESP_LOGW(TAG_I2C_SCAN, "===============================================");
  ESP_LOGW(TAG_I2C_SCAN, "");

  // Предполагая, что на вашей ESP32-H2 используются следующие контакты (измените по необходимости)
  // Порт 0
  scan_port(CONFIG_I2C_0_PORT_NUM, CONFIG_I2C_0_SDA_GPIO, CONFIG_I2C_0_SCL_GPIO);
  // Порт 1 (если SOC_I2C_NUM == 2)
  if(SOC_I2C_NUM > 1) {
    scan_port(CONFIG_I2C_1_PORT_NUM, CONFIG_I2C_1_SDA_GPIO, CONFIG_I2C_1_SCL_GPIO);
  }

  ESP_LOGW(TAG_I2C_SCAN, "");
  ESP_LOGW(TAG_I2C_SCAN, "===============================================");
  ESP_LOGW(TAG_I2C_SCAN, "");
}
