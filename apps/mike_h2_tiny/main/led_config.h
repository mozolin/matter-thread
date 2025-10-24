#pragma once

//-- whether or not to use "driver_led_indicator"
#define USE_DRIVER_LED_INDICATOR true

//-- whether or not to output to the UART console
#define DEBUG_MODE               false

/*
1. Simple version with random intervals
2. Realistic version with UART patterns
3. Version with different activity modes
*/
#define LED_MODE                 1

#define IS_LED_INVERSE_BLINKING  1

/*
ESP OTBR & Zigbee GW (RCP ESP32-H2):
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
GPIO_NUM_22 //-- yellow (old)/green (new)
GPIO_NUM_25 //-- blue

ESP32-H2 SuperMini:
~~~~~~~~~~~~~~~~~~~
GPIO_NUM_8  //-- RGB
GPIO_NUM_13 //-- blue
*/
#define ESP32_GPIO_LED_1         GPIO_NUM_8
//-- is LED #1 an RGB LED or not
#define ESP32_RGB_LED_1          true

#define ESP32_GPIO_LED_2         GPIO_NUM_13
//-- is LED #2 an RGB LED or not
#define ESP32_RGB_LED_2          false
