// driver_led_indicator_v2.h
#pragma once

#include <led_config.h>
#include "led_indicator.h"
#include "led_indicator_strips.h"
// *ДОБАВЛЕНО*: Явное включение для работы с типами данных новой версии.
#include "led_types.h"

#if USE_DRIVER_LED_INDICATOR
  /*********************
   *                   *
   *   LED INDICATOR   *
   *                   *
   *********************/
  //-- GPIO assignment (без изменений)
  #if ESP32_RGB_LED_1
    #define LED_BLINK_GPIO  ESP32_GPIO_LED_1
  #endif
  #if ESP32_RGB_LED_2
    #define LED_BLINK_GPIO  ESP32_GPIO_LED_2
  #endif
  #define USE_RGB_LED  (ESP32_RGB_LED_1 || ESP32_RGB_LED_2)
  #define USE_ORDINARY_LED  (!ESP32_RGB_LED_1 || !ESP32_RGB_LED_2)
  #define LED_NUMBERS 1
  #define LED_RMT_RES_HZ  (10 * 1000 * 1000)

  //-- use it, if there is at least one RGB LED
  #if USE_RGB_LED
  
    // *ИЗМЕНЕНО*: Теперь эта функция должна возвращать хэндл, созданный с помощью нового API.
    // Интерфейс остался прежним, но реализация в .cpp файле полностью изменена.
    extern led_indicator_handle_t configure_indicator(void);
  
    //-- Define blinking type and priority. (enum без изменений)
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
    //-- Объявления массивов blink_step_t (без изменений)
    //-- Just turn on the yellow color
    extern const blink_step_t yellow_on[];
    // ... и так далее для всех остальных blink_step_t ...
    extern const blink_step_t orange_on[];
    extern const blink_step_t double_red_blink[];
    extern const blink_step_t triple_green_blink[];
    extern const blink_step_t red_once_blink[];
    extern const blink_step_t green_once_blink[];
    extern const blink_step_t blue_once_blink[];
    extern const blink_step_t live_once_blink[];
    extern const blink_step_t breath_white_slow_blink[];
    extern const blink_step_t breath_white_fast_blink[];
    extern const blink_step_t breath_blue_blink[];
    extern const blink_step_t color_hsv_ring_blink[];
    extern const blink_step_t color_rgb_ring_blink[];
    #if LED_NUMBERS > 1
      extern const blink_step_t flowing_blink[];
    #endif
  
    // *УДАЛЕНО*: extern blink_step_t const *led_mode[];
    // *УДАЛЕНО*: extern led_indicator_handle_t led_handle;
    // В версии 2.x эти данные инкапсулированы в процессе создания.
  
    #if LIVE_BLINK_TIME_MS > 0
      extern void init_indicator_task(void *pvParameter);
    #endif
  
    // *ДОБАВЛЕНО/ИЗМЕНЕНО*: Объявление get_led_indicator_blink_idx осталось,
    // но внутри .cpp файла для получения индекса используется логика v2.x.
    extern uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay);
  #endif //-- USE_RGB_LED


  #if USE_ORDINARY_LED
    // *ИЗМЕНЕНО*: Для управления простыми GPIO в v2.x используется другой API (не требует создания хэндла).
    // Однако, чтобы не переписывать всю логику для обычных LED, мы оставим её как есть,
    // так как она использует прямые вызовы gpio_set_level, а не API led_indicator.
    // Единственное изменение — это соответствие типов для `led_state_t`.
    #include "driver/gpio.h"
  
    #define LED_RX_GPIO ESP32_GPIO_LED_1
    #define USE_LED_RX_GPIO !ESP32_RGB_LED_1
    #define LED_TX_GPIO ESP32_GPIO_LED_2
    #define USE_LED_TX_GPIO !ESP32_RGB_LED_2
  
    extern bool isLEDInverseBlinking;
  
    // ... остальные объявления для LED_MODE 1, 2, 3 без изменений ...
    // Они работают с прямыми вызовами GPIO и не зависят от версии led_indicator.
    #if LED_MODE == 1
      typedef struct { gpio_num_t gpio; const char* name; bool state; TickType_t next_change; } led_state_t;
      extern uint32_t simple_random(void);
      extern void random_blink_task(void *pvParameters);
    #endif
    #if LED_MODE == 2
      extern void simulate_uart_activity(void *aContext);
    #endif
    #if LED_MODE == 3
      typedef enum { MODE_IDLE, MODE_ACTIVE, MODE_BURST, MODE_ERROR } activity_mode_t;
      extern activity_mode_t current_mode;
      extern void set_mode(activity_mode_t mode);
      extern void uart_simulation_task(void *pvParameters);
    #endif
  #endif //-- USE_ORDINARY_LED
#endif //-- USE_DRIVER_LED_INDICATOR
