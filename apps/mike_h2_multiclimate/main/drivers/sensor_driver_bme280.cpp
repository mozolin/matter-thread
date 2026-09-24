#include "sensor_driver_bme280.h"
#include "app_priv.h"

esp_err_t bme280_init(bme280_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin,
                      i2c_port_t i2c_bus, uint8_t i2c_addr)
{
    if (!dev) return ESP_ERR_INVALID_ARG;

    dev->sda_pin = sda_pin;
    dev->scl_pin = scl_pin;
    dev->i2c_addr = i2c_addr;

    // Инициализация I2C master bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = i2c_bus,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    esp_err_t err = i2c_new_master_bus(&bus_cfg, &dev->bus_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "BME280: i2c_new_master_bus failed: %d", err);
        return err;
    }

    // Инициализация bmp280 через i2cdev
    err = bmp280_init_desc(&dev->dev, i2c_addr, i2c_bus, sda_pin, scl_pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "BME280: bmp280_init_desc failed: %d", err);
        return err;
    }

    err = bmp280_init(&dev->dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "BME280: bmp280_init failed: %d", err);
        return err;
    }

    dev->initialized = true;
    ESP_LOGI(TAG_MULTI_SENSOR, "BME280 initialized at 0x%02X", i2c_addr);
    return ESP_OK;
}

esp_err_t bme280_read_all(bme280_dev_t *dev, int16_t *temperature, uint16_t *humidity, int16_t *pressure)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;

    float temp, press, hum;
    esp_err_t err = bmp280_get_forced_all(&dev->dev, &temp, &press, &hum, true);
    if (err != ESP_OK) return err;

    // Matter ожидает 0.01°C, 0.01%, Па
    *temperature = (int16_t)(temp * 100.0f);
    *humidity = (uint16_t)(hum * 100.0f);
    *pressure = (int16_t)press;  // Па
    return ESP_OK;
}

esp_err_t bme280_reset(bme280_dev_t *dev)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return bmp280_init(&dev->dev);
}
