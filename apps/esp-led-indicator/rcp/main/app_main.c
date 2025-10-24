
#include <stdio.h>
#include <unistd.h>

#include "esp_log.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_openthread.h"
#include "app_priv.h"
#include "esp_vfs_eventfd.h"
#include "driver/uart.h"

#if CONFIG_EXTERNAL_COEX_ENABLE
  #include "esp_coexist.h"
#endif

#if !SOC_IEEE802154_SUPPORTED
  #error "RCP is only supported for the SoCs which have IEEE 802.15.4 module"
#endif

#define TAG_H2 "H2 RCP"

#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  led_indicator_handle_t led_handle;
#endif

extern void otAppNcpInit(otInstance *instance);

#if CONFIG_EXTERNAL_COEX_ENABLE
  #if SOC_EXTERNAL_COEX_ADVANCE
  static void ot_external_coexist_init(void)
  {
    esp_external_coex_gpio_set_t gpio_pin = ESP_OPENTHREAD_DEFAULT_EXTERNAL_COEX_CONFIG();
    esp_external_coex_set_work_mode(EXTERNAL_COEX_FOLLOWER_ROLE);
    ESP_ERROR_CHECK(esp_enable_extern_coex_gpio_pin(CONFIG_EXTERNAL_COEX_WIRE_TYPE, gpio_pin));
  }
  #endif // SOC_EXTERNAL_COEX_ADVANCE
#endif // CONFIG_EXTERNAL_COEX_ENABLE

static void ot_task_worker(void *aContext)
{
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };

    // Initialize the OpenThread stack
    ESP_ERROR_CHECK(esp_openthread_init(&config));

    #if CONFIG_EXTERNAL_COEX_ENABLE
        ot_external_coexist_init();
    #endif // CONFIG_EXTERNAL_COEX_ENABLE

    // Initialize the OpenThread ncp
    otAppNcpInit(esp_openthread_get_instance());

    // Run the main loop
    esp_openthread_launch_mainloop();

    // Clean up
    esp_vfs_eventfd_unregister();
    vTaskDelete(NULL);
}

void app_main(void)
{
  esp_vfs_eventfd_config_t eventfd_config = {
      .max_fds = 2,
  };

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());
  ESP_ERROR_CHECK(esp_vfs_eventfd_register(&eventfd_config));
  xTaskCreate(ot_task_worker, "ot_rcp_main", 3072, xTaskGetCurrentTaskHandle(), 5, NULL);

  //ESP_LOGW(TAG_H2, "~~~ USE_DRIVER_LED_INDICATOR:%d, USE_ORDINARY_LED:%d, LED_MODE:%d", USE_DRIVER_LED_INDICATOR, USE_ORDINARY_LED, LED_MODE);
  
  #if USE_DRIVER_LED_INDICATOR
    //-- at least one LED must not be an RGB LED
    #if USE_ORDINARY_LED
      //--> UART RX/TX Blinking Simulator
      #if LED_MODE == 1
        #if DEBUG_MODE
          ESP_LOGW(TAG_H2, "~~~ BLINK: 1. Simple version with random intervals");
        #endif
        xTaskCreate(random_blink_task, "uart_sim", 4096, NULL, 1, NULL);
      #endif
    
      #if LED_MODE == 2
        #if DEBUG_MODE
          ESP_LOGW(TAG_H2, "~~~ BLINK: 2. Realistic version with UART patterns");
        #endif
        xTaskCreate(simulate_uart_activity, "uart_pattern", 4096, NULL, 1, NULL);
      #endif
    
      #if LED_MODE == 3
        #if DEBUG_MODE
           ESP_LOGW(TAG_H2, "~~~ BLINK: 3. Version with different activity modes");
         #endif
         xTaskCreate(uart_simulation_task, "uart_sim", 4096, NULL, 1, NULL);
      #endif
      //<-- UART RX/TX Blinking Simulator
    #endif //-- USE_ORDINARY_LED
  
    //-- at least one LED must be an RGB LED
    #if USE_RGB_LED
      //--> RGB LED indicator
      led_handle = configure_indicator();
      while(1) {
        //ESP_LOGW(TAG_H2, "~~~ BLINK RGB: BLINK_ONCE_RED");
        get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
      }
      //<-- RGB LED indicator
    #endif //-- USE_RGB_LED
  #endif //-- USE_DRIVER_LED_INDICATOR
    
}
