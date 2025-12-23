#include "sensor_driver.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "SENSOR_DRIVER";

void sensor_gpio_init(gpio_num_t pin, gpio_mode_t mode, gpio_pull_mode_t pull) {
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << pin),
        .mode = mode,
        .pull_up_en = (pull == GPIO_PULLUP_ONLY || pull == GPIO_PULLUP_PULLDOWN),
        .pull_down_en = (pull == GPIO_PULLDOWN_ONLY || pull == GPIO_PULLUP_PULLDOWN),
        .intr_type = GPIO_INTR_DISABLE
    };
    ESP_ERROR_CHECK(gpio_config(&io_conf));
    ESP_LOGI(TAG, "GPIO %d initialized", pin);
}

void sensor_set_interrupt(gpio_num_t pin, sensor_callback_t callback, void* arg) {
    gpio_install_isr_service(0);
    gpio_isr_handler_add(pin, (gpio_isr_t)callback, arg);
    ESP_LOGI(TAG, "Interrupt handler set for GPIO %d", pin);
}