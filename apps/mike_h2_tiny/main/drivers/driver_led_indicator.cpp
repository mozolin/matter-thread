
#include "driver_led_indicator.h"

#if USE_DRIVER_LED_INDICATOR
  bool isLEDInverseBlinking = IS_LED_INVERSE_BLINKING;

  //-- use it, if there is at least one RGB LED
  #if USE_RGB_LED
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
  
    uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay)
    {
      uint8_t idx = 255;
      
      int size = sizeof(led_mode)/sizeof(led_mode[0]);
    
      auto item = led_mode[blink_type];
      for(int i=0; i<size; i++) {
        if(led_mode[i] == item) {
          idx = i;
    
          if(start_delay > 0) {
            led_indicator_start(led_handle, idx);
            //vTaskDelay(pdMS_TO_TICKS(start_delay));
            vTaskDelay(start_delay / portTICK_PERIOD_MS);
    
            led_indicator_stop(led_handle, idx);
            if(stop_delay > 0) {
              //vTaskDelay(pdMS_TO_TICKS(stop_delay));
              vTaskDelay(stop_delay / portTICK_PERIOD_MS);
            }
          }
    
          break;
        }
      }
    
      return idx;
    }
  
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
  
    led_indicator_handle_t configure_indicator(void)
    {
      //-- LED strip general initialization, according to your led board design
      led_strip_config_t strip_config = {
        .strip_gpio_num = LED_BLINK_GPIO,
        .max_leds = LED_NUMBERS,
        .led_pixel_format = LED_PIXEL_FORMAT_GRB,
        .led_model = LED_MODEL_WS2812,
        {
          .invert_out = false,
        },
      };
    
      //-- LED strip backend configuration: RMT
      led_strip_rmt_config_t rmt_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .resolution_hz = LED_RMT_RES_HZ,
        .mem_block_symbols = 0,
        {
          .with_dma = false,
        },
      };
    
      led_indicator_strips_config_t strips_config = {
        .led_strip_cfg = strip_config,
        .led_strip_driver = LED_STRIP_RMT,
        .led_strip_rmt_cfg = rmt_config,
      };
    
      const led_indicator_config_t config = {
        .mode = LED_STRIPS_MODE,
        .led_indicator_strips_config = &strips_config,
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
      };
    
      led_indicator_handle_t led_handle_cfg = NULL;
      led_handle_cfg = led_indicator_create(&config);
      assert(led_handle_cfg != NULL);
  
      return led_handle_cfg;
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

  #if USE_ORDINARY_LED
    /**********************************************
     *                                            *
     *  1. Simple version with random intervals   *
     *                                            *
     **********************************************/
    #if LED_MODE == 1
      //-- Pseudo-random number generator (simple)
      uint32_t simple_random(void) {
        static uint32_t seed = 0;
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
      }
    
      //-- Random blinking task
      void random_blink_task(void *pvParameters) {
        int activity_counter = 0;
    
        #if USE_LED_RX_GPIO && USE_LED_TX_GPIO
          led_state_t leds[] = {
            {LED_RX_GPIO, "RX", false, 0},
            {LED_TX_GPIO, "TX", false, 0}
          };
        #elif USE_LED_RX_GPIO
          led_state_t leds[] = {
            {LED_RX_GPIO, "RX", false, 0},
            {LED_RX_GPIO, "RX", false, 0}
          };
        #else
          led_state_t leds[] = {
            {LED_TX_GPIO, "TX", false, 0},
            {LED_TX_GPIO, "TX", false, 0}
          };
        #endif
    
        //-- LED initialization
        for (int i = 0; i < 2; i++) {
          gpio_reset_pin(leds[i].gpio);
          gpio_set_direction(leds[i].gpio, GPIO_MODE_OUTPUT);
          gpio_set_level(leds[i].gpio, isLEDInverseBlinking ? 1 : 0);
          leds[i].next_change = xTaskGetTickCount() + pdMS_TO_TICKS(100 + simple_random() % 500);
        }
        #if DEBUG_MODE
          printf("LEDs initialized: RX-GPIO%d, TX-GPIO%d\n", LED_RX_GPIO, LED_TX_GPIO);
          printf("Random UART activity simulation started\n");
        #endif
      
        while (1) {
          TickType_t current_time = xTaskGetTickCount();
          bool any_activity = false;
        
          for (int i = 0; i < 2; i++) {
            if (current_time >= leds[i].next_change) {
              //-- Random change of state
              leds[i].state = !leds[i].state;
              gpio_set_level(leds[i].gpio, isLEDInverseBlinking ? !leds[i].state : leds[i].state);
            
              //-- Random interval until next change
              uint32_t delay_ms;
              if (leds[i].state) {
                //-- LED on - short time (packet simulation)
                delay_ms = 20 + (simple_random() % 80); // 20-100ms
              } else {
                //-- LED off - random time until next packet
                delay_ms = 100 + (simple_random() % 2000); // 100-2100ms
              }
            
              leds[i].next_change = current_time + pdMS_TO_TICKS(delay_ms);
              any_activity = true;
            
              #if DEBUG_MODE
                if (leds[i].state) {
                  printf("%s activity: ON for %lims\n", leds[i].name, delay_ms);
                }
              #endif
            }
          }
        
          //-- Activity counter
          if (any_activity) {
            activity_counter++;
            #if DEBUG_MODE
              if (activity_counter % 10 == 0) {
                printf("Activity summary: %d events\n", activity_counter);
              }
            #endif
          }
        
          vTaskDelay(10 / portTICK_PERIOD_MS);
        }
      }
    #endif //-- LED_MODE == 1
  
    /**********************************************
     *                                            *
     *  2. Realistic version with UART patterns   *
     *                                            *
     **********************************************/
    #if LED_MODE == 2
      void simulate_uart_activity(void *aContext)
      {
        #if USE_LED_RX_GPIO
          //-- Activity patterns (data packets)
          const uint32_t rx_patterns[] = {
            0b10101010, //-- Short package
            0b11110000, //-- Medium package
            0b11111111, //-- Long package
            0b11001100  //-- Interleaved packet
          };
          
          int rx_active = 0;
          uint32_t rx_pattern = 0;
          int rx_bit = 0;
          
          gpio_reset_pin(LED_RX_GPIO);
          gpio_set_direction(LED_RX_GPIO, GPIO_MODE_OUTPUT);
        #endif
      
        #if USE_LED_TX_GPIO
          const uint32_t tx_patterns[] = {
            0b01010101,
            0b00001111, 
            0b11100111,
            0b00110011
          };
          
          int tx_active = 0;
          uint32_t tx_pattern = 0;
          int tx_bit = 0;
          
          gpio_reset_pin(LED_TX_GPIO);
          gpio_set_direction(LED_TX_GPIO, GPIO_MODE_OUTPUT);
        #endif
      
        #if DEBUG_MODE
          printf("UART pattern simulation started\n");
        #endif
      
        while (1) {
          //-- Randomly deciding whether to start a new "package"
          #if USE_LED_RX_GPIO
          if (!rx_active && (esp_random() % 100 < 15)) { // 15% chance
            rx_active = 1;
            rx_pattern = rx_patterns[esp_random() % 4];
            rx_bit = 0;
            #if DEBUG_MODE
              printf("RX packet started: 0x%li\n", rx_pattern);
            #endif
          }
          #endif
      
          #if USE_LED_TX_GPIO
          if (!tx_active && (esp_random() % 100 < 10)) { // 10% chance
            tx_active = 1;
            tx_pattern = tx_patterns[esp_random() % 4];
            tx_bit = 0;
            #if DEBUG_MODE
              printf("TX packet started: 0x%li\n", tx_pattern);
            #endif
          }
          #endif
      
          //-- Processing current packet bits
          #if USE_LED_RX_GPIO
          if (rx_active) {
            bool bit_state = (rx_pattern >> rx_bit) & 1;
            gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? !bit_state : bit_state);
            rx_bit++;
          
            if (rx_bit >= 8) {
              rx_active = 0;
              gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
              #if DEBUG_MODE
                printf("RX packet completed\n");
              #endif
            }
          }
          #endif
        
          #if USE_LED_TX_GPIO
          if (tx_active) {
            bool bit_state = (tx_pattern >> tx_bit) & 1;
            gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? !bit_state : bit_state);
            tx_bit++;
          
            if (tx_bit >= 8) {
              tx_active = 0;
              gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
              #if DEBUG_MODE
                printf("TX packet completed\n");
              #endif
            }
          }
          #endif
      
          //-- Bit Delay (UART Speed ​​Simulation)
          vTaskDelay(50 / portTICK_PERIOD_MS);
        }
      }
    #endif //-- LED_MODE == 2
  
  
    /************************************************
     *                                              *
     *   3. Version with different activity modes   *
     *                                              *
     ************************************************/
    #if LED_MODE == 3
      /*
      typedef enum {
        MODE_IDLE,    //-- Rare single flashes
        MODE_ACTIVE,  //-- Frequent packages
        MODE_BURST,   //-- Packages in packs
        MODE_ERROR    //-- Fast blinking (error)
      } activity_mode_t;
      */
      
      activity_mode_t current_mode = MODE_IDLE;
    
      void set_mode(activity_mode_t mode) {
        if (current_mode != mode) {
          current_mode = mode;
          #if DEBUG_MODE
            const char* mode_names[] = {"IDLE", "ACTIVE", "BURST", "ERROR"};
            printf("Mode changed to: %s\n", mode_names[mode]);
          #endif
        }
      }
    
      void uart_simulation_task(void *pvParameters) {
        int counter = 0;
      
        #if USE_LED_RX_GPIO
          gpio_reset_pin(LED_RX_GPIO);
          gpio_set_direction(LED_RX_GPIO, GPIO_MODE_OUTPUT);
        #endif
        
        #if USE_LED_TX_GPIO
          gpio_reset_pin(LED_TX_GPIO);
          gpio_set_direction(LED_TX_GPIO, GPIO_MODE_OUTPUT);
        #endif
    
        #if DEBUG_MODE
          printf("Advanced UART simulation started\n");
        #endif
      
        while (1) {
          counter++;
        
          //-- Changing modes every 30 seconds
          activity_mode_t new_mode = (activity_mode_t)((counter / 300) % 4);
          set_mode(new_mode);
        
          switch (current_mode) {
            case MODE_IDLE:
              //-- Rare single packets
              #if USE_LED_RX_GPIO
                if (counter % 100 == 0) {
                  gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 0 : 1);
                  vTaskDelay(30 / portTICK_PERIOD_MS);
                  gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
                  #if DEBUG_MODE
                    printf("IDLE: Single RX packet\n");
                  #endif
                }
              #endif
              
              #if USE_LED_TX_GPIO
                if (counter % 150 == 0) {
                  gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 0 : 1);
                  vTaskDelay(30 / portTICK_PERIOD_MS);
                  gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
                  #if DEBUG_MODE
                    printf("IDLE: Single TX packet\n");
                  #endif
                }
              #endif
    
              break;
            
            case MODE_ACTIVE:
              //-- Regular traffic
              #if USE_LED_RX_GPIO
                if (counter % 20 == 0) {
                  gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 0 : 1);
                  vTaskDelay(50 / portTICK_PERIOD_MS);
                  gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
                }
              #endif
              
              #if USE_LED_TX_GPIO
                if (counter % 25 == 0) {
                  gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 0 : 1);
                  vTaskDelay(50 / portTICK_PERIOD_MS);
                  gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
                }
              #endif
              
              break;
            
            case MODE_BURST:
              //-- Packages in packs
              #if USE_LED_RX_GPIO
                if (counter % 50 == 0) {
                  #if DEBUG_MODE
                    printf("BURST: RX packet burst\n");
                  #endif
                  for (int i = 0; i < 5; i++) {
                    gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 0 : 1);
                    vTaskDelay(20 / portTICK_PERIOD_MS);
                    gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
                    vTaskDelay(10 / portTICK_PERIOD_MS);
                  }
                }
              #endif
    
              #if USE_LED_TX_GPIO
                if (counter % 60 == 0) {
                  #if DEBUG_MODE
                    printf("BURST: TX packet burst\n");
                  #endif
                  for (int i = 0; i < 3; i++) {
                    gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 0 : 1);
                    vTaskDelay(25 / portTICK_PERIOD_MS);
                    gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
                    vTaskDelay(15 / portTICK_PERIOD_MS);
                  }
                }
              #endif
              
              break;
            
            case MODE_ERROR:
              //-- Fast blinking (error)
              #if USE_LED_RX_GPIO
                gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? !((counter / 2) % 2) : (counter / 2) % 2);
              #endif
    
              #if USE_LED_TX_GPIO
                gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? !((counter / 3) % 2) : (counter / 3) % 2);
              #endif
              
              break;
          }
        
          vTaskDelay(100 / portTICK_PERIOD_MS);
        }
      }
    #endif //-- LED_MODE == 3
  
  #endif //-- USE_ORDINARY_LED

#endif //-- USE_DRIVER_LED_INDICATOR
