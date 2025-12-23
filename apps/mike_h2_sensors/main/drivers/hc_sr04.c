#include "hc_sr04.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/gpio.h"
#include "esp_timer.h"

static const char *TAG = "HC_SR04";

static volatile uint64_t echo_start_time = 0;
static volatile uint64_t echo_end_time = 0;
static volatile bool echo_received = false;

static void IRAM_ATTR hc_sr04_echo_isr_handler_static(void *arg) {
    hc_sr04_echo_isr_handler(arg);
}

void hc_sr04_init(hc_sr04_t *sensor, gpio_num_t trig_pin, gpio_num_t echo_pin) {
    sensor->trig_pin = trig_pin;
    sensor->echo_pin = echo_pin;
    sensor->timeout_us = 30000; // 30 мкс таймаут
    sensor->distance_cm = 0.0;
    sensor->distance_callback = NULL;
    
    // Настройка TRIG как выход
    gpio_config_t trig_conf = {
        .pin_bit_mask = (1ULL << trig_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&trig_conf));
    
    // Настройка ECHO как вход с прерыванием
    gpio_config_t echo_conf = {
        .pin_bit_mask = (1ULL << echo_pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&echo_conf));
    
    // Установка обработчика прерываний для ECHO
    gpio_install_isr_service(0);
    gpio_isr_handler_add(echo_pin, hc_sr04_echo_isr_handler_static, (void*)sensor);
    
    ESP_LOGI(TAG, "HC-SR04 initialized: TRIG=%d, ECHO=%d", trig_pin, echo_pin);
}

float hc_sr04_measure(hc_sr04_t *sensor) {
    // Генерация импульса 10 мкс на TRIG
    gpio_set_level(sensor->trig_pin, 0);
    esp_rom_delay_us(2);
    gpio_set_level(sensor->trig_pin, 1);
    esp_rom_delay_us(10);
    gpio_set_level(sensor->trig_pin, 0);
    
    // Ожидание ответа
    echo_received = false;
    uint64_t start_wait = esp_timer_get_time();
    
    while (!echo_received && (esp_timer_get_time() - start_wait) < sensor->timeout_us) {
        vTaskDelay(1 / portTICK_PERIOD_MS);
    }
    
    if (echo_received && echo_end_time > echo_start_time) {
        uint64_t pulse_duration = echo_end_time - echo_start_time;
        // Расстояние = (время * скорость звука) / 2
        // Скорость звука ~ 340 м/с = 0.034 см/мкс
        sensor->distance_cm = (pulse_duration * 0.034) / 2.0;
        
        if (sensor->distance_callback) {
            sensor->distance_callback(sensor->distance_cm);
        }
        
        ESP_LOGI(TAG, "Distance measured: %.2f cm", sensor->distance_cm);
    } else {
        sensor->distance_cm = -1.0;
        ESP_LOGW(TAG, "Measurement timeout");
    }
    
    return sensor->distance_cm;
}

void hc_sr04_set_callback(hc_sr04_t *sensor, void (*callback)(float distance_cm)) {
    sensor->distance_callback = callback;
}

void hc_sr04_echo_isr_handler(void *arg) {
    hc_sr04_t *sensor = (hc_sr04_t *)arg;
    uint32_t gpio_level = gpio_get_level(sensor->echo_pin);
    
    if (gpio_level == 1) {
        // Начало импульса
        echo_start_time = esp_timer_get_time();
    } else {
        // Конец импульса
        echo_end_time = esp_timer_get_time();
        echo_received = true;
    }
}