#pragma once

#include "sensor_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// RCWL-0516 Microwave Sensor Driver
typedef struct {
    gpio_num_t output_pin;
    bool last_state;
    uint64_t last_detection_time;
} rcwl0516_dev_t;

// Initialize microwave sensor
esp_err_t rcwl0516_init(rcwl0516_dev_t *dev, gpio_num_t output_pin);

// Read microwave sensor state
bool rcwl0516_read(rcwl0516_dev_t *dev);

#ifdef __cplusplus
}
#endif
