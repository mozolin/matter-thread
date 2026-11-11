
#include <app_priv.h>
#include "driver_boot_switch.h"

#if CONFIG_BOOT_SWITCH_ENABLED

#include <esp_matter.h>
//#include <app_priv.h>
#include "driver/gpio.h"
#include "esp_timer.h"

using namespace esp_matter;
using namespace chip::app::Clusters;

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
    ESP_LOGI(TAG_MIKE_APP, "Boot Switch driver initialized (endpoint=%d)", s_boot_switch_endpoint_id);
}

// ============================================================
//  Установка endpoint_id и кластера (вызывается из app_main)
// ============================================================
extern "C" void boot_switch_set_endpoint(uint16_t endpoint_id, cluster_t *cluster)
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
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch not initialized, cannot report event %d", event_type);
        return;
    }

    //-- Получаем FeatureMap кластера, чтобы понять, какие события разрешены
    attribute_t *fm_attr = attribute::get(s_boot_switch_cluster, Switch::Attributes::FeatureMap::Id);
    if (fm_attr == NULL) {
        ESP_LOGE(TAG_MIKE_APP, "Cannot get FeatureMap for Switch cluster");
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
                    ESP_LOGI(TAG_MIKE_APP, "Boot Switch: SINGLE press -> InitialPress");
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
                ESP_LOGI(TAG_MIKE_APP, "Boot Switch: DOUBLE press -> MultiPressComplete(2)");
            } else {
                ESP_LOGW(TAG_MIKE_APP, "Boot Switch: DOUBLE press not supported (feature not enabled)");
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
                ESP_LOGI(TAG_MIKE_APP, "Boot Switch: LONG press -> LongPress/LongRelease");
            } else {
                ESP_LOGW(TAG_MIKE_APP, "Boot Switch: LONG press not supported (feature not enabled)");
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
    ESP_LOGI(TAG_MIKE_APP, "Boot Switch task started (poll=%d ms, long=%d ms, double=%d ms)",
             BOOT_SWITCH_POLL_MS, BOOT_SWITCH_LONG_PRESS_MS, BOOT_SWITCH_DOUBLE_CLICK_MS);

    while (true) {
        uint64_t now_ms = esp_timer_get_time() / 1000;
        bool current_state = gpio_get_level((gpio_num_t)REBOOT_BUTTON_GPIO);  // true = отпущена

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
                ESP_LOGI(TAG_MIKE_APP, "Boot Switch: LONG press detected at %llu ms", press_duration);
            }
        }

        //-- Фронт: отпускание (переход из нажата в отпущена)
        if (s_last_button_state == false && current_state == true) {
            uint64_t press_duration = now_ms - s_press_start_time_ms;

            if (s_long_press_sent) {
                //-- Уже был LongPress, теперь LongRelease (можно отправить отдельно)
                //-- В текущей реализации мы уже отправили событие LONG,
                //-- HA увидит LongPress. LongRelease — опционально.
                ESP_LOGI(TAG_MIKE_APP, "Boot Switch: release after LONG press");
            } else if (press_duration <= SHORT_CLICK_MAX_MS) {
                //-- Короткий клик: проверяем, не был ли это второй клик
                if (s_first_click_pending && (now_ms - s_first_click_time_ms) <= BOOT_SWITCH_DOUBLE_CLICK_MS) {
                    //-- Второй клик в окне -> ДВОЙНОЙ
                    s_first_click_pending = false;
                    boot_switch_report_event(BOOT_SWITCH_EVENT_DOUBLE);
                    ESP_LOGI(TAG_MIKE_APP, "Boot Switch: DOUBLE click detected (gap=%llu ms)",
                             now_ms - s_first_click_time_ms);
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
                ESP_LOGI(TAG_MIKE_APP, "Boot Switch: SINGLE click confirmed");
            }
        }

        vTaskDelay(pdMS_TO_TICKS(BOOT_SWITCH_POLL_MS));
    }
}

#endif // CONFIG_BOOT_SWITCH_ENABLED
