#pragma once

#include "esp_matter_core.h"
//#include <app_priv.h>

/********************
 *                  *
 *   RESET BUTTON   *
 *                  *
 ********************/
/*
#define REBOOT_BUTTON_GPIO 9
//-- Hold for 3 seconds to reboot
#define REBOOT_HOLD_TIME_MS 3000
//-- Hold for 10 seconds to factory reset
#define FACTORY_RESET_HOLD_TIME_MS 10000
*/
extern void reboot_button_task(void *pvParameter);

extern void single_click_timer_init(void);

//=== ДОБАВЛЕНО: обработка двойного клика кнопки Reset ===
//-- Переключает блокировку/разблокировку функции get_led_indicator_blink_idx.
//-- Вызывается из esp_timer-колбэка, когда окно двойного клика истекло
//-- и второго нажатия не последовало (т.е. был одиночный клик).
extern void reset_button_handle_single_click(void);
//=== ДОБАВЛЕНО ===
