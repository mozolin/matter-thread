#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "mq135_driver.h"
#include "esp_log.h"
#include "math.h"

static const char *TAG = "MQ135_DRIVER";

esp_err_t mq135_driver_init(mq135_handle_t *handle, adc_unit_t adc_unit, adc_channel_t adc_channel, const mq135_config_t *config) {
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
    adc_oneshot_chan_cfg_t chan_config = {
        .atten = ADC_ATTEN_DB_12,      // Аттенюация 12 дБ для измерения до ~3.3В
        .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ret = adc_oneshot_config_channel(handle->adc_handle, adc_channel, &chan_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Ошибка конфигурации канала ADC: %s", esp_err_to_name(ret));
        adc_oneshot_del_unit(handle->adc_handle);
        return ret;
    }

    handle->adc_channel = adc_channel;

    // Калибровка ADC (для ESP32-H2 используем криволинейную калибровку)
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
        // Установка значений по умолчанию
        handle->config.r0 = MQ135_DEFAULT_R0;
        handle->config.load_resistor = MQ135_DEFAULT_LOAD_RES;
        handle->config.supply_voltage = MQ135_DEFAULT_SUPPLY_V;
        handle->config.vref = MQ135_DEFAULT_VREF;
        
        // Примерные коэффициенты для MQ-135 (требуют калибровки!)
        // Формат: log10(Rs/R0) = m * log10(ppm) + b
        // Отсюда: ppm = 10 ^ ((log10(Rs/R0) - b) / m)
        // Значения m и b взяты из типовых даташитов, но могут сильно варьироваться.
        // CO2
        handle->config.slope[0]  = MQ135_CO2_SLOPE;
        handle->config.offset[0] = MQ135_CO2_OFFSET;
        // CO
        handle->config.slope[1]  = MQ135_CO_SLOPE;
        handle->config.offset[1] = MQ135_CO_OFFSET;
        // TVOC (усредненно)
        handle->config.slope[2]  = MQ135_TVOC_SLOPE;
        handle->config.offset[2] = MQ135_TVOC_OFFSET;
    }

    ESP_LOGI(TAG, "Драйвер MQ-135 инициализирован (канал: %d)", adc_channel);
    return ESP_OK;
}

void mq135_driver_deinit(mq135_handle_t *handle) {
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

esp_err_t mq135_read(mq135_handle_t *handle, mq135_data_t *data) {
    if (handle == NULL || data == NULL) {
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
        // Если калибровка недоступна, используем простое линейное преобразование
        // (предполагая 12-битную разрядность и опорное напряжение)
        voltage_mv = ((float)raw / 4095.0f) * handle->config.vref * 1000.0f;
    }
    
    float voltage = voltage_mv / 1000.0f; // В вольтах

    // Расчет сопротивления датчика Rs.
    // Схема делителя: VCC --- [Rs] --- (ADC pin) --- [RL] --- GND
    // Напряжение на ADC: V_adc = VCC * RL / (Rs + RL)
    // Отсюда: Rs = RL * (VCC - V_adc) / V_adc
    float rs = 0.0f;
    if (voltage > 0.01f) { // Защита от деления на ноль
        rs = handle->config.load_resistor * (handle->config.supply_voltage - voltage) / voltage;
    } else {
        rs = 1000000.0f; // Очень большое сопротивление, если напряжение почти 0
    }

    // Заполнение данных о напряжении и сопротивлении
    data->co2.voltage = voltage;
    data->co2.resistance = rs;
    data->co.voltage = voltage;
    data->co.resistance = rs;
    data->tvoc.voltage = voltage;
    data->tvoc.resistance = rs;

    // Расчет ppm для каждого газа
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
        data->co2.ppm = 0.0f;
        data->co.ppm = 0.0f;
        data->tvoc.ppm = 0.0f;
    }

    data->timestamp_ms = xTaskGetTickCount() * portTICK_PERIOD_MS;

    ESP_LOGD(TAG, "Напряжение: %.3f В, Rs: %.1f Ом, CO2: %.1f ppm, CO: %.1f ppm, TVOC: %.1f ppm",
             voltage, rs, data->co2.ppm, data->co.ppm, data->tvoc.ppm);

    return ESP_OK;
}
