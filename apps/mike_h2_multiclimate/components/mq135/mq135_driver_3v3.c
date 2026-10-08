/**
 * @file mq135_driver_3v3.c
 * @brief Реализация драйвера MQ-135 для питания от 3.3В.
 *
 * ИЗМЕНЕНО ДЛЯ 3.3V:
 *  - Питание датчика: 3.3В вместо 5В.
 *  - Опорное напряжение ADC: 3.3В (совпадает с питанием).
 *  - Напряжение на выходе AOUT не может превысить 3.3В, поэтому делитель напряжения НЕ требуется.
 *  - Значения R0 по умолчанию пересчитаны для работы при 3.3В (R0 в 3.3В-режиме выше, чем при 5В).
 *  - Добавлена функция mq135_calibrate_r0() для автоматической калибровки.
 *  - Коэффициенты slope/offset подобраны с учётом меньшего диапазона напряжения.
 */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mq135_driver_3v3.h"
#include "esp_log.h"
#include "math.h"

static const char *TAG = "MQ135_DRIVER_3V3";

/**
 * @brief Внутренняя функция: чтение напряжения с ADC и расчёт Rs.
 * 
 * ИЗМЕНЕНО ДЛЯ 3.3V: формула расчёта Rs осталась той же, но с учётом того,
 * что supply_voltage = vref = 3.3В, а делитель не используется.
 */
static esp_err_t mq135_read_internal_3v3(mq135_handle_t *handle, float *voltage_out, float *rs_out) {
    if (handle == NULL || voltage_out == NULL || rs_out == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Чтение сырого значения ADC
    int raw = 0;
    esp_err_t ret = adc_oneshot_read(handle->adc_handle, handle->adc_channel, &raw);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ошибка чтения ADC: %s", esp_err_to_name(ret));
        return ret;
    }

    // Конвертация в напряжение (мВ)
    float voltage_mv = 0.0f;
    if (handle->cali_handle) {
        int calibrated_mv = 0;
        ret = adc_cali_raw_to_voltage(handle->cali_handle, raw, &calibrated_mv);
        if (ret == ESP_OK) {
            voltage_mv = (float)calibrated_mv;
        }
    } else {
        // ИЗМЕНЕНО ДЛЯ 3.3V: линейное преобразование с опорным напряжением 3.3В
        voltage_mv = ((float)raw / 4095.0f) * handle->config.vref * 1000.0f;
    }

    float voltage = voltage_mv / 1000.0f; // В вольтах

    // ИЗМЕНЕНО ДЛЯ 3.3V: при питании от 3.3В и отсутствии делителя
    // формула Rs = RL * (VCC - V_adc) / V_adc остаётся той же,
    // где VCC = 3.3В. Максимум V_adc ~3.3В (при Rs -> 0), минимум ~0В (при Rs -> ∞).
    float rs = 0.0f;
    if (voltage > 0.01f) {
        rs = handle->config.load_resistor * (handle->config.supply_voltage - voltage) / voltage;
    } else {
        rs = 1000000.0f; // Очень большое сопротивление, если напряжение почти 0
    }

    *voltage_out = voltage;
    *rs_out = rs;
    return ESP_OK;
}

esp_err_t mq135_driver_init_3v3(mq135_handle_t *handle, adc_unit_t adc_unit, adc_channel_t adc_channel, const mq135_config_t *config) {
    if (handle == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Инициализация ADC One-Shot
    adc_oneshot_unit_init_cfg_t init_config = {
        .unit_id = adc_unit,
        .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    esp_err_t ret = adc_oneshot_new_unit(&init_config, &handle->adc_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ошибка инициализации ADC юнита: %s", esp_err_to_name(ret));
        return ret;
    }

    // Конфигурация канала ADC
    // ИЗМЕНЕНО ДЛЯ 3.3V: аттенюация 12 дБ даёт полный диапазон до ~3.3В,
    // что идеально соответствует питанию датчика 3.3В.
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,          // Полный диапазон ~0..3.3В
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_oneshot_config_channel(handle->adc_handle, adc_channel, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ошибка конфигурации канала ADC: %s", esp_err_to_name(ret));
        adc_oneshot_del_unit(handle->adc_handle);
        return ret;
    }

    handle->adc_channel = adc_channel;

    // Калибровка ADC (криволинейная для ESP32-H2)
    adc_cali_curve_fitting_config_t cali_config = {
        .unit_id = adc_unit,
        .chan = adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_cali_create_scheme_curve_fitting(&cali_config, &handle->cali_handle);
    if (ret != ESP_OK) {
        ESP_LOGW(TAG, "Калибровка ADC не удалась (не критично): %s", esp_err_to_name(ret));
        handle->cali_handle = NULL;
    }

    // Заполнение конфигурации
    if (config != NULL) {
        handle->config = *config;
    } else {
        /* ============================================================
         *  ЗНАЧЕНИЯ ПО УМОЛЧАНИЮ (ИЗМЕНЕНО ДЛЯ 3.3V)
         * ============================================================ */
        handle->config.r0             = MQ135_DEFAULT_R0;       // 15 кОм
        handle->config.load_resistor  = MQ135_DEFAULT_LOAD_RES; // 10 кОм
        handle->config.supply_voltage = MQ135_DEFAULT_SUPPLY_V; // 3.3 В
        handle->config.vref           = MQ135_DEFAULT_VREF;     // 3.3 В

        // ИЗМЕНЕНО ДЛЯ 3.3V: коэффициенты для CO2, CO, TVOC
        handle->config.slope[0]  = MQ135_CO2_SLOPE;
        handle->config.offset[0] = MQ135_CO2_OFFSET;

        handle->config.slope[1]  = MQ135_CO_SLOPE;
        handle->config.offset[1] = MQ135_CO_OFFSET;

        handle->config.slope[2]  = MQ135_TVOC_SLOPE;
        handle->config.offset[2] = MQ135_TVOC_OFFSET;
    }

    ESP_LOGI(TAG, "Драйвер MQ-135 инициализирован (питание: %.1fВ, канал: %d, R0: %.0f Ом)",
             handle->config.supply_voltage, adc_channel, handle->config.r0);
    return ESP_OK;
}

void mq135_driver_deinit_3v3(mq135_handle_t *handle) {
    if (handle == NULL) return;

    if (handle->cali_handle) {
        adc_cali_delete_scheme_curve_fitting(handle->cali_handle);
        handle->cali_handle = NULL;
    }
    if (handle->adc_handle) {
        adc_oneshot_del_unit(handle->adc_handle);
        handle->adc_handle = NULL;
    }
    ESP_LOGI(TAG, "Драйвер MQ-135 деинициализирован");
}

esp_err_t mq135_read_3v3(mq135_handle_t *handle, mq135_data_t *data) {
    if (handle == NULL || data == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    float voltage = 0.0f;
    float rs = 0.0f;

    esp_err_t ret = mq135_read_internal_3v3(handle, &voltage, &rs);
    if (ret != ESP_OK) {
        return ret;
    }

    // Заполнение данных о напряжении и сопротивлении
    data->co2.voltage  = voltage;
    data->co2.resistance  = rs;
    data->co.voltage   = voltage;
    data->co.resistance   = rs;
    data->tvoc.voltage = voltage;
    data->tvoc.resistance = rs;

    // Расчёт ppm для каждого газа
    // Формула: log10(Rs/R0) = m * log10(ppm) + b  =>  ppm = 10 ^ ((log10(Rs/R0) - b) / m)
    if (rs > 0.0f && handle->config.r0 > 0.0f) {
        float log_rs_r0 = log10f(rs / handle->config.r0);

        // CO2
        float log_ppm_co2 = (log_rs_r0 - handle->config.offset[0]) / handle->config.slope[0];
        data->co2.ppm = powf(10.0f, log_ppm_co2);

        // CO
        float log_ppm_co = (log_rs_r0 - handle->config.offset[1]) / handle->config.slope[1];
        data->co.ppm = powf(10.0f, log_ppm_co);

        // TVOC
        float log_ppm_tvoc = (log_rs_r0 - handle->config.offset[2]) / handle->config.slope[2];
        data->tvoc.ppm = powf(10.0f, log_ppm_tvoc);
    } else {
        data->co2.ppm  = 0.0f;
        data->co.ppm   = 0.0f;
        data->tvoc.ppm = 0.0f;
    }

    data->timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    ESP_LOGD(TAG, "Напряжение: %.3f В, Rs: %.1f Ом, CO2: %.1f ppm, CO: %.1f ppm, TVOC: %.1f ppm",
             voltage, rs, data->co2.ppm, data->co.ppm, data->tvoc.ppm);

    return ESP_OK;
}

/**
 * @brief Калибровка R0 в чистом воздухе.
 * 
 * ИЗМЕНЕНО ДЛЯ 3.3V: функция добавлена специально для варианта питания 3.3В,
 * так как при меньшем напряжении R0 датчика отличается от паспортного (для 5В)
 * и требует обязательной калибровки на месте.
 * 
 * Процедура:
 *  1. Поместите датчик в чистый воздух (улица, проветренное помещение).
 *  2. Прогрейте датчик 24-48 часов (обязательно для MQ-135!).
 *  3. Вызовите эту функцию. Она усреднит N выборок Rs и запишет результат в config.r0.
 */
esp_err_t mq135_calibrate_r0_3v3(mq135_handle_t *handle, uint16_t sample_count) {
    if (handle == NULL || sample_count == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Начало калибровки R0 (ИЗМЕНЕНО ДЛЯ 3.3V). Выборок: %d", sample_count);
    
    float sum_rs = 0.0f;
    uint16_t valid_samples = 0;

    for (uint16_t i = 0; i < sample_count; i++) {
        float voltage = 0.0f;
        float rs = 0.0f;

        esp_err_t ret = mq135_read_internal_3v3(handle, &voltage, &rs);
        if(ret == ESP_OK && rs > 0.0f && rs < 1000000.0f) {
          sum_rs += rs;
          valid_samples++;
        }
        
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if(valid_samples == 0) {
      ESP_LOGE(TAG, "Калибровка не удалась: нет валидных выборок");
      return ESP_FAIL;
    }

    float rs_avg = sum_rs / (float)valid_samples;

    /* ============================================================
     *  ИЗМЕНЕНО ДЛЯ 3.3V:
     *  В чистом воздухе Rs ≈ R0 * (коэффициент для чистого воздуха).
     *  Для MQ-135 в чистом воздухе Rs/R0 ≈ 3.6 (по даташиту).
     *  Поэтому R0 = Rs_avg / 3.6.
     *  Это значение справедливо и для 3.3В, так как кривая чувствительности
     *  нормируется относительно R0, измеренного в тех же условиях.
     * ============================================================ */
    const float RS_R0_RATIO_CLEAN_AIR = 3.6f;
    handle->config.r0 = rs_avg / RS_R0_RATIO_CLEAN_AIR;

    ESP_LOGW("", "");
    ESP_LOGW("", "=====================================================================");
    ESP_LOGW("", "");
    ESP_LOGW("", " Калибровка завершена. Среднее Rs: %.1f Ом, новый R0: %.1f Ом", rs_avg, handle->config.r0);
    ESP_LOGW("", "");
    ESP_LOGW("", "=====================================================================");
    ESP_LOGW("", "");

    vTaskDelay(pdMS_TO_TICKS(10000));
    
    return ESP_OK;
}
