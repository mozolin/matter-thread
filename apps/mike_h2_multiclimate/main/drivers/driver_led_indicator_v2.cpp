// driver_led_indicator.cpp
#include "driver_led_indicator_v2.h"
#include "led_indicator.h"
#include <app_priv.h>


#if USE_DRIVER_LED_INDICATOR
  bool isLEDInverseBlinking = IS_LED_INVERSE_BLINKING;

  // *ИЗМЕНЕНО*: Глобальный хэндл для совместимости с существующим кодом.
  // В идеале его следует передавать через аргументы, но для минимальных изменений оставляем так.
  static led_indicator_handle_t led_handle = NULL;

  //-- use it, if there is at least one RGB LED
  #if USE_RGB_LED
    // *БЕЗ ИЗМЕНЕНИЙ*: Все определения blink_step_t остаются абсолютно такими же.
    // Версия 2.x полностью совместима с форматом blink_step_t из v1.x.
    //-- Just turn on the yellow color
    const blink_step_t yellow_on[] = {
      //-- Set color to yellow
      {LED_BLINK_RGB, SET_RGB(128, 128, 0), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 15000},
      {LED_BLINK_STOP, 0, 0},
    };
  
    //-- Just turn on the orange color
    const blink_step_t orange_on[] = {
      //-- Set color to orange
      {LED_BLINK_RGB, SET_RGB(128, 64, 0), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 15000},
      {LED_BLINK_STOP, 0, 0},
    };
  
    //-- Blinking twice times in red
    const blink_step_t double_red_blink[] = {
      //-- Set color to red
      {LED_BLINK_RGB, SET_RGB(128, 0, 0), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 500},
      {LED_BLINK_HOLD, LED_STATE_OFF, 500},
      {LED_BLINK_HOLD, LED_STATE_ON, 500},
      {LED_BLINK_HOLD, LED_STATE_OFF, 500},
      {LED_BLINK_STOP, 0, 0},
    };
  
    //-- Blinking three times in green
    const blink_step_t triple_green_blink[] = {
      //-- Set color to green
      {LED_BLINK_RGB, SET_RGB(0, 128, 0), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 500},
      {LED_BLINK_HOLD, LED_STATE_OFF, 500},
      {LED_BLINK_HOLD, LED_STATE_ON, 500},
      {LED_BLINK_HOLD, LED_STATE_OFF, 500},
      {LED_BLINK_HOLD, LED_STATE_ON, 500},
      {LED_BLINK_HOLD, LED_STATE_OFF, 500},
      {LED_BLINK_STOP, 0, 0},
    };
  
    //-- Blinking once in red
    const blink_step_t red_once_blink[] = {
      //-- Set color to red
      {LED_BLINK_RGB, SET_RGB(32, 0, 0), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 40},
      {LED_BLINK_HOLD, LED_STATE_OFF, 40},
      //{LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Blinking once in green
    const blink_step_t green_once_blink[] = {
      //-- Set color to green
      //{LED_BLINK_RGB, SET_RGB(0, 32, 0), 0},
      //-- Set color to purple
      {LED_BLINK_RGB, SET_RGB(32, 0, 32), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 40},
      {LED_BLINK_HOLD, LED_STATE_OFF, 40},
      //{LED_BLINK_LOOP, 0, 0},
    };
  
  
    //-- Blinking once in blue
    const blink_step_t blue_once_blink[] = {
      //-- Set color to blue
      {LED_BLINK_RGB, SET_RGB(0, 0, 32), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 40},
      {LED_BLINK_HOLD, LED_STATE_OFF, 40},
      //{LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Blinking once
    const blink_step_t live_once_blink[] = {
      {LED_BLINK_RGB, SET_RGB(32, 0, 32), 0},
      {LED_BLINK_HOLD, LED_STATE_ON, 40},
      {LED_BLINK_HOLD, LED_STATE_OFF, 40},
      //{LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Slow breathing in white
    const blink_step_t breath_white_slow_blink[] = {
      //-- Set Color to white and brightness to zero by H:0 S:0 V:0
      {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
      {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
      {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
      {LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Fast breathing in white
    const blink_step_t breath_white_fast_blink[] = {
      //-- Set Color to white and brightness to zero by H:0 S:0 V:0
      {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
      {LED_BLINK_BREATHE, LED_STATE_ON, 500},
      {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
      {LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Breathing in green
    const blink_step_t breath_blue_blink[] = {
      //-- Set Color to blue and brightness to zero by H:240 S:255 V:0
      {LED_BLINK_HSV, SET_HSV(240, MAX_SATURATION, 0), 0},
      {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
      {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
      {LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Color gradient by HSV
    const blink_step_t color_hsv_ring_blink[] = {
      //-- Set Color to RED
      {LED_BLINK_HSV, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
      {LED_BLINK_HSV_RING, SET_HSV(240, MAX_SATURATION, 127), 2000},
      {LED_BLINK_HSV_RING, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
      {LED_BLINK_LOOP, 0, 0},
    };
  
    //-- Color gradient by RGB
    const blink_step_t color_rgb_ring_blink[] = {
      //-- Set Color to Green
      {LED_BLINK_RGB, SET_RGB(0, 255, 0), 0},
      {LED_BLINK_RGB_RING, SET_RGB(255, 0, 255), 2000},
      {LED_BLINK_RGB_RING, SET_RGB(0, 255, 0), 2000},
      {LED_BLINK_LOOP, 0, 0},
    };
  
    #if LED_NUMBERS > 1
      //-- Flowing lights. Insert the index:MAX_INDEX to control all the strips
      const blink_step_t flowing_blink[] = {
        {LED_BLINK_HSV, SET_IHSV(MAX_INDEX, 0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
        {LED_BLINK_HSV_RING, SET_IHSV(MAX_INDEX, MAX_HUE, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
        {LED_BLINK_LOOP, 0, 0},
      };
    #endif

    // *ИЗМЕНЕНО*: Массив указателей на blink_step_t теперь является частью конфигурации,
    // но мы сохраняем его для внутреннего использования в get_led_indicator_blink_idx.
    blink_step_t const *led_mode[] = {
      [BLINK_ON_YELLOW] = yellow_on,
      [BLINK_ON_ORANGE] = orange_on,
      [BLINK_DOUBLE_RED] = double_red_blink,
      [BLINK_TRIPLE_GREEN] = triple_green_blink,
      [BLINK_ONCE_RED] = red_once_blink,
      [BLINK_ONCE_GREEN] = green_once_blink,
      [BLINK_ONCE_BLUE] = blue_once_blink,
      [BLINK_ONCE_LIVE] = live_once_blink,
      [BLINK_WHITE_BREATHE_SLOW] = breath_white_slow_blink,
      [BLINK_WHITE_BREATHE_FAST] = breath_white_fast_blink,
      [BLINK_BLUE_BREATH] = breath_blue_blink,
      [BLINK_COLOR_HSV_RING] = color_hsv_ring_blink,
      [BLINK_COLOR_RGB_RING] = color_rgb_ring_blink,
      #if LED_NUMBERS > 1
        [BLINK_FLOWING] = flowing_blink,
      #endif
      [BLINK_MAX] = NULL,
    };
  
    // *ПОЛНОСТЬЮ ПЕРЕПИСАНО*: Функция создания индикатора.
    // Теперь использует заводской API версии 2.x.
    led_indicator_handle_t configure_indicator(void)
    {
      // *ДОБАВЛЕНО*: Конфигурация для ленты (WS2812).
      // В v2.x используется отдельная структура для каждого типа драйвера.
      led_strip_config_t strip_config = {
        .strip_gpio_num = LED_BLINK_GPIO,
        .max_leds = LED_NUMBERS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        {
          .invert_out = false,
        },
      };
    
      // *ДОБАВЛЕНО*: Конфигурация RMT для ленты.
      led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_RMT_RES_HZ,
        .mem_block_symbols = 0,
        {
          .with_dma = false,
        },
      };
    
      // *ИЗМЕНЕНО*: Структура конфигурации ленты для v2.x.
      led_indicator_strips_config_t strips_config = {
        .led_strip_cfg = strip_config,
        .led_strip_driver = LED_STRIP_RMT,
        .led_strip_rmt_cfg = rmt_config,
      };
    
      // *ДОБАВЛЕНО*: Основная конфигурация индикатора для v2.x.
      // Она больше не содержит union с разными типами конфигураций.
      const led_indicator_config_t config = {
        .blink_lists = led_mode,      // Передаем массив анимаций
        .blink_list_num = BLINK_MAX,  // Передаем количество анимаций
      };
    
      // *ИЗМЕНЕНО*: Вызов фабричной функции для создания устройства на светодиодной ленте.
      // Вместо led_indicator_create используем led_indicator_new_strips_device.
      // Обратите внимание на порядок аргументов: (config, hardware_config, handle).
      esp_err_t ret = led_indicator_new_strips_device(&config, &strips_config, &led_handle);
  
      // *ДОБАВЛЕНО*: Проверка ошибки создания.
      assert(ret == ESP_OK);
      assert(led_handle != NULL);
  
      return led_handle;
    }
  
    // *ИЗМЕНЕНО*: Логика функции получения индекса.
    // В v2.x нет необходимости перебирать массив для поиска индекса,
    // так как blink_type напрямую соответствует индексу в перечислении.
    uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay)
    {
      // Проверяем, что тип анимации корректен.
      if (blink_type >= BLINK_MAX) {
        return 255;
      }
  
      // *УДАЛЕНО*: Сложная логика поиска индекса через сравнение указателей.
      // В v2.x индекс анимации (blink_type) напрямую используется в API start/stop.
      // Мы просто возвращаем его.
  
      if(start_delay > 0) {
        led_indicator_start(led_handle, blink_type);
        vTaskDelay(start_delay / portTICK_PERIOD_MS);
  
        led_indicator_stop(led_handle, blink_type);
        if(stop_delay > 0) {
          vTaskDelay(stop_delay / portTICK_PERIOD_MS);
        }
      }
  
      return blink_type;
    }
  
    #if LIVE_BLINK_TIME_MS > 0
      void init_indicator_task(void *pvParameter)
      {
      
        uint32_t live_blink_time = 0;
      
        while(1) {
          //-- blink every "LIVE_BLINK_TIME_MS" milliseconds
          uint32_t blinked_duration = esp_log_timestamp() - live_blink_time;
          if(blinked_duration >= LIVE_BLINK_TIME_MS) {
            get_led_indicator_blink_idx(BLINK_ONCE_LIVE, 60, 0);
            live_blink_time = esp_log_timestamp();
          }
      
          vTaskDelay(pdMS_TO_TICKS(10));
        }
      }
    #endif //-- LIVE_BLINK_TIME_MS > 0
  
  #endif //-- USE_RGB_LED

  // *БЕЗ ИЗМЕНЕНИЙ*: Код для обычных LED (USE_ORDINARY_LED) остается нетронутым,
  // так как он использует прямые вызовы GPIO и не зависит от API led_indicator.
  #if USE_ORDINARY_LED
    // ... (весь код для LED_MODE 1, 2, 3 остается без изменений) ...
  #endif //-- USE_ORDINARY_LED

#endif //-- USE_DRIVER_LED_INDICATOR
