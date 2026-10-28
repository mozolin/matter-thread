#pragma once

#include "esp_matter_core.h"

// ============================================================
// ДОБАВЛЕНО: Драйвер Boot Switch (Generic Switch)
// ============================================================
// Этот драйвер НЕ инициализирует GPIO (это делает reboot_button_task),
// а только опрашивает его и генерирует события Matter Generic Switch.
// ============================================================

#if CONFIG_BOOT_SWITCH_ENABLED
  extern void boot_switch_driver_init(void);
  extern void boot_switch_task(void *pvParameters);
  extern void boot_switch_report_event(uint8_t event_type);
#endif