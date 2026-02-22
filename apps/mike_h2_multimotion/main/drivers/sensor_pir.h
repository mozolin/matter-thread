#pragma once

#include "sensor_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// HC-SR501 PIR Sensor Driver
typedef struct {
    gpio_num_t output_pin;
    bool last_state;
    uint64_t last_detection_time;
} hcsr501_dev_t;

// Initialize PIR (HC-SR501) sensor
esp_err_t hcsr501_init(hcsr501_dev_t *dev, gpio_num_t output_pin);

// Read PIR (HC-SR501) sensor state
bool hcsr501_read(hcsr501_dev_t *dev);

#ifdef __cplusplus
}
#endif
