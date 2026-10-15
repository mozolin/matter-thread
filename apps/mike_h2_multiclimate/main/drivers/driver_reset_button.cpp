
#include <esp_matter.h>
#include <app_priv.h>
#include "driver_reset_button.h"
#include "driver/gpio.h"

using namespace esp_matter;

//=== ДОБАВЛЕНО: работа с SSD1306 ===
#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306.h"
#endif
//=== /ДОБАВЛЕНО ===

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
    //=== ДОБАВЛЕНО: авто-выключение экрана через 5 минут ===
    #if CONFIG_SSD1306_ENABLED
      if(ssd1306_display_enabled && ssd1306_last_on_time_ms != 0) {
        uint32_t now_ms = esp_log_timestamp();
        if((now_ms - ssd1306_last_on_time_ms) >= SSD1306_AUTO_OFF_TIMEOUT_MS) {
          ESP_LOGW("Reset Button", "SSD1306 auto-off after 5 min");
          ssd1306_set_display_enabled(false);
        } else {
          //=== ДОБАВЛЕНО: пока экран ещё включён — рисуем оставшееся время ===
          ssd1306_update_timeout_line();
          //=== /ДОБАВЛЕНО ===
        }
      }
    #endif
    //=== /ДОБАВЛЕНО ===
    
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
          
          //=== ИЗМЕНЕНО: короткое нажатие переключает экран ===
          // Было: только вывод структуры Matter-узла.
          // Стало: переключаем дисплей SSD1306 (ON <-> OFF).
          #if CONFIG_SSD1306_ENABLED
            if(ssd1306_initialized) {
              // инвертируем текущее состояние
              ssd1306_set_display_enabled(!ssd1306_display_enabled);
            } else {
              ESP_LOGW("Reset Button", "SSD1306 not initialized, skip toggle");
            }
          #endif
          //=== /ИЗМЕНЕНО ===
          
          /*
          //-- Get Matter node
          node_t *node = node::get();
          if(node) {
            ESP_LOGI("Reset Button", "Matter Node created!");
          	//-- Device structure log
	          log_device_structure(node);
          	//vTaskDelay(pdMS_TO_TICKS(10000));
          } else {
            ESP_LOGE("Reset Button", "Matter Node not created!");
          }
          */
        }
      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}
