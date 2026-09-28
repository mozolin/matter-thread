
#include <stdio.h>
#include <stdbool.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ultrasonic.h>
#include <esp_err.h>

#include "app_priv.h"

#define MAX_DISTANCE_CM 500 // 5m max

#define TRIGGER_GPIO    CONFIG_HCSR04_TRIG_GPIO
#define ECHO_GPIO       CONFIG_HCSR04_ECHO_GPIO


ultrasonic_sensor_t sensor =
{
    .trigger_pin = (gpio_num_t)TRIGGER_GPIO,
    .echo_pin = (gpio_num_t)ECHO_GPIO
};


esp_err_t hcsr04_init1()
{
    ultrasonic_init(&sensor);
    
    return ESP_OK;
}

uint32_t hcsr04_measure_distance1()
{
    float distance;
    esp_err_t res = ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
    if (res != ESP_OK)
    {
        printf("Error %d: ", res);
        switch (res)
        {
            case ESP_ERR_ULTRASONIC_PING:
                printf("Cannot ping (device is in invalid state)\n");
                break;
            case ESP_ERR_ULTRASONIC_PING_TIMEOUT:
                printf("Ping timeout (no device found)\n");
                break;
            case ESP_ERR_ULTRASONIC_ECHO_TIMEOUT:
                printf("Echo timeout (i.e. distance too big)\n");
                break;
            default:
                printf("%s\n", esp_err_to_name(res));
        }
    }
    else
        printf("Distance: %0.04f m\n", distance);
    
    return distance;
}

esp_err_t hcsr04_reset1()
{
    return ESP_OK;
}
