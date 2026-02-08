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

// новый код: медианный фильтр для HC-SR04
#define MEDIAN_FILTER_SIZE 5
static uint32_t distance_filter_buffer[MEDIAN_FILTER_SIZE];
static uint8_t filter_index = 0;

// новый код: функция медианного фильтра
static uint32_t median_filter(uint32_t new_value) {
    // Сохраняем новое значение в буфер
    distance_filter_buffer[filter_index] = new_value;
    filter_index = (filter_index + 1) % MEDIAN_FILTER_SIZE;
    
    // Создаем копию для сортировки
    uint32_t sorted[MEDIAN_FILTER_SIZE];
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        sorted[i] = distance_filter_buffer[i];
    }
    
    // Сортируем пузырьком (для маленького массива достаточно)
    for (int i = 0; i < MEDIAN_FILTER_SIZE - 1; i++) {
        for (int j = 0; j < MEDIAN_FILTER_SIZE - i - 1; j++) {
            if (sorted[j] > sorted[j + 1]) {
                uint32_t temp = sorted[j];
                sorted[j] = sorted[j + 1];
                sorted[j + 1] = temp;
            }
        }
    }
    
    // Возвращаем медианное значение
    return sorted[MEDIAN_FILTER_SIZE / 2];
}

uint32_t base_distance_cm = 0;
uint32_t base2_distance_cm = 0;

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

    // новый код: инициализация фильтра
    for (int i = 0; i < MEDIAN_FILTER_SIZE; i++) {
        distance_filter_buffer[i] = 100; // начальное значение 100 см
    }
    filter_index = 0;

    ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04 initialized: trigger=%d, echo=%d", trigger_pin, echo_pin);

    /*
    static bool fast_task_started = false;
    if (!fast_task_started) {
        ESP_LOGW("", "#######################################");
        ESP_LOGW("", "##                                   ##");
        ESP_LOGW("", "##   TEST TEST TEST TEST TEST TEST   ##");
        ESP_LOGW("", "##                                   ##");
        ESP_LOGW("", "#######################################");
        app_driver_start_fast_ultrasonic_task();
        fast_task_started = true;
    }
    */

    return ESP_OK;
}

esp_err_t hcsr04_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Performing hardware reset");
    
    // 1. Отключаем питание (если возможно)
    // 2. Сбрасываем пины
    gpio_set_level(dev->trigger_pin, 0);
    
    // 3. На короткое время меняем направление echo пина
    gpio_set_direction(dev->echo_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->echo_pin, 0);
    esp_rom_delay_us(100);
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);
    
    // 4. Даем время на восстановление
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 5. Отправляем тестовый импульс
    for (int i = 0; i < 3; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        esp_rom_delay_us(1000); // 1ms между импульсами
    }
    
    ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Reset complete");
    return ESP_OK;
}

#define HC_SR04_SPEED_OF_SOUND_CM_PER_US 0.0343f
#define HC_SR04_MIN_DISTANCE_CM 2
#define HC_SR04_MAX_DISTANCE_CM 400

// новый код: улучшенная функция расчета расстояния
uint32_t hcsr04_calculate_distance(uint64_t pulse_duration_us)
{
    if (pulse_duration_us == 0) {
        return 0;
    }
    
    // Учитываем температуру (опционально, базовая формула для 20°C)
    const float speed_of_sound_cm_per_us = 0.0343f; // 343 м/с = 0.0343 см/мкс
    
    // distance = (time * speed_of_sound) / 2 (round trip)
    float distance_cm = pulse_duration_us * speed_of_sound_cm_per_us / 2.0f;
    
    // Фильтрация невалидных значений
    if (distance_cm < HC_SR04_MIN_DISTANCE_CM) {
        return 0;
    }
    if (distance_cm > HC_SR04_MAX_DISTANCE_CM) {
        return HC_SR04_MAX_DISTANCE_CM;
    }
    
    return (uint32_t)(distance_cm + 0.5f); // Округление до целых
}

// новый код: улучшенная функция измерения расстояния
uint32_t hcsr04_measure_distance(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MULTI_SENSOR, "HC-SR04: Device is NULL");
        return 0;
    }

    const uint8_t MAX_ATTEMPTS = 3;
    const uint64_t MEASUREMENT_TIMEOUT_US = 30000; // 30ms максимум для измерения
    
    uint64_t measurements[MAX_ATTEMPTS];
    uint8_t valid_measurements = 0;
    
    // Делаем несколько измерений
    for (int attempt = 0; attempt < MAX_ATTEMPTS; attempt++) {
        // 1. Гарантированно сбрасываем trigger
        gpio_set_level(dev->trigger_pin, 0);
        //esp_rom_delay_us(2);
        esp_rom_delay_us(2);
        
        // 2. Отправляем импульс 10us
        gpio_set_level(dev->trigger_pin, 1);
        //esp_rom_delay_us(10);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        
        // 3. Ждем начала echo с таймаутом
        uint64_t start_wait = esp_timer_get_time();
        while (gpio_get_level(dev->echo_pin) == 0) {
            if ((esp_timer_get_time() - start_wait) > 10000) { // 10ms таймаут
                ESP_LOGD(TAG_MULTI_SENSOR, "HC-SR04: Echo start timeout");
                break;
            }
        }
        
        // Если echo не начался, пропускаем эту попытку
        if (gpio_get_level(dev->echo_pin) == 0) {
            //vTaskDelay(pdMS_TO_TICKS(20));
            esp_rom_delay_us(1);
            continue;
        }
        
        // 4. Измеряем длительность импульса
        uint64_t pulse_start = esp_timer_get_time();
        uint64_t timeout_start = pulse_start;
        
        while (gpio_get_level(dev->echo_pin) == 1) {
            if ((esp_timer_get_time() - timeout_start) > MEASUREMENT_TIMEOUT_US) {
                ESP_LOGD(TAG_MULTI_SENSOR, "HC-SR04: Echo pulse timeout");
                break;
            }
        }
        
        uint64_t pulse_end = esp_timer_get_time();
        uint64_t pulse_duration = pulse_end - pulse_start;
        
        // 5. Проверяем валидность измерения
        // Минимальное время для 2 см: ~58 мкс, максимальное для 400 см: ~23200 мкс
        if (pulse_duration >= 58 && pulse_duration <= 23200) {
            measurements[valid_measurements] = pulse_duration;
            valid_measurements++;
        }
        
        // Задержка между попытками
        if (attempt < MAX_ATTEMPTS - 1) {
            vTaskDelay(pdMS_TO_TICKS(50));
        }
    }
    
    // Если нет валидных измерений
    if (valid_measurements == 0) {
        ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: No valid measurements");
        return (dev->last_distance_cm > 0) ? dev->last_distance_cm : 0;
    }
    
    // Выбираем медианное измерение из валидных
    uint64_t median_pulse_duration = 0;
    if (valid_measurements == 1) {
        median_pulse_duration = measurements[0];
    } else {
        // Сортируем для нахождения медианы
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
    
    // Вычисляем расстояние
    uint32_t raw_distance = hcsr04_calculate_distance(median_pulse_duration);
    
    // Применяем медианный фильтр
    uint32_t filtered_distance = median_filter(raw_distance);
    
    // Проверка на резкие скачки (не более 50 см за измерение)
    if (dev->last_distance_cm > 0) {
        int32_t diff = abs((int32_t)filtered_distance - (int32_t)dev->last_distance_cm);
        if (diff > 50) { // Слишком большой скачок
            ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Suspicious jump %lu cm -> %lu cm (diff: %lu)", 
                    dev->last_distance_cm, filtered_distance, diff);
            // Возвращаем старое значение или используем фильтрованное с осторожностью
            //filtered_distance = dev->last_distance_cm;
        }
    }
    
    dev->last_distance_cm = filtered_distance;
    dev->last_measurement_time = esp_timer_get_time();
    
    // Логируем при значительных изменениях (>5 см)
    if (abs((int32_t)filtered_distance - (int32_t)base_distance_cm) >= 5) {
        //ESP_LOGW(TAG_MULTI_SENSOR, "~~~ HC-SR04: Distance = %lu cm (filtered)", filtered_distance);
        base_distance_cm = filtered_distance;
    }
    
    return filtered_distance;
}

esp_err_t hcsr04_hard_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGW(TAG_MULTI_SENSOR, "HC-SR04: Performing hard reset");
    
    // 1. Полностью отключаем датчик (если управление питанием доступно)
    // 2. Сбрасываем все пины
    gpio_set_level(dev->trigger_pin, 0);
    
    // 3. На время делаем echo пином выхода и устанавливаем LOW
    gpio_set_direction(dev->echo_pin, GPIO_MODE_OUTPUT);
    gpio_set_level(dev->echo_pin, 0);
    
    // 4. Даем время на полный разряд (50мс)
    vTaskDelay(pdMS_TO_TICKS(50));
    
    // 5. Восстанавливаем конфигурацию
    gpio_set_direction(dev->echo_pin, GPIO_MODE_INPUT);
    
    // 6. Даем время на инициализацию
    vTaskDelay(pdMS_TO_TICKS(100));
    
    // 7. Отправляем серию тестовых импульсов для "пробуждения"
    for (int i = 0; i < 5; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(20));
    }
    
    ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Hard reset complete");
    return ESP_OK;
}

esp_err_t hcsr04_soft_reset(hcsr04_dev_t *dev)
{
    if (dev == NULL) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG_MULTI_SENSOR, "HC-SR04: Performing soft reset");
    
    // Просто отправляем серию импульсов
    for (int i = 0; i < 3; i++) {
        gpio_set_level(dev->trigger_pin, 1);
        esp_rom_delay_us(10);
        gpio_set_level(dev->trigger_pin, 0);
        vTaskDelay(pdMS_TO_TICKS(50));
    }
    
    return ESP_OK;
}
