#include "sensor_pir.h"
#include <esp_timer.h>

#if DEBUG_MODE
  static const char* TAG_PIR = "PIR_SENSOR";
#endif

esp_err_t hcsr501_init_app(hcsr501_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_PIR, "Device pointer is NULL");
        #endif
        return ESP_ERR_INVALID_ARG;
    }

    dev->output_pin = output_pin;
    dev->last_state = false;
    dev->last_detection_time = 0;

    // Configure GPIO as input
    gpio_reset_pin(output_pin);
    esp_err_t err = gpio_set_direction(output_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_PIR, "Failed to set GPIO direction for HC-SR501");
        #endif
        return err;
    }

    // Enable pull-down resistor (HC-SR501 output is active HIGH)
    err = gpio_set_pull_mode(output_pin, GPIO_PULLDOWN_ONLY);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_PIR, "Failed to set pull mode for HC-SR501, continuing anyway");
        #endif
    }

    #if DEBUG_MODE
      ESP_LOGI(TAG_PIR, "HC-SR501 initialized on GPIO %d", output_pin);
    #endif
    return ESP_OK;
}

bool hcsr501_read_app(hcsr501_dev_t *dev)
{
    if (dev == NULL) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_PIR, "HC-SR501 device is NULL");
        #endif
        return false;
    }

    int level = gpio_get_level(dev->output_pin);
    bool current_state = (level == 1);
    
    if (current_state != dev->last_state) {
        dev->last_state = current_state;
        if (current_state) {
            dev->last_detection_time = esp_timer_get_time();
            #if DEBUG_MODE
              ESP_LOGD(TAG_PIR, "Motion detected");
            #endif
        } else {
            #if DEBUG_MODE
              ESP_LOGD(TAG_PIR, "Motion ended");
            #endif
        }
    }
    
    return current_state;
}
