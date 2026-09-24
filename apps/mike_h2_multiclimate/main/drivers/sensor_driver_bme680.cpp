#include "sensor_driver_bme680.h"
#include "app_priv.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <driver/i2c.h>
#include <string.h>

// BME680 Register Map
#define BME680_REG_CHIP_ID          0xD0
#define BME680_REG_RESET             0xE0
#define BME680_REG_STATUS            0x73
#define BME680_REG_CTRL_MEAS        0x74
#define BME680_REG_CONFIG            0x75
#define BME680_REG_CTRL_HUM          0x72
#define BME680_REG_CTRL_GAS_1        0x71
#define BME680_REG_CTRL_GAS_0        0x70
#define BME680_REG_GAS_R_LSB         0x2B
#define BME680_REG_GAS_R_MSB         0x2A
#define BME680_REG_HUM_LSB           0x26
#define BME680_REG_HUM_MSB           0x25
#define BME680_REG_TEMP_LSB          0x24
#define BME680_REG_TEMP_MSB          0x23
#define BME680_REG_PRESS_LSB         0x22
#define BME680_REG_PRESS_MSB         0x21
#define BME680_REG_EAS_STATUS        0x1D
#define BME680_REG_MEAS_STAT_0       0x1D

#define BME680_CHIP_ID               0x61
#define BME680_RESET_CMD             0xB6

// Timing constants
#define BME680_MEASUREMENT_TIME_MS   100
#define BME680_I2C_TIMEOUT_MS        1000
#define BME680_I2C_FREQ_HZ           100000

static const char *TAG = "BME680";

static esp_err_t bme680_write_reg(bme680_dev_t *dev, uint8_t reg, uint8_t value)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_write_byte(cmd, value, true);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin((i2c_port_t)dev->i2c_bus, cmd, pdMS_TO_TICKS(BME680_I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %d", ret);
    }
    
    return ret;
}

static esp_err_t bme680_read_reg(bme680_dev_t *dev, uint8_t reg, uint8_t *data, size_t len)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(cmd, reg, true);
    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (dev->i2c_addr << 1) | I2C_MASTER_READ, true);
    
    if (len > 1) {
        i2c_master_read(cmd, data, len - 1, I2C_MASTER_ACK);
    }
    i2c_master_read_byte(cmd, data + len - 1, I2C_MASTER_NACK);
    i2c_master_stop(cmd);
    
    esp_err_t ret = i2c_master_cmd_begin((i2c_port_t)dev->i2c_bus, cmd, pdMS_TO_TICKS(BME680_I2C_TIMEOUT_MS));
    i2c_cmd_link_delete(cmd);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %d", ret);
    }
    
    return ret;
}

esp_err_t bme680_init(bme680_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t i2c_bus, uint8_t i2c_addr)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->sda_pin = sda_pin;
    dev->scl_pin = scl_pin;
    dev->i2c_bus = i2c_bus;
    dev->i2c_addr = i2c_addr;
    dev->initialized = false;
    
    // Configure I2C
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master = { .clk_speed = BME680_I2C_FREQ_HZ },
        .clk_flags = 0
    };
    
    esp_err_t err = i2c_param_config(i2c_bus, &conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C config failed: %d", err);
        return err;
    }
    
    err = i2c_driver_install(i2c_bus, I2C_MODE_MASTER, 0, 0, 0);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "I2C driver install failed: %d", err);
        return err;
    }
    
    // Check chip ID
    uint8_t chip_id = 0;
    err = bme680_read_reg(dev, BME680_REG_CHIP_ID, &chip_id, 1);
    if (err != ESP_OK || chip_id != BME680_CHIP_ID) {
        ESP_LOGE(TAG, "Invalid BME680 chip ID: 0x%02x", chip_id);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Reset sensor
    err = bme680_write_reg(dev, BME680_REG_RESET, BME680_RESET_CMD);
    if (err != ESP_OK) return err;
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Configure sensor for forced mode with oversampling
    // Set humidity oversampling to x2
    err = bme680_write_reg(dev, BME680_REG_CTRL_HUM, 0x02);
    if (err != ESP_OK) return err;
    
    // Set temperature oversampling x2, pressure oversampling x16
    err = bme680_write_reg(dev, BME680_REG_CTRL_MEAS, 0x6C); // 0b01101100
    if (err != ESP_OK) return err;
    
    // Set gas sensor settings
    err = bme680_write_reg(dev, BME680_REG_CTRL_GAS_1, 0x00); // Gas sensor off for now
    if (err != ESP_OK) return err;
    
    // Set filter and standby time
    err = bme680_write_reg(dev, BME680_REG_CONFIG, 0x00);
    if (err != ESP_OK) return err;
    
    dev->initialized = true;
    ESP_LOGI(TAG, "BME680 initialized on I2C bus %d, addr 0x%02x", i2c_bus, i2c_addr);
    
    return ESP_OK;
}

// Simplified compensation functions (for full implementation, you'd need calibration data)
// This is a simplified version - production code should include full calibration compensation
esp_err_t bme680_read_all(bme680_dev_t *dev, int16_t *temperature, uint16_t *humidity, 
                          int16_t *pressure, uint32_t *gas_resistance)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Trigger measurement
    esp_err_t err = bme680_write_reg(dev, BME680_REG_CTRL_MEAS, 0x6D); // Forced mode
    if (err != ESP_OK) return err;
    
    vTaskDelay(pdMS_TO_TICKS(BME680_MEASUREMENT_TIME_MS));
    
    // Read measurement status
    uint8_t status;
    err = bme680_read_reg(dev, BME680_REG_EAS_STATUS, &status, 1);
    if (err != ESP_OK) return err;
    
    if (!(status & 0x20)) { // Check new data bit
        ESP_LOGW(TAG, "Measurement not ready");
        return ESP_ERR_NOT_FINISHED;
    }
    
    // Read raw data
    uint8_t data[15];
    err = bme680_read_reg(dev, 0x1D, data, 15);
    if (err != ESP_OK) return err;
    
    // Extract raw values (simplified - would need calibration data for accurate values)
    uint32_t raw_temp = ((uint32_t)data[5] << 12) | ((uint32_t)data[6] << 4) | ((uint32_t)data[7] >> 4);
    uint32_t raw_press = ((uint32_t)data[2] << 12) | ((uint32_t)data[3] << 4) | ((uint32_t)data[4] >> 4);
    uint32_t raw_hum = ((uint32_t)data[8] << 8) | (uint32_t)data[9];
    uint16_t raw_gas = ((uint16_t)data[13] << 2) | ((uint16_t)data[14] >> 6);
    uint8_t gas_range = data[14] & 0x0F;
    
    // Simplified conversions (production code would use calibration)
    // Temperature in 0.01°C (rough estimate)
    int32_t temp_comp = (int32_t)(((raw_temp * 100) / 10000) + 2500); // Rough compensation
    
    // Humidity in 0.01% (rough estimate)
    uint32_t hum_comp = (raw_hum * 1000) / 1024; // Rough compensation
    
    // Pressure in Pa (rough estimate)
    int32_t press_comp = (int32_t)((raw_press * 1000) / 256) + 100000; // Rough compensation
    
    // Gas resistance in Ohms (rough estimate)
    uint32_t gas_lut[16] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
    uint32_t gas_comp = gas_lut[gas_range] * raw_gas;
    
    if (temperature) {
        *temperature = (int16_t)temp_comp;
    }
    
    if (humidity) {
        *humidity = (uint16_t)hum_comp;
    }
    
    if (pressure) {
        *pressure = (int16_t)press_comp;
    }
    
    if (gas_resistance) {
        *gas_resistance = gas_comp;
    }
    
    dev->last_temperature = temp_comp;
    dev->last_humidity = hum_comp;
    dev->last_pressure = press_comp;
    dev->last_gas_resistance = gas_comp;
    dev->last_read_time = esp_timer_get_time();
    
    return ESP_OK;
}

esp_err_t bme680_read_temperature(bme680_dev_t *dev, int16_t *temperature)
{
    return bme680_read_all(dev, temperature, NULL, NULL, NULL);
}

esp_err_t bme680_read_humidity(bme680_dev_t *dev, uint16_t *humidity)
{
    return bme680_read_all(dev, NULL, humidity, NULL, NULL);
}

esp_err_t bme680_read_pressure(bme680_dev_t *dev, int16_t *pressure)
{
    return bme680_read_all(dev, NULL, NULL, pressure, NULL);
}

esp_err_t bme680_read_gas(bme680_dev_t *dev, uint32_t *gas_resistance)
{
    return bme680_read_all(dev, NULL, NULL, NULL, gas_resistance);
}

esp_err_t bme680_reset(bme680_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = bme680_write_reg(dev, BME680_REG_RESET, BME680_RESET_CMD);
    if (err != ESP_OK) return err;
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Reconfigure
    err = bme680_write_reg(dev, BME680_REG_CTRL_HUM, 0x02);
    if (err != ESP_OK) return err;
    
    err = bme680_write_reg(dev, BME680_REG_CTRL_MEAS, 0x6C);
    if (err != ESP_OK) return err;
    
    err = bme680_write_reg(dev, BME680_REG_CTRL_GAS_1, 0x00);
    if (err != ESP_OK) return err;
    
    err = bme680_write_reg(dev, BME680_REG_CONFIG, 0x00);
    
    ESP_LOGI(TAG, "BME680 reset completed");
    return err;
}
