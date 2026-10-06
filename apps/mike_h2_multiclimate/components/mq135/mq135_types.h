/**
 * @file mq135_types.h
 * @brief Типы данных и структуры для датчика MQ-135.
 */

#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Перечисление возможных типов газов, измеряемых MQ-135.
 */
typedef enum {
    MQ135_GAS_CO2,      /*!< Углекислый газ (CO2) */
    MQ135_GAS_CO,       /*!< Угарный газ (CO) */
    MQ135_GAS_TVOC,     /*!< Общие летучие органические соединения (TVOC) */
    MQ135_GAS_MAX
} mq135_gas_type_t;

/**
 * @brief Структура калибровочных данных и конфигурации для MQ-135.
 * 
 * Кривая чувствительности MQ-135 описывается логарифмической зависимостью:
 * log10(Rs/R0) = m * log10(ppm) + b
 * где Rs - сопротивление датчика, R0 - сопротивление в чистом воздухе,
 * m и b - коэффициенты наклона и смещения для конкретного газа.
 */
typedef struct {
    float r0;           /*!< Сопротивление датчика в чистом воздухе (Ом) */
    float load_resistor; /*!< Сопротивление нагрузочного резистора (Ом) */
    float supply_voltage; /*!< Напряжение питания датчика (В) */
    float vref;         /*!< Опорное напряжение АЦП (В) */
    float slope[3];     /*!< Коэффициенты наклона (m) для CO2, CO, TVOC */
    float offset[3];    /*!< Коэффициенты смещения (b) для CO2, CO, TVOC */
} mq135_config_t;

/**
 * @brief Структура для хранения одного измерения.
 */
typedef struct {
    float voltage;       /*!< Напряжение на аналоговом выходе датчика (В) */
    float resistance;    /*!< Рассчитанное сопротивление датчика Rs (Ом) */
    float ppm;           /*!< Концентрация газа в ppm */
} mq135_measurement_t;

/**
 * @brief Структура, содержащая последние измеренные значения для всех газов.
 */
typedef struct {
    mq135_measurement_t co2;
    mq135_measurement_t co;
    mq135_measurement_t tvoc;
    uint32_t timestamp_ms; /*!< Время последнего обновления данных */
} mq135_data_t;

#ifdef __cplusplus
}
#endif
