#include "sensor_driver_bme280.h"
#include "app_priv.h"
#include "i2c_bus.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <string.h>

// BME280 Register Map
#define BME280_REG_ID           0xD0
#define BME280_REG_RESET        0xE0
#define BME280_REG_CTRL_HUM     0xF2
#define BME280_REG_STATUS       0xF3
#define BME280_REG_CTRL_MEAS    0xF4
#define BME280_REG_CONFIG       0xF5
#define BME280_REG_PRESS_MSB    0xF7
#define BME280_REG_TEMP_MSB     0xFA
#define BME280_REG_HUM_MSB      0xFD

#define BME280_CHIP_ID          0x60
#define BME280_RESET_CMD        0xB6

// Timing constants
#define BME280_MEASUREMENT_TIME_MS 50
#define BME280_I2C_TIMEOUT_MS   1000
#define BME280_I2C_FREQ_HZ      100000

// Calibration data structure
typedef struct {
    uint16_t dig_T1;
    int16_t dig_T2;
    int16_t dig_T3;
    uint16_t dig_P1;
    int16_t dig_P2;
    int16_t dig_P3;
    int16_t dig_P4;
    int16_t dig_P5;
    int16_t dig_P6;
    int16_t dig_P7;
    int16_t dig_P8;
    int16_t dig_P9;
    uint8_t dig_H1;
    int16_t dig_H2;
    uint8_t dig_H3;
    int16_t dig_H4;
    int16_t dig_H5;
    int8_t dig_H6;
} bme280_calib_data_t;

static const char *TAG = "BME280";

// ИЗМЕНЕНО: используем функции из i2c_bus, а не дублируем их

// Запись в регистр
static esp_err_t bme280_write_reg(bme280_dev_t *dev, uint8_t reg, uint8_t value)
{
    uint8_t write_buf[2] = {reg, value};
    
    esp_err_t ret = i2c_master_transmit(dev->dev_handle, write_buf, sizeof(write_buf), 
                                         pdMS_TO_TICKS(BME280_I2C_TIMEOUT_MS));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C write failed: %d", ret);
    }
    
    return ret;
}

// Чтение из регистра
static esp_err_t bme280_read_reg(bme280_dev_t *dev, uint8_t reg, uint8_t *data, size_t len)
{
    esp_err_t ret = i2c_master_transmit_receive(dev->dev_handle, &reg, 1, data, len,
                                                pdMS_TO_TICKS(BME280_I2C_TIMEOUT_MS));
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "I2C read failed: %d", ret);
    }
    
    return ret;
}

// Чтение калибровочных данных
static esp_err_t bme280_read_calibration(bme280_dev_t *dev, bme280_calib_data_t *calib)
{
    uint8_t buffer[26];
    
    // Read temperature and pressure calibration (0x88-0xA1)
    esp_err_t ret = bme280_read_reg(dev, 0x88, buffer, 24);
    if (ret != ESP_OK) return ret;
    
    calib->dig_T1 = (uint16_t)buffer[0] | ((uint16_t)buffer[1] << 8);
    calib->dig_T2 = (int16_t)buffer[2] | ((int16_t)buffer[3] << 8);
    calib->dig_T3 = (int16_t)buffer[4] | ((int16_t)buffer[5] << 8);
    calib->dig_P1 = (uint16_t)buffer[6] | ((uint16_t)buffer[7] << 8);
    calib->dig_P2 = (int16_t)buffer[8] | ((int16_t)buffer[9] << 8);
    calib->dig_P3 = (int16_t)buffer[10] | ((int16_t)buffer[11] << 8);
    calib->dig_P4 = (int16_t)buffer[12] | ((int16_t)buffer[13] << 8);
    calib->dig_P5 = (int16_t)buffer[14] | ((int16_t)buffer[15] << 8);
    calib->dig_P6 = (int16_t)buffer[16] | ((int16_t)buffer[17] << 8);
    calib->dig_P7 = (int16_t)buffer[18] | ((int16_t)buffer[19] << 8);
    calib->dig_P8 = (int16_t)buffer[20] | ((int16_t)buffer[21] << 8);
    calib->dig_P9 = (int16_t)buffer[22] | ((int16_t)buffer[23] << 8);
    
    // Read humidity calibration (0xE1-0xE7)
    ret = bme280_read_reg(dev, 0xE1, buffer, 7);
    if (ret != ESP_OK) return ret;
    
    calib->dig_H1 = buffer[0];
    calib->dig_H2 = (int16_t)buffer[1] | ((int16_t)buffer[2] << 8);
    calib->dig_H3 = buffer[3];
    calib->dig_H4 = (int16_t)buffer[4] | ((int16_t)(buffer[5] & 0x0F) << 8);
    calib->dig_H5 = (int16_t)(buffer[5] >> 4) | ((int16_t)buffer[6] << 4);
    calib->dig_H6 = (int8_t)buffer[7];
    
    return ESP_OK;
}

// Компенсация температуры
static int32_t bme280_compensate_temperature(int32_t adc_T, bme280_calib_data_t *calib, int32_t *t_fine)
{
    int32_t var1, var2, T;
    
    var1 = ((((adc_T >> 3) - ((int32_t)calib->dig_T1 << 1))) * ((int32_t)calib->dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)calib->dig_T1)) * ((adc_T >> 4) - ((int32_t)calib->dig_T1))) >> 12) *
            ((int32_t)calib->dig_T3)) >> 14;
    
    *t_fine = var1 + var2;
    T = (*t_fine * 5 + 128) >> 8;
    
    return T;
}

// Компенсация давления
static uint32_t bme280_compensate_pressure(int32_t adc_P, bme280_calib_data_t *calib, int32_t t_fine)
{
    int64_t var1, var2, P;
    
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)calib->dig_P6;
    var2 = var2 + ((var1 * (int64_t)calib->dig_P5) << 17);
    var2 = var2 + (((int64_t)calib->dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)calib->dig_P3) >> 8) + ((var1 * (int64_t)calib->dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)calib->dig_P1) >> 33;
    
    if (var1 == 0) {
        return 0;
    }
    
    P = 1048576 - adc_P;
    P = (((P << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)calib->dig_P9) * (P >> 13) * (P >> 13)) >> 25;
    var2 = (((int64_t)calib->dig_P8) * P) >> 19;
    P = ((P + var1 + var2) >> 8) + (((int64_t)calib->dig_P7) << 4);
    
    return (uint32_t)P;
}

// Компенсация влажности
static uint32_t bme280_compensate_humidity(int32_t adc_H, bme280_calib_data_t *calib, int32_t t_fine)
{
    int32_t v_x1_u32r;
    
    v_x1_u32r = (t_fine - ((int32_t)76800));
    v_x1_u32r = (((((adc_H << 14) - (((int32_t)calib->dig_H4) << 20) - (((int32_t)calib->dig_H5) * v_x1_u32r)) +
                  ((int32_t)16384)) >> 15) * (((((((v_x1_u32r * ((int32_t)calib->dig_H6)) >> 10) *
                  (((v_x1_u32r * ((int32_t)calib->dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                  ((int32_t)calib->dig_H2) + 8192) >> 14));
    
    v_x1_u32r = (v_x1_u32r - (((((v_x1_u32r >> 15) * (v_x1_u32r >> 15)) >> 7) * ((int32_t)calib->dig_H1)) >> 4));
    v_x1_u32r = (v_x1_u32r < 0) ? 0 : v_x1_u32r;
    v_x1_u32r = (v_x1_u32r > 419430400) ? 419430400 : v_x1_u32r;
    
    return (uint32_t)(v_x1_u32r >> 12);
}

// Инициализация BME280
esp_err_t bme280_init(bme280_dev_t *dev, gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t i2c_port, uint8_t i2c_addr)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->sda_pin = sda_pin;
    dev->scl_pin = scl_pin;
    dev->i2c_port = i2c_port;
    dev->i2c_addr = i2c_addr;
    dev->initialized = false;
    
    // Инициализация I2C шины через общий менеджер
    esp_err_t err = i2c_bus_init(sda_pin, scl_pin, i2c_port);
    if (err != ESP_OK) {
        return err;
    }
    
    // Добавление устройства на шину
    err = i2c_bus_add_device(i2c_addr, BME280_I2C_FREQ_HZ, &dev->dev_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add device to I2C bus: %d", err);
        return err;
    }
    
    // Check chip ID
    uint8_t chip_id = 0;
    err = bme280_read_reg(dev, BME280_REG_ID, &chip_id, 1);
    if (err != ESP_OK || chip_id != BME280_CHIP_ID) {
        ESP_LOGE(TAG, "Invalid BME280 chip ID: 0x%02x", chip_id);
        i2c_master_bus_rm_device(dev->dev_handle);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Reset sensor
    err = bme280_write_reg(dev, BME280_REG_RESET, BME280_RESET_CMD);
    if (err != ESP_OK) {
        i2c_master_bus_rm_device(dev->dev_handle);
        return err;
    }
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // Configure sensor
    err = bme280_write_reg(dev, BME280_REG_CTRL_HUM, 0x01);
    if (err != ESP_OK) {
        i2c_master_bus_rm_device(dev->dev_handle);
        return err;
    }
    
    err = bme280_write_reg(dev, BME280_REG_CTRL_MEAS, 0x27);
    if (err != ESP_OK) {
        i2c_master_bus_rm_device(dev->dev_handle);
        return err;
    }
    
    err = bme280_write_reg(dev, BME280_REG_CONFIG, 0x00);
    if (err != ESP_OK) {
        i2c_master_bus_rm_device(dev->dev_handle);
        return err;
    }
    
    dev->initialized = true;
    ESP_LOGI(TAG, "BME280 initialized on I2C port %d, addr 0x%02x", i2c_port, i2c_addr);
    
    return ESP_OK;
}

// Чтение всех данных
esp_err_t bme280_read_all(bme280_dev_t *dev, int16_t *temperature, uint16_t *humidity, int16_t *pressure)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    bme280_calib_data_t calib;
    esp_err_t err = bme280_read_calibration(dev, &calib);
    if (err != ESP_OK) return err;
    
    // Trigger measurement
    err = bme280_write_reg(dev, BME280_REG_CTRL_MEAS, 0x27);
    if (err != ESP_OK) return err;
    
    vTaskDelay(pdMS_TO_TICKS(BME280_MEASUREMENT_TIME_MS));
    
    // Read raw data
    uint8_t data[8];
    err = bme280_read_reg(dev, BME280_REG_PRESS_MSB, data, 8);
    if (err != ESP_OK) return err;
    
    int32_t adc_P = (int32_t)data[0] << 12 | (int32_t)data[1] << 4 | (int32_t)data[2] >> 4;
    int32_t adc_T = (int32_t)data[3] << 12 | (int32_t)data[4] << 4 | (int32_t)data[5] >> 4;
    int32_t adc_H = (int32_t)data[6] << 8 | (int32_t)data[7];
    
    int32_t t_fine;
    int32_t temp_raw = bme280_compensate_temperature(adc_T, &calib, &t_fine);
    uint32_t press_raw = bme280_compensate_pressure(adc_P, &calib, t_fine);
    uint32_t hum_raw = bme280_compensate_humidity(adc_H, &calib, t_fine);
    
    if (temperature) {
        *temperature = (int16_t)(temp_raw / 100);
    }
    
    if (humidity) {
        *humidity = (uint16_t)(hum_raw / 1024);
    }
    
    if (pressure) {
        *pressure = (int16_t)(press_raw / 100);
    }
    
    dev->last_temperature = temp_raw / 100;
    dev->last_humidity = hum_raw / 1024;
    dev->last_pressure = press_raw / 100;
    dev->last_read_time = esp_timer_get_time();
    
    return ESP_OK;
}

esp_err_t bme280_read_temperature(bme280_dev_t *dev, int16_t *temperature)
{
    return bme280_read_all(dev, temperature, NULL, NULL);
}

esp_err_t bme280_read_humidity(bme280_dev_t *dev, uint16_t *humidity)
{
    return bme280_read_all(dev, NULL, humidity, NULL);
}

esp_err_t bme280_read_pressure(bme280_dev_t *dev, int16_t *pressure)
{
    return bme280_read_all(dev, NULL, NULL, pressure);
}

esp_err_t bme280_reset(bme280_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t err = bme280_write_reg(dev, BME280_REG_RESET, BME280_RESET_CMD);
    if (err != ESP_OK) return err;
    
    vTaskDelay(pdMS_TO_TICKS(10));
    
    err = bme280_write_reg(dev, BME280_REG_CTRL_HUM, 0x01);
    if (err != ESP_OK) return err;
    
    err = bme280_write_reg(dev, BME280_REG_CTRL_MEAS, 0x27);
    if (err != ESP_OK) return err;
    
    err = bme280_write_reg(dev, BME280_REG_CONFIG, 0x00);
    
    ESP_LOGI(TAG, "BME280 reset completed");
    return err;
}