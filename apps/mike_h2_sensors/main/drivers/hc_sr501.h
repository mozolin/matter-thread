#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    gpio_num_t gpio_pin;
    bool motion_detected;
    uint32_t debounce_ms;
    uint64_t last_detection_time;
    void (*motion_callback)(bool motion);
} hc_sr501_t;

void hc_sr501_init(hc_sr501_t *sensor, gpio_num_t pin);
bool hc_sr501_read(hc_sr501_t *sensor);
void hc_sr501_set_callback(hc_sr501_t *sensor, void (*callback)(bool motion));
void hc_sr501_isr_handler(void *arg);

#ifdef __cplusplus
}
#endif