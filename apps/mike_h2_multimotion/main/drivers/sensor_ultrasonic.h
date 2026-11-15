#pragma once

#include "sensor_common.h"

#ifdef __cplusplus
extern "C" {
#endif

// HC-SR04 Ultrasonic Sensor Driver
typedef struct {
    gpio_num_t trigger_pin;
    gpio_num_t echo_pin;
    uint32_t last_distance_cm;
    uint64_t last_measurement_time;
} hcsr04_dev_t;

// Initialize ultrasonic sensor
esp_err_t hcsr04_init_app(hcsr04_dev_t *dev, gpio_num_t trigger_pin, gpio_num_t echo_pin);

// Measure distance with ultrasonic sensor (in cm)
uint32_t hcsr04_read_app(hcsr04_dev_t *dev);

// Reset functions
esp_err_t hcsr04_reset_app(hcsr04_dev_t *dev);

#ifdef __cplusplus
}
#endif
