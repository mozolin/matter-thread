#include "rcwl_0516.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

static const char *TAG = "RCWL_0516";

static void IRAM_ATTR rcwl_0516_isr_handler_static(void *arg) {
    rcwl_0516_isr_handler(arg);
}

void rcwl_0516_init(rcwl_0516_t *sensor, gpio_num_t pin) {
    sensor->gpio_pin = pin;
    sensor->motion_detected = false;
    sensor->debounce_ms = 1000; // 1 секунда дебаунс
    sensor->last_detection_time = 0;
    sensor->motion_callback = NULL;
    
    // Настройка GPIO как вход
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLDOWN_ONLY,
        .pull_down_en = GPIO_PULLDOWN_ENABLE,
        .intr_type = GPIO_INTR_ANYEDGE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    
    // Установка обработчика прерываний
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, rcwl_0516_isr_handler_static, (void*)sensor);
    
    ESP_LOGI(TAG, "RCWL-0516 initialized on GPIO %d", pin);
}

bool rcwl_0516_read(rcwl_0516_t *sensor) {
    return gpio_get_level(sensor->gpio_pin);
}

void rcwl_0516_set_callback(rcwl_0516_t *sensor, void (*callback)(bool motion)) {
    sensor->motion_callback = callback;
}

void rcwl_0516_isr_handler(void *arg) {
    rcwl_0516_t *sensor = (rcwl_0516_t *)arg;
    uint64_t current_time = esp_timer_get_time() / 1000; // мс
    
    if ((current_time - sensor->last_detection_time) > sensor->debounce_ms) {
        bool motion = gpio_get_level(sensor->gpio_pin);
        sensor->motion_detected = motion;
        sensor->last_detection_time = current_time;
        
        if (sensor->motion_callback) {
            sensor->motion_callback(motion);
        }
        
        ESP_LOGI(TAG, "Microwave motion %s detected", motion ? "detected" : "cleared");
    }
}