
#include <esp_matter.h>
#include <app_priv.h>
#include "driver_reboot_button.h"
#include "driver/gpio.h"

using namespace esp_matter;


//=== ДОБАВЛЕНО: esp_timer для отложенной обработки одиночного клика ===
#include "esp_timer.h"

//-- Флаг: был ли уже первый клик (ожидаем возможный второй)
static bool s_first_click_pending = false;
//-- Хэндл одноразового таймера, который "срабатывает", если второго клика не последовало
static esp_timer_handle_t s_single_click_timer = NULL;
//=== /ДОБАВЛЕНО ===

//=== ДОБАВЛЕНО: обработчики одиночного и двойного клика ===

//-- Колбэк esp_timer: вызывается, если в течение DOUBLE_CLICK_INTERVAL_MS
//-- второго нажатия не произошло. Значит это был одиночный клик.
static void single_click_timer_cb(void *arg)
{
  (void)arg;
  //-- Сбрасываем признак ожидания второго клика
  s_first_click_pending = false;

  //-- Выполняем действие одиночного клика: переключаем SSD1306 (как было раньше)
  #if CONFIG_SSD1306_ENABLED
    if(ssd1306_initialized) {
      ssd1306_set_display_enabled(!ssd1306_display_enabled);
    } else {
      ESP_LOGW("Reset Button", "SSD1306 not initialized, skip toggle");
    }
  #endif
}

//-- Публичная обёртка, объявленная в заголовке (может пригодиться другим модулям)
void reset_button_handle_single_click(void)
{
  single_click_timer_cb(NULL);
}

//-- Инициализация одноразового таймера. Вызывается один раз при старте задачи.
void single_click_timer_init(void)
{
  if(s_single_click_timer != NULL) {
    return; //-- уже создан
  }
  const esp_timer_create_args_t args = {
    .callback = &single_click_timer_cb,
    .arg      = NULL,
    .dispatch_method = ESP_TIMER_TASK,
    .name     = "reset_single_click",
    .skip_unhandled_events = true,
  };
  ESP_ERROR_CHECK(esp_timer_create(&args, &s_single_click_timer));
}
//=== /ДОБАВЛЕНО ===

void reboot_button_task(void *pvParameter)
{
  #if CONFIG_REBOOT_BUTTON_ENABLED
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
  
  //=== ДОБАВЛЕНО: создаём esp_timer для отложенного одиночного клика ===
  single_click_timer_init();
  //=== /ДОБАВЛЕНО ===

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
    //=== ДОБАВЛЕНО ===
    
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
        	
          
          //=== ИЗМЕНЕНО: различаем одиночный и двойной клик ===
                    //
                    // Логика:
                    //   • Если это первый короткий клик — запускаем одноразовый esp_timer
                    //     на DOUBLE_CLICK_INTERVAL_MS. Если за это время второго клика
                    //     не будет — таймер вызовет single_click_timer_cb() и выполнит
                    //     действие одиночного клика (переключение SSD1306).
                    //   • Если во время окна приходит второй клик — останавливаем таймер
                    //     и выполняем действие двойного клика (переключение блокировки LED).
                    //
                    if(pressed_duration <= SHORT_CLICK_MAX_MS) {
                      if(!s_first_click_pending) {
                        ESP_LOGW("Reset Button", "****************************");
                        ESP_LOGW("Reset Button", "*                          *");
                        ESP_LOGW("Reset Button", "*   Reset button pressed   *");
                        ESP_LOGW("Reset Button", "*                          *");
                        ESP_LOGW("Reset Button", "****************************");
                        //-- Первый клик в окне: стартуем таймер ожидания второго
                        s_first_click_pending = true;
                        esp_timer_stop(s_single_click_timer);
                        esp_timer_start_once(s_single_click_timer, (uint64_t)DOUBLE_CLICK_INTERVAL_MS * 1000ULL);
                      } else {
                        ESP_LOGW("Reset Button", "*********************************");
                        ESP_LOGW("Reset Button", "*                               *");
                        ESP_LOGW("Reset Button", "*   Reset button double click   *");
                        ESP_LOGW("Reset Button", "*                               *");
                        ESP_LOGW("Reset Button", "*********************************");
                        //-- Второй клик в окне: отменяем одиночное действие и делаем двойное
                        s_first_click_pending = false;
                        esp_timer_stop(s_single_click_timer);
                        #if USE_DRIVER_LED_INDICATOR
                          toggle_led_indicator_lock();
                        #endif
                      }
                    } else {
                      //-- Долгое нажатие (но меньше REBOOT_HOLD_TIME_MS) — не клик,
                      //-- игнорируем, чтобы не сбивать окно двойного клика.
                      ESP_LOGD("Reset Button", "Long-ish press (%lu ms), ignored",
                               (unsigned long)pressed_duration);
                    }
                    //=== /ИЗМЕНЕНО ===
        }
      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  #endif
}
