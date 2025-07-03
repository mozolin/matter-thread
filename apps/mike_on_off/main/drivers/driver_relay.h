#include <cstdint>
#pragma once

#include <esp_err.h>
/**************
 *            *
 *   RELAYS   *
 *            *
 **************/
//-- Функция для выключения ВСЕХ реле, кроме указанного
extern void turn_off_other_relays(uint8_t excluded_endpoint);

//-- Управление реле (с взаимным отключением)
extern void relay_set_on_off(uint8_t endpoint, bool state);

//-- Функция для сохранения состояния реле в NVS
extern esp_err_t save_relay_state(uint8_t endpoint, bool state);

//-- Функция для загрузки состояния реле из NVS
extern bool load_relay_state(uint8_t endpoint);
