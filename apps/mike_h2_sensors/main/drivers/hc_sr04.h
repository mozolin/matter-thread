#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t trig_pin;
    gpio_num_t echo_pin;
    uint32_t timeout_us;
    float distance_cm;
    void (*distance_callback)(float distance_cm);
} hc_sr04_t;

void hc_sr04_init(hc_sr04_t *sensor, gpio_num_t trig_pin, gpio_num_t echo_pin);
float hc_sr04_measure(hc_sr04_t *sensor);
void hc_sr04_set_callback(hc_sr04_t *sensor, void (*callback)(float distance_cm));
void hc_sr04_echo_isr_handler(void *arg);

#ifdef __cplusplus
}
#endif