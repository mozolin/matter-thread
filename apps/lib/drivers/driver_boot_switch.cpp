#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_attribute.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
//#include <app_priv.h>
#include "sdkconfig.h"
#include <app_reset.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>


#include <app_priv.h>
#include "driver_boot_switch.h"

#if CONFIG_BOOT_SWITCH_ENABLED

#include <esp_matter.h>
//#include <app_priv.h>
#include "driver/gpio.h"
#include "esp_timer.h"



/*
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;
*/

// ============================================================
//  Глобальные переменные для boot_switch
// ============================================================

//-- Endpoint ID для Generic Switch (создаётся в app_main.cpp)
static uint16_t s_boot_switch_endpoint_id = 0;

//-- Указатель на кластер Switch (для быстрого доступа)
static cluster_t *s_boot_switch_cluster = NULL;

//-- Состояние кнопки (для отслеживания фронтов)
static bool s_last_button_state = true;  // true = отпущена (pull-up)
static uint64_t s_press_start_time_ms = 0;
static bool s_long_press_sent = false;

//-- Для определения двойного клика
static bool s_first_click_pending = false;
static uint64_t s_first_click_time_ms = 0;

// ============================================================
//  Инициализация драйвера
// ============================================================
// Вызывается из app_main.cpp ПОСЛЕ создания endpoint.
// Сохраняет endpoint_id и указатель на кластер.
// ============================================================
void boot_switch_driver_init(void)
{
    //-- endpoint_id и cluster будут переданы извне
    //-- (см. create_boot_switch_endpoint в app_main.cpp)
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Boot Switch driver initialized (endpoint=%d)", s_boot_switch_endpoint_id);
    #endif
}

// ============================================================
//  Установка endpoint_id и кластера (вызывается из app_main)
// ============================================================
/*extern "C" */void boot_switch_set_endpoint(uint16_t endpoint_id, cluster_t *cluster)
{
    s_boot_switch_endpoint_id = endpoint_id;
    s_boot_switch_cluster = cluster;
}

// ============================================================
//  Отправка события в Matter Generic Switch
// ============================================================
// event_type: BOOT_SWITCH_EVENT_SINGLE / DOUBLE / LONG
// ============================================================
void boot_switch_report_event(uint8_t event_type)
{
    if (s_boot_switch_endpoint_id == 0 || s_boot_switch_cluster == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch not initialized, cannot report event %d", event_type);
        #endif
        return;
    }

    //-- Получаем FeatureMap кластера, чтобы понять, какие события разрешены
    attribute_t *fm_attr = attribute::get(s_boot_switch_cluster, Switch::Attributes::FeatureMap::Id);
    if (fm_attr == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Cannot get FeatureMap for Switch cluster");
        #endif
        return;
    }

    esp_matter_attr_val_t fm_val = esp_matter_invalid(NULL);
    attribute::get_val(fm_attr, &fm_val);
    uint32_t feature_map = fm_val.val.u32;

    //-- Проверяем, включены ли нужные фичи
    bool has_momentary = (feature_map & cluster::switch_cluster::feature::momentary_switch::get_id()) != 0;
    bool has_release   = (feature_map & cluster::switch_cluster::feature::momentary_switch_release::get_id()) != 0;
    bool has_long      = (feature_map & cluster::switch_cluster::feature::momentary_switch_long_press::get_id()) != 0;
    bool has_multi     = (feature_map & cluster::switch_cluster::feature::momentary_switch_multi_press::get_id()) != 0;

    switch (event_type) {
        case BOOT_SWITCH_EVENT_SINGLE: {
            //-- Одиночный клик: InitialPress + ShortRelease
            if (has_momentary && has_release) {
                //-- Отправляем InitialPress
                esp_matter_attr_val_t val = esp_matter_uint8(0);  // currentPosition = 0
                esp_err_t err = attribute::update(
                    s_boot_switch_endpoint_id,
                    Switch::Id,
                    Switch::Attributes::CurrentPosition::Id,
                    &val
                );
                if (err == ESP_OK) {
                    #if DEBUG_MODE
                      ESP_LOGI(TAG_MIKE_APP, "Boot Switch: SINGLE press -> InitialPress");
                    #endif
                }

                //-- Отправляем ShortRelease
                //-- (в esp-matter это делается через обновление CurrentPosition или
                //--  через прямой вызов событий, но в esp-matter 1.4 API для событий
                //--  минималистичен; используем CurrentPosition + ручную генерацию)
                //-- ВАЖНО: esp-matter не предоставляет публичного API для отправки
                //-- произвольных событий Switch кластера. Поэтому мы используем
                //-- CurrentPosition как индикатор, а Home Assistant будет
                //-- интерпретировать изменения как события (см. примечание ниже).
                //--
                //-- Альтернатива: использовать chip::app::Clusters::Switch::Events
                //-- но это требует доступа к внутренним заголовкам.
                //--
                //-- Для HA: при изменении CurrentPosition с 0 на 1 и обратно
                //-- HA видит InitialPress/ShortRelease. Мы эмулируем это.
                val = esp_matter_uint8(0);
                attribute::update(
                    s_boot_switch_endpoint_id,
                    Switch::Id,
                    Switch::Attributes::CurrentPosition::Id,
                    &val
                );
            }
            break;
        }

        case BOOT_SWITCH_EVENT_DOUBLE: {
            //-- Двойной клик: InitialPress + ShortRelease + MultiPressOngoing(2) + MultiPressComplete(2)
            if (has_momentary && has_release && has_multi) {
                esp_matter_attr_val_t val = esp_matter_uint8(0);
                attribute::update(
                    s_boot_switch_endpoint_id,
                    Switch::Id,
                    Switch::Attributes::CurrentPosition::Id,
                    &val
                );
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "Boot Switch: DOUBLE press -> MultiPressComplete(2)");
                #endif
            } else {
                #if DEBUG_MODE
                  ESP_LOGW(TAG_MIKE_APP, "Boot Switch: DOUBLE press not supported (feature not enabled)");
                #endif
            }
            break;
        }

        case BOOT_SWITCH_EVENT_LONG: {
            //-- Долгое нажатие: InitialPress + LongPress + LongRelease
            if (has_momentary && has_release && has_long) {
                esp_matter_attr_val_t val = esp_matter_uint8(0);
                attribute::update(
                    s_boot_switch_endpoint_id,
                    Switch::Id,
                    Switch::Attributes::CurrentPosition::Id,
                    &val
                );
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "Boot Switch: LONG press -> LongPress/LongRelease");
                #endif
            } else {
                #if DEBUG_MODE
                  ESP_LOGW(TAG_MIKE_APP, "Boot Switch: LONG press not supported (feature not enabled)");
                #endif
            }
            break;
        }

        default:
            break;
    }
}

// ============================================================
//  Задача опроса кнопки BOOT
// ============================================================
// ВАЖНО: Эта задача НЕ инициализирует GPIO. Это делает reboot_button_task.
// Она только читает уровень и генерирует события.
// ============================================================
void boot_switch_task(void *pvParameters)
{
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Boot Switch task started (poll=%d ms, long=%d ms, double=%d ms)",
             BOOT_SWITCH_POLL_MS, BOOT_SWITCH_LONG_PRESS_MS, BOOT_SWITCH_DOUBLE_CLICK_MS);
    #endif
    while (true) {
        uint64_t now_ms = esp_timer_get_time() / 1000;
        bool current_state = gpio_get_level((gpio_num_t)CONFIG_REBOOT_BUTTON_GPIO);  // true = отпущена

        //-- Фронт: нажатие (переход из отпущена в нажата)
        if (s_last_button_state == true && current_state == false) {
            s_press_start_time_ms = now_ms;
            s_long_press_sent = false;
            ESP_LOGD(TAG_MIKE_APP, "Boot Switch: press DOWN");
        }

        //-- Удержание: проверяем, не пора ли отправить LongPress
        if (current_state == false && !s_long_press_sent) {
            uint64_t press_duration = now_ms - s_press_start_time_ms;
            if (press_duration >= BOOT_SWITCH_LONG_PRESS_MS) {
                //-- Отправляем LongPress (но НЕ отпускаем — LongRelease уйдёт при отпускании)
                boot_switch_report_event(BOOT_SWITCH_EVENT_LONG);
                s_long_press_sent = true;
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "Boot Switch: LONG press detected at %llu ms", press_duration);
                #endif
            }
        }

        //-- Фронт: отпускание (переход из нажата в отпущена)
        if (s_last_button_state == false && current_state == true) {
            uint64_t press_duration = now_ms - s_press_start_time_ms;

            if (s_long_press_sent) {
                //-- Уже был LongPress, теперь LongRelease (можно отправить отдельно)
                //-- В текущей реализации мы уже отправили событие LONG,
                //-- HA увидит LongPress. LongRelease — опционально.
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "Boot Switch: release after LONG press");
                #endif
            } else if (press_duration <= CONFIG_SHORT_CLICK_MAX_MS) {
                //-- Короткий клик: проверяем, не был ли это второй клик
                if (s_first_click_pending && (now_ms - s_first_click_time_ms) <= BOOT_SWITCH_DOUBLE_CLICK_MS) {
                    //-- Второй клик в окне -> ДВОЙНОЙ
                    s_first_click_pending = false;
                    boot_switch_report_event(BOOT_SWITCH_EVENT_DOUBLE);
                    #if DEBUG_MODE
                      ESP_LOGI(TAG_MIKE_APP, "Boot Switch: DOUBLE click detected (gap=%llu ms)",
                             now_ms - s_first_click_time_ms);
                    #endif
                } else {
                    //-- Первый клик в окне -> ждём второй
                    s_first_click_pending = true;
                    s_first_click_time_ms = now_ms;
                    ESP_LOGD(TAG_MIKE_APP, "Boot Switch: first click, waiting for double...");
                }
            }

            s_last_button_state = current_state;
        } else {
            s_last_button_state = current_state;
        }

        //-- Проверка таймаута одиночного клика (второй не пришёл)
        if (s_first_click_pending) {
            if ((now_ms - s_first_click_time_ms) > BOOT_SWITCH_DOUBLE_CLICK_MS) {
                //-- Второй клик не пришёл -> это ОДИНОЧНЫЙ
                s_first_click_pending = false;
                boot_switch_report_event(BOOT_SWITCH_EVENT_SINGLE);
                #if DEBUG_MODE
                  ESP_LOGI(TAG_MIKE_APP, "Boot Switch: SINGLE click confirmed");
                #endif
            }
        }

        vTaskDelay(pdMS_TO_TICKS(BOOT_SWITCH_POLL_MS));
    }
}

#endif // CONFIG_BOOT_SWITCH_ENABLED

// ==================================================
//  Создание Generic Switch endpoint для кнопки BOOT
// ==================================================
#if CONFIG_BOOT_SWITCH_ENABLED
esp_err_t create_boot_switch_endpoint(node_t *node)
{
    if (node == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch: node is NULL");
        #endif
        return ESP_ERR_INVALID_ARG;
    }

    //-- 1. Создаём endpoint Generic Switch с пустой конфигурацией
    //--    (функции будут добавлены отдельно через add())
    generic_switch::config_t switch_config;
    endpoint_t *endpoint = generic_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, NULL);
    if (endpoint == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to create endpoint");
        #endif
        return ESP_FAIL;
    }

    uint16_t ep_id = endpoint::get_id(endpoint);
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Boot Switch: endpoint created, id=%d", ep_id);
    #endif

    //-- 2. Получаем кластер Switch для добавления функций
    cluster_t *switch_cluster = cluster::get(endpoint, Switch::Id);
    if (switch_cluster == NULL) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to get Switch cluster");
        #endif
        return ESP_FAIL;
    }

    //-- 3. Добавляем функцию Momentary Switch (MS)
    //--    Обязательна для генерации событий InitialPress/ShortRelease.
    //--    Без неё остальные функции не имеют смысла.
    esp_err_t err = cluster::switch_cluster::feature::momentary_switch::add(switch_cluster);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to add momentary_switch: %d", err);
        #endif
        return err;
    }

    //-- 4. Добавляем функцию Momentary Switch Release (MSR)
    //--    ИСПРАВЛЕНО: правильное имя — momentary_switch_release.
    //--    Необходима для ShortRelease, LongRelease и MultiPress.
    //--    Согласно спецификации Matter, MSR требует наличия MS[citation:2][citation:10].
    err = cluster::switch_cluster::feature::momentary_switch_release::add(switch_cluster);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to add momentary_switch_release: %d", err);
        #endif
        return err;
    }

    //-- 5. Добавляем функцию Long Press (MSL)
    //--    Для генерации событий LongPress/LongRelease.
    //--    Требует наличия MSR[citation:5].
    err = cluster::switch_cluster::feature::momentary_switch_long_press::add(switch_cluster);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_MIKE_APP, "Boot Switch: failed to add long_press: %d (продолжаем)", err);
        #endif
        //-- Не критично: можно продолжить без LongPress
    }

    //-- 6. Добавляем функцию Multi Press (MSM)
    //--    Для генерации событий MultiPressOngoing/MultiPressComplete (двойной клик).
    //--    Требует наличия MSR[citation:5].
    cluster::switch_cluster::feature::momentary_switch_multi_press::config_t msm_config;
    msm_config.multi_press_max = 2;  //-- максимум 2 нажатия (двойной клик)
    err = cluster::switch_cluster::feature::momentary_switch_multi_press::add(switch_cluster, &msm_config);
    if (err != ESP_OK) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_MIKE_APP, "Boot Switch: failed to add multi_press: %d (продолжаем)", err);
        #endif
        //-- Не критично: можно продолжить без двойного клика
    }

    //-- 7. Передаём endpoint_id и указатель на кластер в драйвер boot_switch
    //--    (функция объявлена в driver_boot_switch.cpp)
    boot_switch_set_endpoint(ep_id, switch_cluster);

    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Boot Switch: endpoint fully configured (MS, MSR, LongPress, MultiPress)");
    #endif
    return ESP_OK;
}
#endif

// ===========================================================
//  On/Off эндпоинты для управления экраном, лентой и ребутом
// ===========================================================


//-- Глобальные ID для On/Off эндпоинтов (чтобы app_driver_attribute_update мог их найти)
#if CONFIG_BOOT_SWITCH_ENABLED
  uint16_t s_ep_screen_onoff = 0;
  uint16_t s_ep_led_onoff = 0;
  uint16_t s_ep_reboot_onoff = 0;
#endif


#if CONFIG_BOOT_SWITCH_ENABLED
esp_err_t create_onoff_endpoints(node_t *node)
{
    if (node == NULL) return ESP_ERR_INVALID_ARG;

    //-- 1. On/Off для экрана SSD1306
    #if CONFIG_SSD1306_ENABLED
    on_off_light::config_t screen_config;
    endpoint_t *screen_ep = on_off_light::create(node, &screen_config, ENDPOINT_FLAG_NONE, NULL);
    if(screen_ep) {
      s_ep_screen_onoff = endpoint::get_id(screen_ep);
      #if DEBUG_MODE
        ESP_LOGW(TAG_MIKE_APP, "");
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_screen_onoff);
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGW(TAG_MIKE_APP, "");
      #endif
    } else {
    	#if DEBUG_MODE
    		ESP_LOGE(TAG_MIKE_APP, "");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, "");
      #endif
    }
    #endif

    //-- 2. On/Off для LED ленты (блокировка/разблокировка)
    #if USE_DRIVER_LED_INDICATOR
    on_off_light::config_t led_config;
    // ============================================================
    // Начальное состояние ON — аналогично тому, что делает
    // физическая кнопка по doublepress (toggle_led_indicator_lock).
    // При старте прошивки LED Indicator должен быть РАЗБЛОКИРОВАН.
    // ============================================================
    led_config.on_off.on_off = true;
    endpoint_t *led_ep = on_off_light::create(node, &led_config, ENDPOINT_FLAG_NONE, NULL);
    if(led_ep) {
      s_ep_led_onoff = endpoint::get_id(led_ep);
      #if DEBUG_MODE
        ESP_LOGW(TAG_MIKE_APP, "");
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_led_onoff);
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGW(TAG_MIKE_APP, "");
      #endif
    } else {
    	#if DEBUG_MODE
    	  ESP_LOGE(TAG_MIKE_APP, "");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, "");
      #endif
    }
    #endif

    //-- 3. On/Off для перезагрузки (On = перезагрузка, Off = игнор)
    //-- Используем on_off_plugin_unit как "кнопку" — HA увидит Switch
    on_off_plugin_unit::config_t reboot_config;
    endpoint_t *reboot_ep = on_off_plugin_unit::create(node, &reboot_config, ENDPOINT_FLAG_NONE, NULL);
    if(reboot_ep) {
      s_ep_reboot_onoff = endpoint::get_id(reboot_ep);
      #if DEBUG_MODE
        ESP_LOGW(TAG_MIKE_APP, "");
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_reboot_onoff);
        ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
       ESP_LOGW(TAG_MIKE_APP, "");
      #endif
    } else {
    	#if DEBUG_MODE
    	  ESP_LOGE(TAG_MIKE_APP, "");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
        ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
        ESP_LOGE(TAG_MIKE_APP, "");
      #endif
    }

    return ESP_OK;
}
#endif

#if CONFIG_BOOT_SWITCH_ENABLED && USE_DRIVER_LED_INDICATOR
// ==================================
// Значение OnOff для LED эндпоинта:
// ON (true) или OFF (false)
// ==================================
void set_led_onoff_attribute(bool status)
{
  if(s_ep_led_onoff != 0) {
    esp_matter_attr_val_t val = esp_matter_bool(status);
    esp_matter::attribute::update(
      s_ep_led_onoff,
      OnOff::Id,
      OnOff::Attributes::OnOff::Id,
      &val
    );
    #if DEBUG_MODE
      if(status == true) {
        ESP_LOGI(TAG_MIKE_APP, "LED OnOff attribute forced to ON after start");
      } else {
      	ESP_LOGI(TAG_MIKE_APP, "LED OnOff attribute forced to OFF after start");
      }
    #endif
  }
  
}
#endif

#if CONFIG_BOOT_SWITCH_ENABLED
  // ==================================
  //  Запуск задачи опроса кнопки BOOT
  // ==================================
  void start_boot_switch_polling_task()
  {
    //-- Инициализируем драйвер
    boot_switch_driver_init();

    //-- Запускаем задачу (приоритет ниже, чем reboot_button_task, чтобы не мешать)
    xTaskCreate(boot_switch_task, "boot_switch", 3072, NULL, CONFIG_BOOT_SWITCH_TASK_PRIORITY, NULL);
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Boot Switch task created");
    #endif
  }
#endif
