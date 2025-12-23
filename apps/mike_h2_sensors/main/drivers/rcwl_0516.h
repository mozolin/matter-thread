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
} rcwl_0516_t;

void rcwl_0516_init(rcwl_0516_t *sensor, gpio_num_t pin);
bool rcwl_0516_read(rcwl_0516_t *sensor);
void rcwl_0516_set_callback(rcwl_0516_t *sensor, void (*callback)(bool motion));
void rcwl_0516_isr_handler(void *arg);

#ifdef __cplusplus
}
#endif