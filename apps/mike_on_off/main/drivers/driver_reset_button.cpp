
#include "driver_reset_button.h"
#include "driver_relay.h"
#include "driver/gpio.h"

/********************
 *                  *
 *   RESET BUTTON   *
 *                  *
 ********************/
void reboot_button_task(void *pvParameter)
{
  uint32_t button_pressed_time = 0;
  bool button_pressed = false;

  //-- Configure Reset GPIO
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << (gpio_num_t)REBOOT_BUTTON_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  bool state = false;

  while(1) {
    //-- Reset button pressed
    if(gpio_get_level((gpio_num_t)REBOOT_BUTTON_GPIO) == 0) {
      if(!button_pressed) {
        button_pressed_time = esp_log_timestamp();
        button_pressed = true;
        //ESP_LOGI("RebootButton", "Button pressed");

        state = !state;

        relay_set_on_off(5, state);
				vTaskDelay(pdMS_TO_TICKS(5000));
	      
				bool s1 = load_relay_state(1);
				bool s2 = load_relay_state(2);
				bool s3 = load_relay_state(3);
				bool s4 = load_relay_state(4);
				bool s5 = load_relay_state(5);
				bool s6 = load_relay_state(6);
				bool s7 = load_relay_state(7);
				bool s8 = load_relay_state(8);
	      
	      ESP_LOGW("Reset Button", "ENDPNT: 1|2|3|4|5|6|7|8");
        ESP_LOGW("Reset Button", "STATES: %d|%d|%d|%d|%d|%d|%d|%d", s1, s2, s3, s4, s5, s6, s7, s8);

      }
    } else {
      //-- Reset button released
      if(button_pressed) {
        //ESP_LOGI("RebootButton", "Button released");
        uint32_t pressed_duration = esp_log_timestamp() - button_pressed_time;
        
        if(pressed_duration >= FACTORY_RESET_HOLD_TIME_MS) {
          ESP_LOGE("Reset Button", "*******************************");
          ESP_LOGE("Reset Button", "*                             *");
          ESP_LOGE("Reset Button", "*   Factory reset triggered   *");
          ESP_LOGE("Reset Button", "*                             *");
          ESP_LOGE("Reset Button", "*******************************");
          vTaskDelay(pdMS_TO_TICKS(5000));
          esp_matter::factory_reset();
          esp_restart();
        } else if (pressed_duration >= REBOOT_HOLD_TIME_MS) {
          ESP_LOGW("Reset Button", "************************");
          ESP_LOGW("Reset Button", "*                      *");
          ESP_LOGW("Reset Button", "*   Reboot triggered   *");
          ESP_LOGW("Reset Button", "*                      *");
          ESP_LOGW("Reset Button", "************************");
          vTaskDelay(pdMS_TO_TICKS(5000));
          esp_restart();
        }

      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
