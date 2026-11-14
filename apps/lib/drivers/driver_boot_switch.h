#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "esp_matter_core.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;


// ============================================================
//  Boot Switch (Generic Switch) для событий кнопки BOOT
// ============================================================
#if CONFIG_BOOT_SWITCH_ENABLED
  //-- Период опроса GPIO кнопки BOOT в задаче boot_switch (мс)
  #define BOOT_SWITCH_POLL_MS              20
  //-- Порог удержания для генерации LongPress (мс)
  //-- ВАЖНО: не пересекается с REBOOT_HOLD_TIME_MS (3000),
  //-- но семантически LongPress в HA = удержание > 3 сек.
  //-- Поскольку reboot уже срабатывает на 3000 мс, LongPress
  //-- будет отправлен ДО перезагрузки (на 3000 мс), но после
  //-- этого устройство уйдёт в ребут. Если вы хотите чистый
  //-- LongPress без ребута — увеличьте REBOOT_HOLD_TIME_MS
  //-- или измените логику reboot_button_task.
  #define BOOT_SWITCH_LONG_PRESS_MS        3000
  //-- Окно между кликами для определения двойного нажатия (мс)
  #define BOOT_SWITCH_DOUBLE_CLICK_MS      400

  //-- Внешние функции драйвера boot_switch
  extern void boot_switch_driver_init(void);
  extern void boot_switch_task(void *pvParameters);
  extern void boot_switch_report_event(uint8_t event_type);

  //-- Типы событий (для внутреннего использования)
  #define BOOT_SWITCH_EVENT_SINGLE         1
  #define BOOT_SWITCH_EVENT_DOUBLE         2
  #define BOOT_SWITCH_EVENT_LONG           3

// ============================================================
//  Внешние ID On/Off эндпоинтов для управления из HA
// ============================================================
// Эти переменные определяются в app_main.cpp и используются
// в app_driver.cpp (app_driver_attribute_update) для маршрутизации
// On/Off команд от Home Assistant.
// ============================================================

  extern uint16_t s_ep_screen_onoff;   // On/Off для SSD1306
  extern uint16_t s_ep_led_onoff;      // On/Off для LED ленты (блокировка)
  extern uint16_t s_ep_reboot_onoff;   // On/Off для перезагрузки ESP32-H2

  //-- Внешняя функция для передачи endpoint_id в драйвер
  /*extern "C" */void boot_switch_set_endpoint(uint16_t endpoint_id, cluster_t *cluster);

  extern esp_err_t create_boot_switch_endpoint(node_t *node);
  extern esp_err_t create_onoff_endpoints(node_t *node);

#endif

#if CONFIG_BOOT_SWITCH_ENABLED && USE_DRIVER_LED_INDICATOR
  // ==================================
  // Значение OnOff для LED эндпоинта:
  // ON (true) или OFF (false)
  // ==================================
  void set_led_onoff_attribute(bool status);
#endif

#if CONFIG_BOOT_SWITCH_ENABLED
  // ==================================
  //  Запуск задачи опроса кнопки BOOT
  // ==================================
  void start_boot_switch_polling_task(void);
#endif
