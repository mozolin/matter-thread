#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "driver/gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

// Общая структура для всех датчиков
typedef struct {
    gpio_num_t gpio_pin;
    bool last_state;
    uint32_t debounce_time;
    uint64_t last_trigger_time;
} sensor_common_t;

// Инициализация GPIO
void sensor_gpio_init(gpio_num_t pin, gpio_mode_t mode, gpio_pull_mode_t pull);

// Функции обработки прерываний
typedef void (*sensor_callback_t)(void* arg);
void sensor_set_interrupt(gpio_num_t pin, sensor_callback_t callback, void* arg);

#ifdef __cplusplus
}
#endif