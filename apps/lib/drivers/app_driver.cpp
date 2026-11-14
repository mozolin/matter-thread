#include <stdlib.h>
#include <string.h>
#include <esp_log.h>
#include <esp_matter.h>
#include <app_priv.h>

#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
#endif

using namespace chip::app::Clusters;
using namespace esp_matter;

/*
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  // ============================================================
  //  Обработка On/Off для экрана, ленты и ребута из HA
  // ============================================================
  #if CONFIG_BOOT_SWITCH_ENABLED
    //-- Проверяем, не наш ли это эндпоинт
    if (cluster_id == OnOff::Id && attribute_id == OnOff::Attributes::OnOff::Id) {
        //-- Экран
        #if CONFIG_SSD1306_ENABLED
        if (endpoint_id == s_ep_screen_onoff) {
            bool turn_on = val->val.b;
            if (ssd1306_initialized) {
                ssd1306_set_display_enabled(turn_on);
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "HA -> Screen %s", turn_on ? "ON" : "OFF");
                #endif
            }
            return ESP_OK;
        }
        #endif

        //-- LED лента (блокировка)
        #if USE_DRIVER_LED_INDICATOR
        if (endpoint_id == s_ep_led_onoff) {
            bool turn_on = val->val.b;
            //-- turn_on = true -> разблокировать (led_indicator_locked = false)
            //-- turn_on = false -> заблокировать (led_indicator_locked = true)
            extern bool led_indicator_locked;
            led_indicator_locked = !turn_on;
            #if DEBUG_MODE
              ESP_LOGI(TAG_MIKE_APP, "HA -> LED %s (locked=%d)", turn_on ? "ON" : "OFF", led_indicator_locked);
            #endif
            return ESP_OK;
        }
        #endif

        //-- Перезагрузка (On = немедленный reboot)
        if (endpoint_id == s_ep_reboot_onoff) {
            if (val->val.b) {
                #if DEBUG_MODE
                  ESP_LOGW(TAG_MIKE_APP, "HA -> Reboot requested, restarting in 1 sec...");
                #endif
                vTaskDelay(pdMS_TO_TICKS(1000));
                esp_restart();
            }
            return ESP_OK;
        }
    }
  #endif
    
  // Sensors are read-only in this implementation
  return ESP_OK;
}
*/
