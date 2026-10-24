#pragma once

#include <led_config.h>
#include "led_indicator.h"

#if USE_DRIVER_LED_INDICATOR
  /*********************
   *                   *
   *   LED INDICATOR   *
   *                   *
   *********************/
  //-- GPIO assignment
  //-- use LED 1, if it is RGB LED
  #if ESP32_RGB_LED_1
    #define LED_BLINK_GPIO  ESP32_GPIO_LED_1
  #endif
  //-- use LED 2, if it is RGB LED
  #if ESP32_RGB_LED_2
    #define LED_BLINK_GPIO  ESP32_GPIO_LED_2
  #endif
  //-- use RGB LED
  #define USE_RGB_LED  (ESP32_RGB_LED_1 || ESP32_RGB_LED_2)
  //-- use ordinary LED
  #define USE_ORDINARY_LED  (!ESP32_RGB_LED_1 || !ESP32_RGB_LED_2)
  //-- Numbers of the LED in the strip
  #define LED_NUMBERS 1
  //-- 10MHz resolution, 1 tick = 0.1us (led strip needs a high resolution)
  #define LED_RMT_RES_HZ  (10 * 1000 * 1000)

  //-- use it, if there is at least one RGB LED
  #if USE_RGB_LED
  
    extern led_indicator_handle_t configure_indicator(void);
  
    //-- Define blinking type and priority.
    enum {
      BLINK_ON_YELLOW,
      BLINK_ON_ORANGE,
      BLINK_DOUBLE_RED,
      BLINK_TRIPLE_GREEN,
      BLINK_ONCE_RED,
      BLINK_ONCE_GREEN,
      BLINK_ONCE_BLUE,
      BLINK_ONCE_LIVE,
      BLINK_WHITE_BREATHE_SLOW,
      BLINK_WHITE_BREATHE_FAST,
      BLINK_BLUE_BREATH,
      BLINK_COLOR_HSV_RING,
      BLINK_COLOR_RGB_RING,
      #if LED_NUMBERS > 1
        BLINK_FLOWING,
      #endif
      BLINK_MAX,
    };
    //-- Just turn on the yellow color
    extern const blink_step_t yellow_on[];
    //-- Just turn on the orange color
    extern const blink_step_t orange_on[];
    //-- Blinking twice times in red
    extern const blink_step_t double_red_blink[];
    //-- Blinking three times in green
    extern const blink_step_t triple_green_blink[];
    //-- Blinking once in red
    extern const blink_step_t red_once_blink[];
    //-- Blinking once in green
    extern const blink_step_t green_once_blink[];
    //-- Blinking once in blue
    extern const blink_step_t blue_once_blink[];
    //-- Blinking once
    extern const blink_step_t live_once_blink[];
    //-- Slow breathing in white
    extern const blink_step_t breath_white_slow_blink[];
    //-- Fast breathing in white
    extern const blink_step_t breath_white_fast_blink[];
    //-- Breathing in green
    extern const blink_step_t breath_blue_blink[];
    //-- Color gradient by HSV
    extern const blink_step_t color_hsv_ring_blink[];
    //-- Color gradient by RGB
    extern const blink_step_t color_rgb_ring_blink[];
    #if LED_NUMBERS > 1
      //-- Flowing lights. Insert the index:MAX_INDEX to control all the strips
      extern const blink_step_t flowing_blink[];
    #endif
  
    extern blink_step_t const *led_mode[];
  
    #if LIVE_BLINK_TIME_MS > 0
      extern void init_indicator_task(void *pvParameter);
    #endif
  
    extern led_indicator_handle_t led_handle;
  
    extern uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay);
  #endif //-- USE_RGB_LED


  #if USE_ORDINARY_LED
    #include "driver/gpio.h"
  
    //-- LED 1 for UART activity indication
    #define LED_RX_GPIO ESP32_GPIO_LED_1
    //-- use, if LED 1 is not RGB 
    #define USE_LED_RX_GPIO !ESP32_RGB_LED_1
  
    //-- LED 2 for UART activity indication
    #define LED_TX_GPIO ESP32_GPIO_LED_2
    //-- use, if LED 2 is not RGB 
    #define USE_LED_TX_GPIO !ESP32_RGB_LED_2
  
  
    extern bool isLEDInverseBlinking;
  
    /**********************************************
     *                                            *
     *  1. Simple version with random intervals   *
     *                                            *
     **********************************************/
    #if LED_MODE == 1
      //-- Structure for LED state
      typedef struct {
        gpio_num_t gpio;
        const char* name;
        bool state;
        TickType_t next_change;
      } led_state_t;
  
      //-- Pseudo-random number generator (simple)
      extern uint32_t simple_random(void);
  
      //-- Random blinking task
      extern void random_blink_task(void *pvParameters);
  
    #endif //-- LED_MODE == 1
  
  
    /**********************************************
     *                                            *
     *  2. Realistic version with UART patterns   *
     *                                            *
     **********************************************/
    #if LED_MODE == 2
  
      extern void simulate_uart_activity(void *aContext);
  
    #endif //-- LED_MODE == 1
  
  
    /************************************************
     *                                              *
     *   3. Version with different activity modes   *
     *                                              *
     ************************************************/
    #if LED_MODE == 3
      typedef enum {
        MODE_IDLE,    //-- Rare single flashes
        MODE_ACTIVE,  //-- Frequent packages
        MODE_BURST,   //-- Packages in packs
        MODE_ERROR    //-- Fast blinking (error)
      } activity_mode_t;
    
      extern activity_mode_t current_mode;
  
      extern void set_mode(activity_mode_t mode);
  
      extern void uart_simulation_task(void *pvParameters);
  
    #endif //-- LED_MODE == 3
  
  #endif //-- USE_ORDINARY_LED

#endif //-- USE_DRIVER_LED_INDICATOR
