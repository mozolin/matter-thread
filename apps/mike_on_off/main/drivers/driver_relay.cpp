
#include "driver_relay.h"
#include "driver/gpio.h"
#include <vector>

#include "nvs_flash.h"
#include "nvs.h"


/**************
 *            *
 *   RELAYS   *
 *            *
 **************/
// Конфигурация реле (пины и эндпоинты)
struct RelayConfig {
  uint8_t endpoint;
  gpio_num_t gpio_pin;
};

// Список всех реле (можно расширить)
static const std::vector<RelayConfig> relays = {
  {1, GPIO_NUM_0},
	{2, GPIO_NUM_1},
	{3, GPIO_NUM_2},
	{4, GPIO_NUM_3},
	{5, GPIO_NUM_5},
	{6, GPIO_NUM_10},
	{7, GPIO_NUM_11},
	{8, GPIO_NUM_12},
};

// Функция для выключения ВСЕХ реле, кроме указанного
void turn_off_other_relays(uint8_t excluded_endpoint)
{
  for(const auto &relay : relays) {
    if(relay.endpoint != excluded_endpoint) {
      bool state = !load_relay_state(relay.endpoint);
      gpio_set_level(relay.gpio_pin, state);  // Физически выключаем
      save_relay_state(relay.endpoint, state);  // Сохраняем в NVS
    }
  }
}

// Управление реле (с взаимным отключением)
void relay_set_on_off(uint8_t endpoint, bool state)
{
  for(const auto &relay : relays) {
    if(relay.endpoint == endpoint) {
      gpio_set_level(relay.gpio_pin, state ? 1 : 0);
      save_relay_state(endpoint, state);

      // Если включаем реле - выключаем все остальные
      //if(state) {
        turn_off_other_relays(endpoint);
      //}
      break;
    }
  }
}


/*********************
 *                   *
 *   NVS SAVE/LOAD   *
 *                   *
 *********************/
//-- Функция для сохранения состояния реле в NVS
esp_err_t save_relay_state(uint8_t endpoint, bool state)
{
  nvs_handle_t handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) return err;

  char key[10];
  snprintf(key, sizeof(key), "relay_%d", endpoint);
  err = nvs_set_u8(handle, key, state ? 1 : 0);
  nvs_close(handle);
  return err;
}

//-- Функция для загрузки состояния реле из NVS
bool load_relay_state(uint8_t endpoint)
{
  nvs_handle_t handle;
  esp_err_t err = nvs_open("storage", NVS_READWRITE, &handle);
  if (err != ESP_OK) return false;

  char key[10];
  snprintf(key, sizeof(key), "relay_%d", endpoint);
  uint8_t state = 0;
  nvs_get_u8(handle, key, &state);
  nvs_close(handle);
  return state == 1;
}
