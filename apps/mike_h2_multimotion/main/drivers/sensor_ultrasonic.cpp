#include "sensor_ultrasonic.h"
#include <esp_timer.h>
#include <stdlib.h>

static const char* TAG_ULTRASONIC = "ULTRASONIC_SENSOR";

// Медианный фильтр для HC-SR04
#define MEDIAN_FILTER_SIZE 5
static uint32_t distance_filter_buffer[MEDIAN_FILTER_SIZE];
static uint8_t filter_index = 0;

#define HC_SR04_SPEED_OF_SOUND_CM_PER_US 0.0343f
#define HC_SR04_MIN_DISTANCE_CM 2
#define HC_SR04_MAX_DISTANCE_CM 400

static uint32_t median_filter(uint32_t new_value) {
    distance_filter_buffer[filter_index] = new_value;
    filter_index = (filter_index + 1) % MEDIAN_FILTER_SIZE;
    
    uint32_t sorted[MEDIAN_FILTER_SIZE];
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        sorted[i] = distance_filter_buffer[i];
    }
    
    for (int i = 0; i < MEDIAN_FILTER_SIZE - 1; i++) {
        for (int j = 0; j < MEDIAN_FILTER_SIZE - i - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                uint32_t temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    return sorted[MEDIAN_FILTER_SIZE / 2];
}

static uint32_t hcsr04_calculate_distance(uint64_t pulse_duration_us)
{
    if (pulse_duration_us == 0) {
        return 0;
    }
    
    float distance_cm = pulse_duration_us * HC_SR04_SPEED_OF_SOUND_CM_PER_US / 2.0f;
    
    if (distance_cm < HC_SR04_MIN_DISTANCE_CM) {
        return 0;
    }
    if (distance_cm > HC_SR04_MAX_DISTANCE_CM) {
        return HC_SR04_MAX_DISTANCE_CM;
    }
    
    return (uint32_t)(distance_cm + 0.5f);
}

esp_err_t hcsr04_init(hcsr04_dev_t *dev, gpio_num_t trigger_pin, gpio_num_t echo_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_ULTRASONIC, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->trigger_pin = trigger_pin;
    dev->echo_pin = echo_pin;
    dev->last_distance_cm = 0;
    dev->last_measurement_time = 0;

    // Configure trigger pin as output
    gpio_reset_pin(trigger_pin);
    esp_err_t err = gpio_set_direction(trigger_pin, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ULTRASONIC, "Failed to set GPIO direction for HC-SR04 trigger");
        return err;
    }
    gpio_set_level(trigger_pin, 0);

    // Configure echo pin as input
    gpio_reset_pin(echo_pin);
    err = gpio_set_direction(echo_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ULTRASONIC, "Failed to set GPIO direction for HC-SR04 echo");
        return err;
    }

    // Initialize filter
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        distance_filter_buffer[i] = 100;
    }
    filter_index = 0;

    ESP_LOGI(TAG_ULTRASONIC, "HC-SR04 initialized: trigger=%d, echo=%d", trigger_pin, echo_pin);

    return ESP_OK;
}

uint32_t hcsr04_measure_distance(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_ULTRASONIC, "Device is NULL");
        return 0;
    }

    const uint8_t MAX_ATTEMPTS = 3;
    const uint64_t MEASUREMENT_TIMEOUT_US = 30000;
    
    uint64_t measurements[MAX_ATTEMPTS];
    uint8_t valid_measurements = 0;
    
    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        gpio_set_level(dev->trigger_pin, 0);
        esp_rom_delay_us(2);
        
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        
        uint64_t start_wait = esp_timer_get_time();
        while (gpio_get_level(dev->echo_pin) == 0) {
            if ((esp_timer_get_time() - start_wait) > 10000) {
                break;
            }
        }
        
        if (gpio_get_level(dev->echo_pin) == 0) {
            continue;
        }
        
        uint64_t pulse_start = esp_timer_get_time();
        uint64_t timeout_start = pulse_start;
        
        while (gpio_get_level(dev->echo_pin) == 1) {
            if ((esp_timer_get_time() - timeout_start) > MEASUREMENT_TIMEOUT_US) {
                break;
            }
        }
        
        uint64_t pulse_end = esp_timer_get_time();
        uint64_t pulse_duration = pulse_end - pulse_start;
        
        if (pulse_duration >= 58 && pulse_duration <= 23200) {
            measurements[valid_measurements] = pulse_duration;
            valid_measurements++;
        }
        
        if (attempt < MAX_ATTEMPTS - 1) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    
    if (valid_measurements == 0) {
        ESP_LOGW(TAG_ULTRASONIC, "No valid measurements");
        return (dev->last_distance_cm > 0) ? dev->last_distance_cm : 0;
    }
    
    uint64_t median_pulse_duration = 0;
    if (valid_measurements == 1) {
        median_pulse_duration = measurements[0];
    } else {
        for (int i = 0; i < valid_measurements - 1; i++) {
            for (int j = i + 1; j < valid_measurements; j++) {
                if (measurements[i] > measurements[j]) {
                    uint64_t temp = measurements[i];
                    measurements[i] = measurements[j];
                    measurements[j] = temp;
                }
            }
        }
        median_pulse_duration = measurements[valid_measurements / 2];
    }
    
    uint32_t raw_distance = hcsr04_calculate_distance(median_pulse_duration);
    uint32_t filtered_distance = median_filter(raw_distance);
    
    dev->last_distance_cm = filtered_distance;
    dev->last_measurement_time = esp_timer_get_time();
    
    return filtered_distance;
}

esp_err_t hcsr04_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGW(TAG_ULTRASONIC, "Performing hardware reset");
    
    gpio_set_level(dev->trigger_pin, 0);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->echo_pin, 0);
    esp_rom_delay_us(100);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    for (int i = 0; i < 3; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        esp_rom_delay_us(1000);
    }
    
    ESP_LOGI(TAG_ULTRASONIC, "Reset complete");
    return ESP_OK;
}

esp_err_t hcsr04_hard_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGW(TAG_ULTRASONIC, "Performing hard reset");
    
    gpio_set_level(dev->trigger_pin, 0);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->echo_pin, 0);
    
    vTaskDelay(pdMS_TO_TICKS(50));
    
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);
    
    vTaskDelay(pdMS_TO_TICKS(100));
    
    for (int i = 0; i < 5; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    ESP_LOGI(TAG_ULTRASONIC, "Hard reset complete");
    return ESP_OK;
}

esp_err_t hcsr04_soft_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG_ULTRASONIC, "Performing soft reset");
    
    for (int i = 0; i < 3; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    return ESP_OK;
}
