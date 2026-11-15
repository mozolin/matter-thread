#include "sensor_microwave.h"
#include <esp_timer.h>

#if DEBUG_MODE
  static const char* TAG_MICROWAVE = "MICROWAVE_SENSOR";
#endif

esp_err_t rcwl0516_init_app(rcwl0516_dev_t *dev, gpio_num_t output_pin)
{
    if (dev == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MICROWAVE, "Device pointer is NULL");
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
          ESP_LOGE(TAG_MICROWAVE, "Failed to set GPIO direction for RCWL-0516");
        #endif
        return err;
    }

    // RCWL-0516 output is active LOW, enable pull-up
    err = gpio_set_pull_mode(output_pin, GPIO_PULLUP_ONLY);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_MICROWAVE, "Failed to set pull mode for RCWL-0516, continuing anyway");
        #endif
    }

    #if DEBUG_MODE
      ESP_LOGI(TAG_MICROWAVE, "RCWL-0516 initialized on GPIO %d", output_pin);
    #endif
    return ESP_OK;
}

bool rcwl0516_read_app(rcwl0516_dev_t *dev)
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
            #if DEBUG_MODE
              ESP_LOGD(TAG_MICROWAVE, "Motion detected");
            #endif
        } else {
            #if DEBUG_MODE
              ESP_LOGD(TAG_MICROWAVE, "Motion ended");
            #endif
        }
    }
    
    return current_state;
}
