/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

//#include <stdlib.h>
#include "app_priv.h"
#include "sensor_driver.h"
#include <driver/gpio.h>
#include <esp_timer.h>
#include <esp_err.h>
#include <esp_log.h>

//static const char *TAG = "SensorDriver";

uint32_t base_distance_cm = 0;

// HC-SR501 PIR Sensor Implementation
esp_err_t hcsr501_init(hcsr501_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = output_pin;
    dev->last_state = false;
    dev->last_detection_time = 0;

    // Configure GPIO as input
    gpio_reset_pin(output_pin);
    esp_err_t err = gpio_set_direction(output_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set GPIO direction for HC-SR501");
        return err;
    }

    // Enable pull-down resistor (HC-SR501 output is active HIGH)
    err = gpio_set_pull_mode(output_pin, GPIO_PULLDOWN_ONLY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_SENSOR, "Failed to set pull mode for HC-SR501, continuing anyway");
    }

    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501 initialized on GPIO %d", output_pin);
    return ESP_OK;
}

bool hcsr501_read(hcsr501_dev_t *dev)
{
    if (dev == NULL) {
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501: is NULL");
        return false;
    }

    int level = gpio_get_level(dev->output_pin);
    bool current_state = (level == 1);
    
    if (current_state != dev->last_state) {
        dev->last_state = current_state;
        if (current_state) {
            dev->last_detection_time = esp_timer_get_time();
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501: Motion detected");
        } else {
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR501: Motion ended");
        }
    }
    
    return current_state;
}

// RCWL-0516 Microwave Sensor Implementation
esp_err_t rcwl0516_init(rcwl0516_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = output_pin;
    dev->last_state = false;
    dev->last_detection_time = 0;

    // Configure GPIO as input
    gpio_reset_pin(output_pin);
    esp_err_t err = gpio_set_direction(output_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set GPIO direction for RCWL-0516");
        return err;
    }

    // RCWL-0516 output is active LOW, enable pull-up
    err = gpio_set_pull_mode(output_pin, GPIO_PULLUP_ONLY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_SENSOR, "Failed to set pull mode for RCWL-0516, continuing anyway");
    }

    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ RCWL-0516 initialized on GPIO %d", output_pin);
    return ESP_OK;
}

bool rcwl0516_read(rcwl0516_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    int level = gpio_get_level(dev->output_pin);
    // RCWL-0516 output is active LOW, so invert the reading
    bool current_state = (level == 1);
    
    if (current_state != dev->last_state) {
        dev->last_state = current_state;
        if (current_state) {
            dev->last_detection_time = esp_timer_get_time();
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ RCWL-0516: Motion detected");
        } else {
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ RCWL-0516: Motion ended");
        }
    }
    
    return current_state;
}

// HC-SR04 Ultrasonic Sensor Implementation
esp_err_t hcsr04_init(hcsr04_dev_t *dev, gpio_num_t trigger_pin, gpio_num_t echo_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Device pointer is NULL");
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
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set GPIO direction for HC-SR04 trigger");
        return err;
    }
    gpio_set_level(trigger_pin, 0);

    // Configure echo pin as input
    gpio_reset_pin(echo_pin);
    err = gpio_set_direction(echo_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set GPIO direction for HC-SR04 echo");
        return err;
    }

    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04 initialized: trigger=%d, echo=%d", trigger_pin, echo_pin);
    return ESP_OK;
}

uint32_t hcsr04_measure_distance(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return 0;
    }

    // Send 10us pulse on trigger pin
    gpio_set_level(dev->trigger_pin, 0);
    //ets_delay_us(2);
    vTaskDelay(pdMS_TO_TICKS(2));
    gpio_set_level(dev->trigger_pin, 1);
    //ets_delay_us(10);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(dev->trigger_pin, 0);

    // Wait for echo pin to go high
    uint32_t start_time = esp_timer_get_time();
    while (gpio_get_level(dev->echo_pin) == 0) {
        if ((esp_timer_get_time() - start_time) > 10000) { // 10ms timeout
            ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Echo pin timeout (no response)");
            return 0;
        }
    }

    // Measure pulse width (echo high duration)
    start_time = esp_timer_get_time();
    while (gpio_get_level(dev->echo_pin) == 1) {
        if ((esp_timer_get_time() - start_time) > 60000) { // 60ms timeout (~10m distance)
            ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Echo pulse too long");
            return 0;
        }
    }
    uint32_t end_time = esp_timer_get_time();

    // Calculate distance in cm
    // Speed of sound = 340 m/s = 0.034 cm/μs
    // Distance = (time * 0.034) / 2 (round trip)
    uint32_t pulse_duration = end_time - start_time;
    uint32_t distance_cm = (pulse_duration * 0.034) / 2;

    // Filter out unreasonable values (max 400cm for HC-SR04)
    if (distance_cm > 400) {
        distance_cm = 400;
    }

    dev->last_distance_cm = distance_cm;
    dev->last_measurement_time = esp_timer_get_time();
    
    if(abs((int)distance_cm - (int)base_distance_cm) >= 5) {
    	ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04: Distance = %lu cm", distance_cm);
    	base_distance_cm = distance_cm;
    }
    
    return distance_cm;
}

// KY-038 Sound Sensor Implementation

esp_err_t ky038_init(ky038_dev_t *dev, gpio_num_t digital_pin, gpio_num_t analog_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = digital_pin;
    dev->analog_pin = analog_pin;
    dev->last_state = false;
    dev->last_sound_level = 0;
    dev->last_detection_time = 0;
    dev->sound_threshold = 2048; // Среднее значение по умолчанию (50% от 4095)

    // Configure digital output pin as input
    gpio_reset_pin(digital_pin);
    esp_err_t err = gpio_set_direction(digital_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "~~~ Failed to set GPIO direction for KY-038 digital pin");
        return err;
    }

    // Enable pull-down resistor для цифрового выхода
    err = gpio_set_pull_mode(digital_pin, GPIO_PULLDOWN_ONLY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Failed to set pull mode for KY-038 digital pin, continuing anyway");
    }

    // Если используется аналоговый пин, настраиваем ADC
    if (analog_pin != GPIO_NUM_NC) {
        // Настройка ADC (требует дополнительной конфигурации в зависимости от платы)
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038 analog pin configured on GPIO %d", analog_pin);
        // Для ESP32: потребуется настройка ADC через esp_adc_cal_characterize()
    }

    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038 Sound Sensor initialized: digital=%d, analog=%d", 
             digital_pin, analog_pin);
    return ESP_OK;
}

bool ky038_read_digital(ky038_dev_t *dev)
{
    if (dev == NULL) {
        return false;
    }

    // KY-038 имеет цифровой выход, который становится HIGH при обнаружении звука
    int level = gpio_get_level(dev->output_pin);
    bool current_state = (level == 1);
    
    if (current_state != dev->last_state) {
        dev->last_state = current_state;
        if (current_state) {
            dev->last_detection_time = esp_timer_get_time();
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038: Sound detected (digital)");
        } else {
            ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038: Sound ended");
        }
    }
    
    return current_state;
}

uint32_t ky038_read_analog(ky038_dev_t *dev)
{
    if (dev == NULL || dev->analog_pin == GPIO_NUM_NC) {
        return 0;
    }

    // Чтение аналогового значения с ADC
    // Для ESP32 используем API ADC
    uint32_t adc_reading = 0;
    
    // Пример для ESP32:
    // int raw = adc1_get_raw(ADC1_CHANNEL_X); // X зависит от GPIO
    // adc_reading = esp_adc_cal_raw_to_voltage(raw, &adc_chars);
    
    // Заглушка - возвращаем тестовое значение
    // В реальной реализации здесь будет чтение ADC
    adc_reading = 2000 + (esp_timer_get_time() % 1000); // Тестовые данные
    
    dev->last_sound_level = adc_reading;
    
    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038: Analog sound level = %lu", adc_reading);
    return adc_reading;
}

void ky038_set_threshold(ky038_dev_t *dev, uint32_t threshold)
{
    if (dev != NULL) {
        dev->sound_threshold = threshold;
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ KY-038: Sound threshold set to %lu", threshold);
    }
}
