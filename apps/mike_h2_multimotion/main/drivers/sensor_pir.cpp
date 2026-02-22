#include "sensor_pir.h"
#include <esp_timer.h>

static const char* TAG_PIR = "PIR_SENSOR";

esp_err_t hcsr501_init(hcsr501_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_PIR, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = output_pin;
    dev->last_state = false;
    dev->last_detection_time = 0;

    // Configure GPIO as input
    gpio_reset_pin(output_pin);
    esp_err_t err = gpio_set_direction(output_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_PIR, "Failed to set GPIO direction for HC-SR501");
        return err;
    }

    // Enable pull-down resistor (HC-SR501 output is active HIGH)
    err = gpio_set_pull_mode(output_pin, GPIO_PULLDOWN_ONLY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_PIR, "Failed to set pull mode for HC-SR501, continuing anyway");
    }

    ESP_LOGI(TAG_PIR, "HC-SR501 initialized on GPIO %d", output_pin);
    return ESP_OK;
}

bool hcsr501_read(hcsr501_dev_t *dev)
{
    if (dev == NULL) {
        ESP_LOGW(TAG_PIR, "HC-SR501 device is NULL");
        return false;
    }

    int level = gpio_get_level(dev->output_pin);
    bool current_state = (level == 1);
    
    if (current_state != dev->last_state) {
        dev->last_state = current_state;
        if (current_state) {
            dev->last_detection_time = esp_timer_get_time();
            ESP_LOGD(TAG_PIR, "Motion detected");
        } else {
            ESP_LOGD(TAG_PIR, "Motion ended");
        }
    }
    
    return current_state;
}
