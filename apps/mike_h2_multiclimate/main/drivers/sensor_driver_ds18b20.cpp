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
#define DS18B20_CMD_SEARCH_ROM    0xF0
#define DS18B20_CMD_MATCH_ROM     0x55
#define DS18B20_CMD_READ_ROM      0x33

// Timing constants (in microseconds)
#define DS18B20_RESET_PULSE_US    480
#define DS18B20_RESET_WAIT_US     480
#define DS18B20_SLOT_US           60
#define DS18B20_WRITE_0_US        60
#define DS18B20_WRITE_1_US        5
#define DS18B20_READ_SLOT_US      60
#define DS18B20_RECOVERY_US       5
#define DS18B20_CONVERT_TIME_MS   750  // 12-bit resolution

static const char *TAG = "DS18B20";

static void ds18b20_set_output(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
    gpio_set_level(pin, 1);
}

static void ds18b20_set_input(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

static bool ds18b20_wait_for_presence(gpio_num_t pin)
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
    bool presence = (gpio_get_level(pin) == 0);
    
    // Wait for presence pulse to end
    ets_delay_us(DS18B20_RESET_WAIT_US);
    
    return presence;
}

static bool ds18b20_reset(gpio_num_t pin)
{
    return ds18b20_wait_for_presence(pin);
}

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

static void ds18b20_write_byte(gpio_num_t pin, uint8_t byte)
{
    for (int i = 0; i < 8; i++) {
        ds18b20_write_bit(pin, (byte >> i) & 0x01);
    }
}

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
    
    // Check for device presence
    if (!ds18b20_reset(data_pin)) {
        ESP_LOGE(TAG, "No DS18B20 device found on pin %d", data_pin);
        return ESP_ERR_NOT_FOUND;
    }
    
    // Read ROM code (if multiple devices, this will fail)
    ds18b20_write_byte(data_pin, DS18B20_CMD_READ_ROM);
    for (int i = 0; i < 8; i++) {
        dev->rom_code[i] = ds18b20_read_byte(data_pin);
    }
    
    // Check CRC
    uint8_t crc = dev->rom_code[7];
    // Simple CRC validation would go here
    
    if (dev->rom_code[0] != 0x28) { // DS18B20 family code
        ESP_LOGW(TAG, "Unexpected family code: 0x%02x", dev->rom_code[0]);
    }
    
    dev->initialized = true;
    ESP_LOGI(TAG, "DS18B20 initialized on pin %d, ROM: %02x%02x%02x%02x%02x%02x%02x%02x",
             data_pin,
             dev->rom_code[0], dev->rom_code[1], dev->rom_code[2], dev->rom_code[3],
             dev->rom_code[4], dev->rom_code[5], dev->rom_code[6], dev->rom_code[7]);
    
    return ESP_OK;
}

esp_err_t ds18b20_read_temperature(ds18b20_dev_t *dev, int16_t *temperature)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Reset and check presence
    if (!ds18b20_reset(dev->data_pin)) {
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
    if (!ds18b20_reset(dev->data_pin)) {
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
    
    // Read remaining scratchpad bytes (ignore for now)
    for (int i = 0; i < 7; i++) {
        ds18b20_read_byte(dev->data_pin);
    }
    
    // Combine into 16-bit signed value
    int16_t raw_temp = (int16_t)((temp_msb << 8) | temp_lsb);
    
    // Convert to 0.01°C
    // For DS18B20, each LSB is 0.0625°C
    // So raw_temp * 0.0625 = temperature in °C
    // Multiply by 100 to get 0.01°C: raw_temp * 6.25
    // raw_temp * 625 / 100 = temperature in 0.01°C
    int16_t temp_celsius_x100 = (raw_temp * 625) / 100;
    
    if (temperature) {
        *temperature = temp_celsius_x100;
    }
    
    dev->last_temperature = temp_celsius_x100;
    dev->last_read_time = esp_timer_get_time();
    
    ESP_LOGD(TAG, "Temperature: %.2f°C", temp_celsius_x100 / 100.0f);
    
    return ESP_OK;
}

esp_err_t ds18b20_reset(ds18b20_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (!ds18b20_reset(dev->data_pin)) {
        ESP_LOGE(TAG, "Reset failed - device not responding");
        return ESP_ERR_NOT_FOUND;
    }
    
    ESP_LOGI(TAG, "DS18B20 reset completed");
    return ESP_OK;
}

int ds18b20_search(ds18b20_dev_t *dev, uint8_t max_devices, uint64_t *rom_codes)
{
    if (!dev || !dev->initialized || !rom_codes) {
        return 0;
    }
    
    // Simplified search for demonstration
    // Full implementation would implement the 1-Wire search algorithm
    
    if (!ds18b20_reset(dev->data_pin)) {
        return 0;
    }
    
    ds18b20_write_byte(dev->data_pin, DS18B20_CMD_SEARCH_ROM);
    
    // For now, just try to read one ROM
    uint8_t rom[8];
    for (int i = 0; i < 8; i++) {
        rom[i] = 0;
        for (int bit = 0; bit < 8; bit++) {
            uint8_t b1 = ds18b20_read_bit(dev->data_pin);
            uint8_t b2 = ds18b20_read_bit(dev->data_pin);
            
            if (b1 && b2) {
                // No devices
                return 0;
            }
            
            if (b1 != b2) {
                // All devices have same bit
                if (b1) {
                    rom[i] |= (1 << bit);
                }
                ds18b20_write_bit(dev->data_pin, b1);
            } else {
                // Conflict - would need branching logic
                // Simplified: assume bit 0
                ds18b20_write_bit(dev->data_pin, 0);
            }
        }
    }
    
    memcpy(rom_codes, rom, 8);
    return 1;
}
