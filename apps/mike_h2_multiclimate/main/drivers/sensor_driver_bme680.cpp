#include "sensor_driver_bme680.h"
#include "app_priv.h"

esp_err_t bme680_init(bme680_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin,
                      i2c_port_t i2c_bus, uint8_t i2c_addr)
{
    if (!dev) return ESP_ERR_INVALID_ARG;

    dev->sda_pin = sda_pin;
    dev->scl_pin = scl_pin;
    dev->i2c_addr = i2c_addr;

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
        ESP_LOGE(TAG_MULTI_SENSOR, "BME680: i2c_new_master_bus failed: %d", err);
        return err;
    }

    err = bme680_init_desc(&dev->dev, i2c_addr, i2c_bus, sda_pin, scl_pin);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "BME680: bme680_init_desc failed: %d", err);
        return err;
    }

    err = bme680_init(&dev->dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "BME680: bme680_init failed: %d", err);
        return err;
    }

    // Настройка oversampling и фильтра (опционально)
    bme680_set_oversampling_rates(&dev->dev, BME680_OSR_4X, BME680_OSR_4X, BME680_OSR_2X);
    bme680_set_filter_size(&dev->dev, BME680_IIR_SIZE_3);
    bme680_set_heater_profile(&dev->dev, 0, 200, 100);  // 200°C, 100ms

    dev->initialized = true;
    ESP_LOGI(TAG_MULTI_SENSOR, "BME680 initialized at 0x%02X", i2c_addr);
    return ESP_OK;
}

esp_err_t bme680_read_all(bme680_dev_t *dev, int16_t *temperature, uint16_t *humidity,
                          int16_t *pressure, uint32_t *gas_resistance)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;

    bme680_values_float_t values;
    esp_err_t err = bme680_measure_float(&dev->dev, &values);
    if (err != ESP_OK) return err;

    *temperature = (int16_t)(values.temperature * 100.0f);
    *humidity = (uint16_t)(values.humidity * 100.0f);
    *pressure = (int16_t)values.pressure;
    *gas_resistance = (uint32_t)values.gas_resistance;
    return ESP_OK;
}

esp_err_t bme680_reset(bme680_dev_t *dev)
{
    if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return bme680_init(&dev->dev);
}
