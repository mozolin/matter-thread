/*
   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

#pragma once

#include <esp_err.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "soc/gpio_num.h"
#include "app_priv.h"

#ifdef __cplusplus
extern "C" {
#endif

// HC-SR501 PIR Sensor Driver
typedef struct {
    gpio_num_t output_pin;
    bool last_state;
    uint64_t last_detection_time;
} hcsr501_dev_t;

// RCWL-0516 Microwave Sensor Driver
typedef struct {
    gpio_num_t output_pin;
    bool last_state;
    uint64_t last_detection_time;
} rcwl0516_dev_t;

// HC-SR04 Ultrasonic Sensor Driver
typedef struct {
    gpio_num_t trigger_pin;
    gpio_num_t echo_pin;
    uint32_t last_distance_cm;
    uint64_t last_measurement_time;
} hcsr04_dev_t;

// Initialize PIR sensor
esp_err_t hcsr501_init(hcsr501_dev_t *dev, gpio_num_t output_pin);

// Read PIR sensor state
bool hcsr501_read(hcsr501_dev_t *dev);

// Initialize microwave sensor
esp_err_t rcwl0516_init(rcwl0516_dev_t *dev, gpio_num_t output_pin);

// Read microwave sensor state
bool rcwl0516_read(rcwl0516_dev_t *dev);

// Initialize ultrasonic sensor
esp_err_t hcsr04_init(hcsr04_dev_t *dev, gpio_num_t trigger_pin, gpio_num_t echo_pin);

// Measure distance with ultrasonic sensor (in cm)
uint32_t hcsr04_measure_distance(hcsr04_dev_t *dev);

#ifdef __cplusplus
}
#endif