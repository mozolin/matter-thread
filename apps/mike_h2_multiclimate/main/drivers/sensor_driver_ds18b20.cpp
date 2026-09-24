#include "sensor_driver_ds18b20.h"
#include "app_priv.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>

// DS18B20 Commands
#define DS18B20_CMD_SKIP_ROM      0xCC
#define DS18B20_CMD_CONVERT_T     0x44
#define DS18B20_CMD_READ_SCRATCH  0xBE
#define DS18B20_CMD_READ_ROM      0x33

// Timing constants (in microseconds)
#define DS18B20_RESET_PULSE_US     480
#define DS18B20_RESET_WAIT_US      480
#define DS18B20_SLOT_US             60
#define DS18B20_WRITE_0_US          60
#define DS18B20_WRITE_1_US           5
#define DS18B20_READ_SLOT_US        60
#define DS18B20_RECOVERY_US          5
#define DS18B20_CONVERT_TIME_MS     750  // 12-bit resolution

static const char *TAG = "DS18B20";

// ИСПРАВЛЕНО: правильная работа с open-drain выходом
static void ds18b20_set_output(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);  // Подтягиваем к 1 (высокий уровень)
}

static void ds18b20_set_input(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

// ИСПРАВЛЕНО: улучшенная функция сброса
static bool ds18b20_reset_pulse(gpio_num_t pin)
{
    // Pull line low for reset pulse
    ds18b20_set_output(pin);
    gpio_set_level(pin, 0);
    ets_delay_us(DS18B20_RESET_PULSE_US);
    
    // Release line and wait for presence pulse
    gpio_set_level(pin, 1);
    ds18b20_set_input(pin);
    ets_delay_us(DS18B20_RESET_WAIT_US);
    
    // Check if device pulled line low (presence pulse)
    int level = gpio_get_level(pin);
    bool presence = (level == 0);
    
    if (presence) {
        ESP_LOGD(TAG, "Presence pulse detected");
    } else {
        ESP_LOGD(TAG, "No presence pulse (level=%d)", level);
    }
    
    // Wait for presence pulse to end
    ets_delay_us(DS18B20_RESET_WAIT_US);
    
    return presence;
}

// ИСПРАВЛЕНО: запись бита с правильными таймингами
static void ds18b20_write_bit(gpio_num_t pin, uint8_t bit)
{
    ds18b20_set_output(pin);
    
    if (bit) {
        // Write 1: pull low for 1-15us, then release
        gpio_set_level(pin, 0);
        ets_delay_us(5);
        gpio_set_level(pin, 1);
        ets_delay_us(DS18B20_WRITE_1_US);
    } else {
        // Write 0: pull low for 60-120us
        gpio_set_level(pin, 0);
        ets_delay_us(DS18B20_WRITE_0_US);
        gpio_set_level(pin, 1);
        ets_delay_us(DS18B20_RECOVERY_US);
    }
}

// ИСПРАВЛЕНО: чтение бита с правильными таймингами
static uint8_t ds18b20_read_bit(gpio_num_t pin)
{
    uint8_t bit;
    
    ds18b20_set_output(pin);
    gpio_set_level(pin, 0);
    ets_delay_us(2);
    gpio_set_level(pin, 1);
    ds18b20_set_input(pin);
    ets_delay_us(5);
    
    bit = gpio_get_level(pin);
    ets_delay_us(DS18B20_READ_SLOT_US - 7);
    
    return bit;
}

// ИСПРАВЛЕНО: запись байта
static void ds18b20_write_byte(gpio_num_t pin, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ds18b20_write_bit(pin, (byte >> i) & 0x01);
    }
}

// ИСПРАВЛЕНО: чтение байта
static uint8_t ds18b20_read_byte(gpio_num_t pin)
{
    uint8_t byte = 0;
    
    for (int i = 0; i < 8; i++) {
        if (ds18b20_read_bit(pin)) {
            byte |= (1 << i);
        }
    }
    
    return byte;
}

// ИСПРАВЛЕНО: функция инициализации
esp_err_t ds18b20_init(ds18b20_dev_t *dev, gpio_num_t data_pin)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->data_pin = data_pin;
    dev->initialized = false;
    memset(dev->rom_code, 0, sizeof(dev->rom_code));
    
    // Configure GPIO
    gpio_reset_pin(data_pin);
    ds18b20_set_output(data_pin);
    gpio_set_level(data_pin, 1);
    
    // Небольшая задержка для стабилизации
    vTaskDelay(pdMS_TO_TICKS(10));
    
    // ИСПРАВЛЕНО: более надежная проверка присутствия
    for (int attempt = 0; attempt < 3; attempt++) {
        if (ds18b20_reset_pulse(data_pin)) {
            ESP_LOGI(TAG, "DS18B20 detected on pin %d (attempt %d)", data_pin, attempt + 1);
            
            // Try to read ROM
            ds18b20_write_byte(data_pin, DS18B20_CMD_READ_ROM);
            for (int i = 0; i < 8; i++) {
                dev->rom_code[i] = ds18b20_read_byte(data_pin);
            }
            
            // Check family code (0x28 for DS18B20)
            if (dev->rom_code[0] == 0x28) {
                ESP_LOGI(TAG, "DS18B20 ROM: %02x%02x%02x%02x%02x%02x%02x%02x",
                         dev->rom_code[0], dev->rom_code[1], dev->rom_code[2], dev->rom_code[3],
                         dev->rom_code[4], dev->rom_code[5], dev->rom_code[6], dev->rom_code[7]);
                dev->initialized = true;
                return ESP_OK;
            } else {
                ESP_LOGW(TAG, "Unexpected family code: 0x%02x", dev->rom_code[0]);
            }
        }
        
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    ESP_LOGE(TAG, "No DS18B20 device found on pin %d after 3 attempts", data_pin);
    return ESP_ERR_NOT_FOUND;
}

// ИСПРАВЛЕНО: чтение температуры
esp_err_t ds18b20_read_temperature(ds18b20_dev_t *dev, int16_t *temperature)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // ИСПРАВЛЕНО: проверка присутствия перед каждым чтением
    if (!ds18b20_reset_pulse(dev->data_pin)) {
        ESP_LOGE(TAG, "Device not present");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Skip ROM (single device)
    ds18b20_write_byte(dev->data_pin, DS18B20_CMD_SKIP_ROM);
    
    // Start temperature conversion
    ds18b20_write_byte(dev->data_pin, DS18B20_CMD_CONVERT_T);
    
    // Wait for conversion to complete
    vTaskDelay(pdMS_TO_TICKS(DS18B20_CONVERT_TIME_MS));
    
    // Reset again
    if (!ds18b20_reset_pulse(dev->data_pin)) {
        ESP_LOGE(TAG, "Device not present after conversion");
        return ESP_ERR_NOT_FOUND;
    }
    
    // Skip ROM
    ds18b20_write_byte(dev->data_pin, DS18B20_CMD_SKIP_ROM);
    
    // Read scratchpad
    ds18b20_write_byte(dev->data_pin, DS18B20_CMD_READ_SCRATCH);
    
    // Read temperature LSB and MSB
    uint8_t temp_lsb = ds18b20_read_byte(dev->data_pin);
    uint8_t temp_msb = ds18b20_read_byte(dev->data_pin);
    
    // Read remaining scratchpad bytes (ignore)
    for (int i = 0; i < 7; i++) {
        ds18b20_read_byte(dev->data_pin);
    }
    
    // Combine into 16-bit signed value
    int16_t raw_temp = (int16_t)((temp_msb << 8) | temp_lsb);
    
    // Convert to 0.01°C (raw_temp * 0.0625 * 100 = raw_temp * 6.25)
    int16_t temp_celsius_x100 = (raw_temp * 625) / 100;
    
    if (temperature) {
        *temperature = temp_celsius_x100;
    }
    
    dev->last_temperature = temp_celsius_x100;
    dev->last_read_time = esp_timer_get_time();
    
    ESP_LOGD(TAG, "Temperature: %.2f°C (raw: 0x%04x)", temp_celsius_x100 / 100.0f, raw_temp);
    
    return ESP_OK;
}

// ИСПРАВЛЕНО: сброс датчика
esp_err_t ds18b20_reset(ds18b20_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!ds18b20_reset_pulse(dev->data_pin)) {
        ESP_LOGE(TAG, "Reset failed - device not responding");
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "DS18B20 reset completed");
    return ESP_OK;
}