#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ultrasonic.h>
#include <esp_err.h>

#include "sensor_ultrasonic.h"

#define MAX_DISTANCE_CM 500 // 5m max
#define TRIGGER_GPIO    3
#define ECHO_GPIO       5

static const char* TAG_ULTRASONIC = "ULTRASONIC_SENSOR";

ultrasonic_sensor_t sensor =
{
    .trigger_pin = (gpio_num_t)TRIGGER_GPIO,
    .echo_pin = (gpio_num_t)ECHO_GPIO
};


esp_err_t hcsr04_init(hcsr04_dev_t *dev, gpio_num_t trigger_pin, gpio_num_t echo_pin)
{
    /*
    ultrasonic_init(&sensor);
    */


    gpio_num_t t_pin = sensor.trigger_pin;
    gpio_num_t e_pin = sensor.echo_pin;

    // Configure trigger pin as output
    gpio_reset_pin(t_pin);
    esp_err_t err = gpio_set_direction(t_pin, GPIO_MODE_OUTPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ULTRASONIC, "Failed to set GPIO direction for HC-SR04 trigger");
        return err;
    }
    gpio_set_level(t_pin, 0);

    // Configure echo pin as input
    gpio_reset_pin(e_pin);
    err = gpio_set_direction(e_pin, GPIO_MODE_INPUT);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_ULTRASONIC, "Failed to set GPIO direction for HC-SR04 echo");
        return err;
    }

    return ESP_OK;
}

uint32_t hcsr04_measure_distance(hcsr04_dev_t *dev)
{
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    float distance;
    esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
    if (res != ESP_OK)
    {
        switch (res)
        {
            case ESP_ERR_ULTRASONIC_PING:
                ESP_LOGE(TAG_ULTRASONIC, "Error %d: Cannot ping (device is in invalid state)", res);
                break;
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                ESP_LOGE(TAG_ULTRASONIC, "Error %d: Ping timeout (no device found)", res);
                break;
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                ESP_LOGE(TAG_ULTRASONIC, "Error %d: Echo timeout (i.e. distance too big)", res);
                break;
            default:
                ESP_LOGE(TAG_ULTRASONIC, "Error %d: %s", res, esp_err_to_name(res));
        }
    }
    else {
        distance *= 100;
        ESP_LOGW(TAG_ULTRASONIC, "Distance: %0.04f m", distance);
    }
    
    return distance;
}

esp_err_t hcsr04_reset(hcsr04_dev_t *dev)
{
    return ESP_OK;
}
