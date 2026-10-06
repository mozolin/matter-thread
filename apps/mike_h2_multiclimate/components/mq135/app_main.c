/**
 * @file app_main.c
 * @brief Основное приложение: инициализация Matter, создание кластеров и обновление данных с MQ-135.
 */

#include <stdio.h>
#include <string.h>

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

// Заголовочные файлы ESP-Matter
#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include <esp_matter_prov.h>

// Заголовочные файлы для конкретных кластеров
#include <esp_matter_cluster.h>
// Используем пространства имен для читаемости
using namespace esp_matter;
using namespace esp_matter::cluster;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;

#include "mq135_driver.h"

static const char *TAG = "MQ135_MATTER";

// Глобальный дескриптор драйвера
static mq135_handle_t s_mq135_handle;
// Глобальные указатели на кластеры для быстрого доступа при обновлении
static cluster_t *s_co2_cluster = NULL;
static cluster_t *s_co_cluster = NULL;
static cluster_t *s_tvoc_cluster = NULL;

// Хэндл для задачи, которая будет обновлять данные
static TaskHandle_t s_update_task_handle = NULL;

/**
 * @brief Callback, вызываемый при изменении атрибутов (не используется для датчиков, но требуется).
 */
static esp_err_t app_attribute_update_cb(callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                          uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data) {
    // Датчики только для чтения, поэтому мы не ожидаем здесь изменений от контроллера.
    return ESP_OK;
}

/**
 * @brief Создает все необходимые кластеры для датчика качества воздуха.
 */
static void create_air_quality_clusters(endpoint_t *endpoint) {
    // === 1. Кластер Carbon Dioxide Concentration Measurement (0x040D) ===
    s_co2_cluster = cluster::create(endpoint, CarbonDioxideConcentrationMeasurement::Id, CLUSTER_FLAG_SERVER);
    if (s_co2_cluster) {
        // Включение функции NumericMeasurement (обязательна для MeasuredValue)
        CarbonDioxideConcentrationMeasurement::feature::numeric_measurement::config_t co2_feature_cfg;
        co2_feature_cfg.measured_value = 400.0f; // Начальное значение (примерно атмосферный уровень)
        co2_feature_cfg.min_measured_value = 0.0f;
        co2_feature_cfg.max_measured_value = 5000.0f;
        co2_feature_cfg.measurement_unit = 0; // 0 = PPM (согласно MeasurementUnitEnum)
        co2_feature_cfg.measurement_medium = 0; // 0 = Air
        
        esp_err_t err = CarbonDioxideConcentrationMeasurement::feature::numeric_measurement::add(s_co2_cluster, &co2_feature_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ошибка добавления фичи NumericMeasurement в кластер CO2: %s", esp_err_to_name(err));
        }
    }

    // === 2. Кластер Carbon Monoxide Concentration Measurement (0x040C) ===
    s_co_cluster = cluster::create(endpoint, CarbonMonoxideConcentrationMeasurement::Id, CLUSTER_FLAG_SERVER);
    if (s_co_cluster) {
        CarbonMonoxideConcentrationMeasurement::feature::numeric_measurement::config_t co_feature_cfg;
        co_feature_cfg.measured_value = 0.0f;
        co_feature_cfg.min_measured_value = 0.0f;
        co_feature_cfg.max_measured_value = 1000.0f;
        co_feature_cfg.measurement_unit = 0; // PPM
        co_feature_cfg.measurement_medium = 0; // Air
        
        esp_err_t err = CarbonMonoxideConcentrationMeasurement::feature::numeric_measurement::add(s_co_cluster, &co_feature_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ошибка добавления фичи NumericMeasurement в кластер CO: %s", esp_err_to_name(err));
        }
    }

    // === 3. Кластер Total Volatile Organic Compounds Concentration Measurement (0x041E) ===
    s_tvoc_cluster = cluster::create(endpoint, TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, CLUSTER_FLAG_SERVER);
    if (s_tvoc_cluster) {
        TotalVolatileOrganicCompoundsConcentrationMeasurement::feature::numeric_measurement::config_t tvoc_feature_cfg;
        tvoc_feature_cfg.measured_value = 0.0f;
        tvoc_feature_cfg.min_measured_value = 0.0f;
        tvoc_feature_cfg.max_measured_value = 5000.0f;
        tvoc_feature_cfg.measurement_unit = 0; // PPM
        tvoc_feature_cfg.measurement_medium = 0; // Air
        
        esp_err_t err = TotalVolatileOrganicCompoundsConcentrationMeasurement::feature::numeric_measurement::add(s_tvoc_cluster, &tvoc_feature_cfg);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ошибка добавления фичи NumericMeasurement в кластер TVOC: %s", esp_err_to_name(err));
        }
    }
}

/**
 * @brief Задача FreeRTOS для периодического чтения датчика и обновления атрибутов Matter.
 */
static void sensor_update_task(void *pvParameters) {
    mq135_data_t sensor_data;
    endpoint_t *endpoint = (endpoint_t *)pvParameters;
    
    while (1) {
        // 1. Чтение данных с MQ-135
        esp_err_t err = mq135_read(&s_mq135_handle, &sensor_data);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "Ошибка чтения MQ-135: %s", esp_err_to_name(err));
            vTaskDelay(pdMS_TO_TICKS(5000));
            continue;
        }

        ESP_LOGI(TAG, "Данные MQ-135 -> CO2: %.1f ppm, CO: %.1f ppm, TVOC: %.1f ppm",
                 sensor_data.co2.ppm, sensor_data.co.ppm, sensor_data.tvoc.ppm);

        // 2. Обновление атрибута MeasuredValue для кластера CO2
        if (s_co2_cluster) {
            attribute_t *attr = attribute::get(s_co2_cluster, CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id);
            if (attr) {
                esp_matter_attr_val_t val = esp_matter_float(sensor_data.co2.ppm);
                err = attribute::update(endpoint::get_id(endpoint), CarbonDioxideConcentrationMeasurement::Id, 
                                        CarbonDioxideConcentrationMeasurement::Attributes::MeasuredValue::Id, &val);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Ошибка обновления атрибута CO2 MeasuredValue: %s", esp_err_to_name(err));
                }
            }
        }

        // 3. Обновление атрибута MeasuredValue для кластера CO
        if (s_co_cluster) {
            attribute_t *attr = attribute::get(s_co_cluster, CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Id);
            if (attr) {
                esp_matter_attr_val_t val = esp_matter_float(sensor_data.co.ppm);
                err = attribute::update(endpoint::get_id(endpoint), CarbonMonoxideConcentrationMeasurement::Id, 
                                        CarbonMonoxideConcentrationMeasurement::Attributes::MeasuredValue::Id, &val);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Ошибка обновления атрибута CO MeasuredValue: %s", esp_err_to_name(err));
                }
            }
        }

        // 4. Обновление атрибута MeasuredValue для кластера TVOC
        if (s_tvoc_cluster) {
            attribute_t *attr = attribute::get(s_tvoc_cluster, TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id);
            if (attr) {
                esp_matter_attr_val_t val = esp_matter_float(sensor_data.tvoc.ppm);
                err = attribute::update(endpoint::get_id(endpoint), TotalVolatileOrganicCompoundsConcentrationMeasurement::Id, 
                                        TotalVolatileOrganicCompoundsConcentrationMeasurement::Attributes::MeasuredValue::Id, &val);
                if (err != ESP_OK) {
                    ESP_LOGE(TAG, "Ошибка обновления атрибута TVOC MeasuredValue: %s", esp_err_to_name(err));
                }
            }
        }

        // Ожидание перед следующим измерением (например, 10 секунд)
        vTaskDelay(pdMS_TO_TICKS(10000));
    }
}

/**
 * @brief Основная точка входа приложения.
 */
extern "C" void app_main() {
    esp_err_t err = ESP_OK;

    // === 1. Инициализация драйвера MQ-135 ===
    // Выбираем GPIO 3, который соответствует каналу ADC1_CHANNEL_2 на ESP32-H2
    // (Согласно таблице ADC1_GPIO3_CHANNEL = 2)
    adc_unit_t adc_unit = ADC_UNIT_1;
    adc_channel_t adc_channel = ADC_CHANNEL_2; // GPIO3
    
    ESP_LOGI(TAG, "Инициализация драйвера MQ-135 на GPIO3 (ADC1_CHANNEL_2)...");
    err = mq135_driver_init(&s_mq135_handle, adc_unit, adc_channel, NULL);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Критическая ошибка: не удалось инициализировать драйвер MQ-135. Перезагрузка...");
        esp_restart();
    }

    // === 2. Инициализация Matter Node и Endpoint ===
    ESP_LOGI(TAG, "Инициализация Matter...");
    
    // Настройка node
    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, NULL);
    if (!node) {
        ESP_LOGE(TAG, "Не удалось создать Matter Node");
        return;
    }

    // Создание endpoint с типом Air Quality Sensor
    // Это стандартный тип устройства, который подходит для нашего датчика.
    air_quality_sensor::config_t air_quality_config;
    endpoint_t *endpoint = air_quality_sensor::create(node, &air_quality_config, ENDPOINT_FLAG_NONE, NULL);
    if (!endpoint) {
        ESP_LOGE(TAG, "Не удалось создать Air Quality Sensor endpoint");
        return;
    }

    // Создание кластеров измерения концентрации
    create_air_quality_clusters(endpoint);

    // === 3. Запуск Matter стека ===
    ESP_LOGI(TAG, "Запуск Matter стека...");
    err = esp_matter::start(app_event_cb); // app_event_cb может быть вашей функцией обработки событий
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Ошибка запуска Matter: %s", esp_err_to_name(err));
        return;
    }

    // === 4. Создание задачи для опроса датчика ===
    xTaskCreate(sensor_update_task, "mq135_update", 4096, endpoint, 5, &s_update_task_handle);
    
    ESP_LOGI(TAG, "Приложение успешно запущено. Ожидание подключения...");
}

// Заглушка для app_event_cb, если она не определена где-то еще.
// В реальном проекте здесь обрабатываются события Matter (подключение, отключение и т.д.).
static void app_event_cb(const chip::DeviceLayer::ChipDeviceEvent *event, intptr_t arg) {
    switch (event->Type) {
        case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
            ESP_LOGI(TAG, "Устройство успешно добавлено в сеть Matter!");
            break;
        case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
            ESP_LOGI(TAG, "Устройство удалено из сети Matter.");
            break;
        default:
            break;
    }
}