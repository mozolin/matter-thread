
#include <esp_matter.h>
#include <app_priv.h>
#include "driver_reset_button.h"
#include "driver/gpio.h"

using namespace esp_matter;

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

  while(1) {
    //-- Reset button pressed
    if(gpio_get_level((gpio_num_t)REBOOT_BUTTON_GPIO) == 0) {
      if(!button_pressed) {
        button_pressed_time = esp_log_timestamp();
        button_pressed = true;
        //ESP_LOGI("RebootButton", "Button pressed");
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
        } else if(pressed_duration >= REBOOT_HOLD_TIME_MS) {
          ESP_LOGW("Reset Button", "************************");
          ESP_LOGW("Reset Button", "*                      *");
          ESP_LOGW("Reset Button", "*   Reboot triggered   *");
          ESP_LOGW("Reset Button", "*                      *");
          ESP_LOGW("Reset Button", "************************");
          vTaskDelay(pdMS_TO_TICKS(5000));
          esp_restart();
        } else {
        	ESP_LOGW("Reset Button", "****************************");
          ESP_LOGW("Reset Button", "*                          *");
          ESP_LOGW("Reset Button", "*   Reset button pressed   *");
          ESP_LOGW("Reset Button", "*                          *");
          ESP_LOGW("Reset Button", "****************************");
          //-- Get Matter node
          //esp_matter::node_t *node = node::get();
          node_t *node = node::get();
          if(node) {
          	//-- Device structure log
	          //log_device_structure(node);
          	//vTaskDelay(pdMS_TO_TICKS(10000));
          }
        }
      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
