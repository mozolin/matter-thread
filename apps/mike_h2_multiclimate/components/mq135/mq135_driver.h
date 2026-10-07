/**
 * @file mq135_driver.h
 * @brief Драйвер для аналогового датчика качества воздуха MQ-135.
 */

#pragma once

#include "mq135_types.h"
#include "esp_err.h"
//#include "driver/adc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"


// Значения по умолчанию для калибровки (требуется уточнение для конкретного датчика и условий)
#define MQ135_DEFAULT_R0           10000.0f  // 10 кОм
#define MQ135_DEFAULT_LOAD_RES     10000.0f  // 10 кОм
#define MQ135_DEFAULT_SUPPLY_V     5.0f      // 5 В (типично для MQ-135, но может быть 3.3В)
#define MQ135_DEFAULT_VREF         3.3f      // Опорное напряжение ADC ESP32-H2

//-- КОЭФФИЦИЕНТЫ КРИВОЙ ЧУВСТВИТЕЛЬНОСТИ
// CO2: Наклон и смещение
#define MQ135_CO2_SLOPE     (-0.42f)
#define MQ135_CO2_OFFSET    ( 1.75f)
// CO: Наклон и смещение
#define MQ135_CO_SLOPE      (-0.48f)
#define MQ135_CO_OFFSET     ( 1.95f)
// TVOC (усреднённая кривая)
#define MQ135_TVOC_SLOPE    (-0.35f)
#define MQ135_TVOC_OFFSET   ( 1.60f)


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Дескриптор драйвера MQ-135.
 */
typedef struct {
    adc_oneshot_unit_handle_t adc_handle; /*!< Хэндл юнита ADC */
    adc_cali_handle_t cali_handle;        /*!< Хэндл калибровки ADC */
    adc_channel_t adc_channel;            /*!< Номер канала ADC */
    mq135_config_t config;                /*!< Калибровочные коэффициенты и параметры */
} mq135_handle_t;

/**
 * @brief Инициализация драйвера MQ-135.
 * 
 * @param[out] handle Указатель на структуру дескриптора, которая будет инициализирована.
 * @param[in] adc_unit Юнит ADC (ADC_UNIT_1 или ADC_UNIT_2).
 * @param[in] adc_channel Номер канала ADC (например, ADC_CHANNEL_3 для GPIO3).
 * @param[in] config Указатель на структуру с калибровочными данными. Если NULL, используются значения по умолчанию.
 * @return 
 *      - ESP_OK: Успешно
 *      - ESP_ERR_INVALID_ARG: Неверные аргументы
 *      - Другие коды ошибок ESP-IDF
 */
esp_err_t mq135_driver_init(mq135_handle_t *handle, adc_unit_t adc_unit, adc_channel_t adc_channel, const mq135_config_t *config);

/**
 * @brief Освобождение ресурсов драйвера.
 * 
 * @param handle Дескриптор драйвера.
 */
void mq135_driver_deinit(mq135_handle_t *handle);

/**
 * @brief Выполняет измерение и рассчитывает концентрацию для всех газов.
 * 
 * @param[in] handle Дескриптор драйвера.
 * @param[out] data Указатель на структуру для сохранения результатов.
 * @return 
 *      - ESP_OK: Успешно
 *      - ESP_ERR_INVALID_STATE: Драйвер не инициализирован
 *      - Другие коды ошибок
 */
esp_err_t mq135_read(mq135_handle_t *handle, mq135_data_t *data);

#ifdef __cplusplus
}
#endif
