#pragma once

#include <esp_err.h>
#include <esp_matter.h>

#if CONFIG_BOOT_SWITCH_ENABLED

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Инициализация драйвера Boot Switch
 *
 * Настраивает GPIO 9 как вход с подтяжкой, создаёт внутренние таймеры
 * для детекции одиночного/двойного нажатия и длинного нажатия.
 * Вызывается один раз при старте приложения.
 *
 * @return ESP_OK при успехе
 */
esp_err_t boot_switch_driver_init(void);

/**
 * @brief Создание Matter endpoint для Generic Switch
 *
 * Создаёт endpoint с cluster Switch и необходимыми feature flags:
 * - MomentarySwitch (MS)
 * - MomentarySwitchRelease (MSR)
 * - MomentarySwitchLongPress (MSL)
 * - MomentarySwitchMultiPress (MSM)
 *
 * @param node Указатель на Matter node
 * @return endpoint_id созданного endpoint или 0 при ошибке
 */
uint16_t boot_switch_create_endpoint(esp_matter::node_t *node);

/**
 * @brief Запуск задачи опроса кнопки
 *
 * Задача читает состояние GPIO 9 и генерирует соответствующие
 * Matter switch события (InitialPress, ShortRelease, LongPress,
 * LongRelease, MultiPressOngoing, MultiPressComplete).
 *
 * @param pvParameters Не используется
 */
void boot_switch_task(void *pvParameters);

#ifdef __cplusplus
}
#endif

#endif // CONFIG_BOOT_SWITCH_ENABLED
