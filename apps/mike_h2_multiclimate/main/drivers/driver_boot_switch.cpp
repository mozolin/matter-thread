
#include <app_priv.h>
#include "driver_boot_switch.h"

#if CONFIG_BOOT_SWITCH_ENABLED

#include <esp_log.h>
#include <esp_timer.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_matter.h>
#include <esp_matter_attribute.h>
#include <app_priv.h>

using namespace esp_matter;
using namespace chip::app::Clusters;
using namespace esp_matter::cluster;

// ============================================================
// ВНУТРЕННИЕ КОНСТАНТЫ
// ============================================================

#define BOOT_SWITCH_GPIO               9
#define BOOT_SWITCH_TASK_PERIOD_MS     20      // УВЕЛИЧЕНО с 10 до 20 мс: снижает нагрузку на планировщик
#define BOOT_SWITCH_DOUBLE_CLICK_MS    400     // окно двойного клика
#define BOOT_SWITCH_LONG_PRESS_MS      3000    // длинное нажатие = 3 сек

// ============================================================
// ВНУТРЕННЕЕ СОСТОЯНИЕ
// ============================================================

static uint16_t s_switch_endpoint_id = 0;

// Состояние кнопки
static bool s_button_pressed = false;
static uint32_t s_press_start_ms = 0;

// Детекция двойного клика
static bool s_first_click_pending = false;
static uint32_t s_first_click_time_ms = 0;

// Детекция длинного нажатия
static bool s_long_press_sent = false;

// ============================================================
// ПУБЛИЧНЫЕ ФУНКЦИИ
// ============================================================

esp_err_t boot_switch_driver_init(void)
{
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BOOT_SWITCH_GPIO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };

    esp_err_t err = gpio_config(&io_conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "Boot switch: GPIO config failed: %s", esp_err_to_name(err));
        return err;
    }

    ESP_LOGI(TAG_MIKE_APP, "Boot switch driver initialized on GPIO %d", BOOT_SWITCH_GPIO);
    return ESP_OK;
}

uint16_t boot_switch_create_endpoint(esp_matter::node_t *node)
{
    if (node == NULL) {
        return 0;
    }

    // ============================================================
    // Конфигурация Generic Switch endpoint
    // Feature flags соответствуют требованиям Matter Spec:
    // MS (MomentarySwitch) — обязательно для InitialPress
    // MSR (MomentarySwitchRelease) — для ShortRelease/LongRelease
    // MSL (MomentarySwitchLongPress) — для LongPress/LongRelease
    // MSM (MomentarySwitchMultiPress) — для MultiPressOngoing/Complete
    // ============================================================
    generic_switch::config_t switch_config;

    switch_config.switch_cluster.feature_flags =
        cluster::switch_cluster::feature::momentary_switch::get_id() |
        cluster::switch_cluster::feature::momentary_switch_release::get_id() |
        cluster::switch_cluster::feature::momentary_switch_long_press::get_id() |
        cluster::switch_cluster::feature::momentary_switch_multi_press::get_id();

    endpoint_t *endpoint = generic_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, NULL);
    if (endpoint == NULL) {
        ESP_LOGE(TAG_MIKE_APP, "Boot switch: failed to create Generic Switch endpoint");
        return 0;
    }

    s_switch_endpoint_id = endpoint::get_id(endpoint);

    ESP_LOGI(TAG_MIKE_APP, "Boot switch Generic Switch created, endpoint_id=%d", s_switch_endpoint_id);
    return s_switch_endpoint_id;
}

void boot_switch_task(void *pvParameters)
{
    (void)pvParameters;

    ESP_LOGI(TAG_MIKE_APP, "Boot switch task started (period=%d ms)", BOOT_SWITCH_TASK_PERIOD_MS);

    while (true) {
        uint32_t now_ms = esp_log_timestamp();
        bool level = (gpio_get_level((gpio_num_t)BOOT_SWITCH_GPIO) == 0); // активный низкий

        // ============================================================
        // ОБРАБОТКА НАЖАТИЯ (фронт: отпущена -> нажата)
        // ============================================================
        if (level && !s_button_pressed) {
            s_button_pressed = true;
            s_press_start_ms = now_ms;
            s_long_press_sent = false;

            // Отправляем InitialPress через официальную функцию esp-matter
            chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                switch_cluster::event::send_initial_press(endpoint_id, 1);
            });

            // Проверяем, это второй клик в окне двойного нажатия?
            if (s_first_click_pending) {
                uint32_t elapsed = now_ms - s_first_click_time_ms;
                if (elapsed <= BOOT_SWITCH_DOUBLE_CLICK_MS) {
                    // Второй клик — отправляем MultiPressOngoing с count=2
                    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                        switch_cluster::event::send_multi_press_ongoing(endpoint_id, 1, 2);
                    });
                } else {
                    s_first_click_pending = false;
                }
            }
        }

        // ============================================================
        // ОБРАБОТКА ОТПУСКАНИЯ (фронт: нажата -> отпущена)
        // ============================================================
        else if (!level && s_button_pressed) {
            s_button_pressed = false;
            uint32_t press_duration = now_ms - s_press_start_ms;

            // ============================================================
            // ДЛИННОЕ НАЖАТИЕ (>= 3 сек)
            // ============================================================
            if (s_long_press_sent || press_duration >= BOOT_SWITCH_LONG_PRESS_MS) {
                chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                    switch_cluster::event::send_long_release(endpoint_id, 0);
                });
                s_long_press_sent = false;
                s_first_click_pending = false;
            }
            // ============================================================
            // КОРОТКОЕ НАЖАТИЕ (< 3 сек)
            // ============================================================
            else {
                chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                    switch_cluster::event::send_short_release(endpoint_id, 0);
                });

                if (s_first_click_pending) {
                    // Это был второй клик — завершаем двойное нажатие
                    chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                        switch_cluster::event::send_multi_press_complete(endpoint_id, 0, 2);
                    });
                    s_first_click_pending = false;
                } else {
                    // Это первый клик — запускаем окно ожидания второго
                    s_first_click_pending = true;
                    s_first_click_time_ms = now_ms;
                }
            }
        }

        // ============================================================
        // ОТПРАВКА LongPress ВО ВРЕМЯ УДЕРЖАНИЯ
        // ============================================================
        if (s_button_pressed && !s_long_press_sent) {
            uint32_t held_ms = now_ms - s_press_start_ms;
            if (held_ms >= BOOT_SWITCH_LONG_PRESS_MS) {
                chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                    switch_cluster::event::send_long_press(endpoint_id, 1);
                });
                s_long_press_sent = true;
            }
        }

        // ============================================================
        // ПРОВЕРКА ИСТЕЧЕНИЯ ОКНА ДВОЙНОГО КЛИКА
        // ============================================================
        if (s_first_click_pending && !s_button_pressed) {
            uint32_t elapsed = now_ms - s_first_click_time_ms;
            if (elapsed > BOOT_SWITCH_DOUBLE_CLICK_MS) {
                // Окно истекло — завершаем одиночное нажатие
                chip::DeviceLayer::SystemLayer().ScheduleLambda([endpoint_id = s_switch_endpoint_id]() {
                    switch_cluster::event::send_multi_press_complete(endpoint_id, 0, 1);
                });
                s_first_click_pending = false;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(BOOT_SWITCH_TASK_PERIOD_MS));
    }
}

#endif // CONFIG_BOOT_SWITCH_ENABLED
