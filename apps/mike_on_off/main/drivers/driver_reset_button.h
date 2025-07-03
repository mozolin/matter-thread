#pragma once

/********************
 *                  *
 *   RESET BUTTON   *
 *                  *
 ********************/
#include "esp_matter_core.h"
#define REBOOT_BUTTON_GPIO 9
//-- Hold for 3 seconds to reboot
#define REBOOT_HOLD_TIME_MS 3000
//-- Hold for 10 seconds to factory reset
#define FACTORY_RESET_HOLD_TIME_MS 10000
extern void reboot_button_task(void *pvParameter);
