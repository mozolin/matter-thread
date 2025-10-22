
//-- UART RX/TX blinking emulation
#pragma once

#include "driver/gpio.h"

#define TAG "ot_esp_rcp"

#define CMD_UART_NUM 0

//-- LED for UART activity indication
#define LED_RX_GPIO GPIO_NUM_8 //-- yellow
#define LED_TX_GPIO GPIO_NUM_13 //-- blue

#define DEBUG_MODE false

//-- 1. Simple version with random intervals
//-- 2. Realistic version with UART patterns
//-- 3. Version with different activity modes
#define LED_MODE 1

bool isLEDInverseBlinking = false;


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

  static led_state_t leds[] = {
    {LED_RX_GPIO, "RX", false, 0},
    {LED_TX_GPIO, "TX", false, 0}
  };

  //-- Pseudo-random number generator (simple)
  static uint32_t simple_random(void) {
    static uint32_t seed = 0;
    seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    return seed;
  }

  //-- Random blinking task
  static void random_blink_task(void *pvParameters) {
    int activity_counter = 0;

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
#endif



/**********************************************
 *                                            *
 *  2. Realistic version with UART patterns   *
 *                                            *
 **********************************************/
#if LED_MODE == 2
  //-- Activity patterns (data packets)
  static const uint32_t rx_patterns[] = {
    0b10101010, //-- Short package
    0b11110000, //-- Medium package
    0b11111111, //-- Long package
    0b11001100  //-- Interleaved packet
  };

  static const uint32_t tx_patterns[] = {
    0b01010101,
    0b00001111, 
    0b11100111,
    0b00110011
  };

  bool isInversed = true;

  static void simulate_uart_activity(void *aContext)
  {
    int rx_active = 0;
    int tx_active = 0;
    uint32_t rx_pattern = 0;
    uint32_t tx_pattern = 0;
    int rx_bit = 0;
    int tx_bit = 0;
  
    gpio_reset_pin(LED_RX_GPIO);
    gpio_set_direction(LED_RX_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_TX_GPIO);
    gpio_set_direction(LED_TX_GPIO, GPIO_MODE_OUTPUT);
  
    #if DEBUG_MODE
    	printf("UART pattern simulation started\n");
    #endif
  
    while (1) {
      //-- Randomly deciding whether to start a new "package"
      if (!rx_active && (esp_random() % 100 < 15)) { // 15% chance
        rx_active = 1;
        rx_pattern = rx_patterns[esp_random() % 4];
        rx_bit = 0;
        #if DEBUG_MODE
        	printf("RX packet started: 0x%li\n", rx_pattern);
        #endif
      }
    
      if (!tx_active && (esp_random() % 100 < 10)) { // 10% chance
        tx_active = 1;
        tx_pattern = tx_patterns[esp_random() % 4];
        tx_bit = 0;
        #if DEBUG_MODE
        	printf("TX packet started: 0x%li\n", tx_pattern);
        #endif
      }
    
      //-- Processing current packet bits
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
    
      //-- Bit Delay (UART Speed ​​Simulation)
      vTaskDelay(50 / portTICK_PERIOD_MS);
    }
  }
#endif


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

  static activity_mode_t current_mode = MODE_IDLE;

  static void set_mode(activity_mode_t mode) {
    if (current_mode != mode) {
      current_mode = mode;
      #if DEBUG_MODE
		    const char* mode_names[] = {"IDLE", "ACTIVE", "BURST", "ERROR"};
    	  printf("Mode changed to: %s\n", mode_names[mode]);
      #endif
    }
  }

  static void uart_simulation_task(void *pvParameters) {
    int counter = 0;
  
    gpio_reset_pin(LED_RX_GPIO);
    gpio_set_direction(LED_RX_GPIO, GPIO_MODE_OUTPUT);
    gpio_reset_pin(LED_TX_GPIO);
    gpio_set_direction(LED_TX_GPIO, GPIO_MODE_OUTPUT);

    #if DEBUG_MODE
    	printf("Advanced UART simulation started\n");
    #endif
  
    while (1) {
      counter++;
    
      //-- Changing modes every 30 seconds
      activity_mode_t new_mode = (counter / 300) % 4;
      set_mode(new_mode);
    
      switch (current_mode) {
        case MODE_IDLE:
          //-- Rare single packets
          if (counter % 100 == 0) {
            gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 0 : 1);
            vTaskDelay(30 / portTICK_PERIOD_MS);
            gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
            #if DEBUG_MODE
            	printf("IDLE: Single RX packet\n");
            #endif
          }
          if (counter % 150 == 0) {
            gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 0 : 1);
            vTaskDelay(30 / portTICK_PERIOD_MS);
            gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
            #if DEBUG_MODE
            	printf("IDLE: Single TX packet\n");
            #endif
          }
          break;
        
        case MODE_ACTIVE:
          //-- Regular traffic
          if (counter % 20 == 0) {
            gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 0 : 1);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? 1 : 0);
          }
          if (counter % 25 == 0) {
            gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 0 : 1);
            vTaskDelay(50 / portTICK_PERIOD_MS);
            gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? 1 : 0);
          }
          break;
        
        case MODE_BURST:
          //-- Packages in packs
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
          break;
        
        case MODE_ERROR:
          //-- Fast blinking (error)
          gpio_set_level(LED_RX_GPIO, isLEDInverseBlinking ? !((counter / 2) % 2) : (counter / 2) % 2);
          gpio_set_level(LED_TX_GPIO, isLEDInverseBlinking ? !((counter / 3) % 2) : (counter / 3) % 2);
          break;
      }
    
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }
#endif
