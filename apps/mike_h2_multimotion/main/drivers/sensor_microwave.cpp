#include "sensor_microwave.h"
#include <esp_timer.h>

static const char* TAG_MICROWAVE = "MICROWAVE_SENSOR";

esp_err_t rcwl0516_init(rcwl0516_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        ESP_LOGE(TAG_MICROWAVE, "Device pointer is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = output_pin;
    dev->last_state = false;
    dev->last_detection_time = 0;

    // Configure GPIO as input
    gpio_reset_pin(output_pin);
    esp_err_t err = gpio_set_direction(output_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MICROWAVE, "Failed to set GPIO direction for RCWL-0516");
        return err;
    }

    // RCWL-0516 output is active LOW, enable pull-up
    err = gpio_set_pull_mode(output_pin, GPIO_PULLUP_ONLY);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MICROWAVE, "Failed to set pull mode for RCWL-0516, continuing anyway");
    }

    ESP_LOGI(TAG_MICROWAVE, "RCWL-0516 initialized on GPIO %d", output_pin);
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
            ESP_LOGD(TAG_MICROWAVE, "Motion detected");
        } else {
            ESP_LOGD(TAG_MICROWAVE, "Motion ended");
        }
    }
    
    return current_state;
}
