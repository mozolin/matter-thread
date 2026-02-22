#include "sensor_driver_dht11.h"
#include "app_priv.h"
#include <esp_log.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <rom/ets_sys.h>

// DHT11 Timing Constants (in microseconds)
#define DHT11_START_SIGNAL_US     18000
#define DHT11_START_SIGNAL_HOLD   30
#define DHT11_RESPONSE_TIMEOUT    100
#define DHT11_BIT_TIMEOUT         100
#define DHT11_MIN_READ_INTERVAL_MS 2000  // DHT11 needs at least 1 second between reads

static const char *TAG = "DHT11";

static void dht11_set_output(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_OUTPUT_OD);
}

static void dht11_set_input(gpio_num_t pin)
{
    gpio_set_direction(pin, GPIO_MODE_INPUT);
}

static bool dht11_wait_for_level(gpio_num_t pin, int level, uint32_t timeout_us)
{
    uint64_t start_time = esp_timer_get_time();
    
    while (gpio_get_level(pin) != level) {
        if ((esp_timer_get_time() - start_time) > timeout_us) {
            return false;
        }
    }
    
    return true;
}

esp_err_t dht11_init(dht11_dev_t *dev, gpio_num_t data_pin)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    dev->data_pin = data_pin;
    dev->initialized = false;
    
    // Configure GPIO
    gpio_reset_pin(data_pin);
    dht11_set_output(data_pin);
    gpio_set_level(data_pin, 1);  // Pull high by default
    
    dev->initialized = true;
    ESP_LOGI(TAG, "DHT11 initialized on pin %d", data_pin);
    
    return ESP_OK;
}

esp_err_t dht11_read(dht11_dev_t *dev, int16_t *temperature, uint16_t *humidity)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    uint8_t data[5] = {0};
    
    // Check minimum read interval
    uint64_t now = esp_timer_get_time();
    if (dev->last_read_time > 0 && (now - dev->last_read_time) < (DHT11_MIN_READ_INTERVAL_MS * 1000)) {
        // Return cached values if too soon
        if (temperature) *temperature = dev->last_temperature;
        if (humidity) *humidity = dev->last_humidity;
        return ESP_OK;
    }
    
    // Start signal: pull low for >18ms, then pull high for 20-40us
    dht11_set_output(dev->data_pin);
    gpio_set_level(dev->data_pin, 0);
    ets_delay_us(DHT11_START_SIGNAL_US);
    gpio_set_level(dev->data_pin, 1);
    ets_delay_us(DHT11_START_SIGNAL_HOLD);
    dht11_set_input(dev->data_pin);
    
    // Wait for sensor response (low)
    if (!dht11_wait_for_level(dev->data_pin, 0, DHT11_RESPONSE_TIMEOUT)) {
        ESP_LOGW(TAG, "No response from DHT11 (low)");
        return ESP_ERR_TIMEOUT;
    }
    
    // Wait for response high
    if (!dht11_wait_for_level(dev->data_pin, 1, DHT11_RESPONSE_TIMEOUT)) {
        ESP_LOGW(TAG, "No response from DHT11 (high)");
        return ESP_ERR_TIMEOUT;
    }
    
    // Wait for data start
    if (!dht11_wait_for_level(dev->data_pin, 0, DHT11_RESPONSE_TIMEOUT)) {
        ESP_LOGW(TAG, "No data start from DHT11");
        return ESP_ERR_TIMEOUT;
    }
    
    // Read 40 bits (5 bytes)
    for (int i = 0; i < 40; i++) {
        // Wait for bit start (low)
        if (!dht11_wait_for_level(dev->data_pin, 1, DHT11_BIT_TIMEOUT)) {
            ESP_LOGW(TAG, "Timeout waiting for bit %d high", i);
            return ESP_ERR_TIMEOUT;
        }
        
        uint64_t high_start = esp_timer_get_time();
        
        // Wait for bit end (low)
        if (!dht11_wait_for_level(dev->data_pin, 0, DHT11_BIT_TIMEOUT)) {
            ESP_LOGW(TAG, "Timeout waiting for bit %d low", i);
            return ESP_ERR_TIMEOUT;
        }
        
        uint64_t high_end = esp_timer_get_time();
        uint32_t high_duration = high_end - high_start;
        
        // If high duration > 30us, it's a 1, otherwise 0
        if (high_duration > 30) {
            data[i / 8] |= (1 << (7 - (i % 8)));
        }
    }
    
    // Verify checksum
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        ESP_LOGW(TAG, "Checksum error: %02x != %02x", checksum, data[4]);
        return ESP_ERR_INVALID_CRC;
    }
    
    // DHT11 provides integer values
    // Humidity: data[0] = integer part (%), data[1] = decimal part (always 0 for DHT11)
    // Temperature: data[2] = integer part (°C), data[3] = decimal part (always 0 for DHT11)
    
    uint16_t hum = data[0] * 100;  // Convert to 0.01% (e.g., 45% -> 4500)
    int16_t temp = data[2] * 100;  // Convert to 0.01°C (e.g., 25°C -> 2500)
    
    // Check for negative temperature (MSB of data[3] indicates sign)
    if (data[3] & 0x80) {
        temp = -temp;
    }
    
    dev->last_humidity = hum;
    dev->last_temperature = temp;
    dev->last_read_time = esp_timer_get_time();
    
    if (temperature) {
        *temperature = temp;
    }
    
    if (humidity) {
        *humidity = hum;
    }
    
    ESP_LOGD(TAG, "DHT11 - Temp: %.1f°C, Hum: %d%%", temp / 100.0f, data[0]);
    
    return ESP_OK;
}

esp_err_t dht11_read_temperature(dht11_dev_t *dev, int16_t *temperature)
{
    return dht11_read(dev, temperature, NULL);
}

esp_err_t dht11_read_humidity(dht11_dev_t *dev, uint16_t *humidity)
{
    return dht11_read(dev, NULL, humidity);
}

esp_err_t dht11_reset(dht11_dev_t *dev)
{
    if (!dev || !dev->initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    dht11_set_output(dev->data_pin);
    gpio_set_level(dev->data_pin, 1);
    vTaskDelay(pdMS_TO_TICKS(10));
    
    ESP_LOGI(TAG, "DHT11 reset completed");
    return ESP_OK;
}
