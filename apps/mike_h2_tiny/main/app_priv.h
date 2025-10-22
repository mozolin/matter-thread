#pragma once

#include <esp_err.h>
#include <esp_matter.h>

#define TAG_H2   "Mike H2"

/*
ESP OTBR & Zigbee GW (RCP ESP32-H2):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GPIO_NUM_22 //-- yellow (old)/green (new)
GPIO_NUM_25 //-- blue

ESP32-H2 SuperMini:
~~~~~~~~~~~~~~~~~~~
GPIO_NUM_8  //-- RGB
GPIO_NUM_13 //-- blue

if ESP32H2_GPIO_LED_1 = -1 => do not use this LED!
*/
#define ESP32H2_GPIO_LED_1   GPIO_NUM_8
#define ESP32H2_RGB_LED_1    true

#define ESP32H2_GPIO_LED_2   GPIO_NUM_13
#define ESP32H2_RGB_LED_2    false


#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
	#include "esp_openthread_types.h"
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
	#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
  {                                                                                   \
    .radio_mode = RADIO_MODE_NATIVE,                                                \
  }

	#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
  {                                                                                   \
    .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
  }

	#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
  {                                                                                   \
    .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
  }
#endif


//#include "esp32h2_led.h"
#include "driver_led_indicator.h"

