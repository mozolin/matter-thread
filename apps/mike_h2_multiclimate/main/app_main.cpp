#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_attribute.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "sensor_driver_bme280.h"
#include "sensor_driver_bme680.h"
#include "sensor_driver_ds18b20.h"
#if CONFIG_DHT11_ENABLED
  #include "sensor_driver_dht11.h"
#endif
#if CONFIG_MQ135_ENABLED
  #if CONFIG_MQ135_3V3_CIRCUIT
    #include "sensor_driver_mq135_3v3.h"
  #else
    #include "sensor_driver_mq135.h"
  #endif
  // Глобальный дескриптор драйвера
  mq135_handle_t s_mq135_handle;
  // Глобальные указатели на кластеры для быстрого доступа при обновлении
  cluster_t *s_air_quality_cluster = NULL;
  cluster_t *s_co2_cluster = NULL;
  cluster_t *s_co_cluster = NULL;
  cluster_t *s_tvoc_cluster = NULL;
  // Хэндл для задачи, которая будет обновлять данные
  TaskHandle_t s_update_task_handle = NULL;
#endif
#if CONFIG_SHT41_ENABLED
  #include "sensor_driver_sht41.h"
#endif

#if CONFIG_BOOT_SWITCH_ENABLED
  #include "driver_boot_switch.h"
  //-- Внешняя функция для передачи endpoint_id в драйвер
  extern "C" void boot_switch_set_endpoint(uint16_t endpoint_id, cluster_t *cluster);
#endif

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

//--> uses for CommissioningWindowOpen
constexpr auto k_timeout_seconds = CONFIG_COMMISSIONING_WINDOW_OPEN;
//<-- uses for CommissioningWindowOpen
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[SENSOR_TYPE_MAX];
sensor_data_t sensors[SENSOR_TYPE_MAX];


#if CONFIG_SSD1306_ENABLED
  //-- SSD1306 device instance
  SSD1306_t ssd1306dev;
  //-- Is SSD1306 initialized?
  bool ssd1306_initialized = false;
  //=== состояние дисплея (по умолчанию — ВЫКЛЮЧЕН) ===
  bool ssd1306_display_enabled = false;
  uint64_t ssd1306_last_on_time_ms = 0;
#endif

/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
/*
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  led_indicator_handle_t led_handle;
#endif
*/
#if USE_DRIVER_LED_INDICATOR
  led_indicator_handle_t led_handle;
#endif

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Interface IP Address changed");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning complete");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning failed, fail safe timer expired");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning session started");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning session stopped");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning window opened");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Commissioning window closed");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Fabric removed successfully");
        #endif
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    #if DEBUG_MODE
                      ESP_LOGE(TAG_MIKE_APP, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    #endif
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Fabric will be removed");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Fabric is updated");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Fabric is committed");
        #endif
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "BLE deinitialized and memory reclaimed");
        #endif
        break;

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    #if DEBUG_MODE
      ESP_LOGI(TAG_MIKE_APP, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    #endif
    return ESP_OK;
}

static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

static esp_err_t create_sensor_endpoint(sensor_type_t sensor_cfg, node_t* node)
{
    esp_err_t err = ESP_OK;

    //if (!node || !sensor_cfg) {
    if(!node) {
      return ESP_ERR_INVALID_ARG;
    }

    // Check if sensor is already configured
    for(int i = 0; i < configured_sensors; i++) {
      if(sensor_mapping_list[i].sensor_type == sensor_cfg) {
        #if DEBUG_MODE
          ESP_LOGW(TAG_MIKE_APP, "Sensor type %d already configured", sensor_cfg);
        #endif
        return ESP_ERR_INVALID_STATE;
      }
    }

    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg) {
        case SENSOR_TYPE_BME280: {
            // Create endpoint with multiple clusters
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", " Start creating endpoint for BME280");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", "");
            #endif
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Failed to create endpoint for BME280");
              #endif
              return ESP_FAIL;
            }

            //-- Add Humidity Measurement cluster
            cluster::relative_humidity_measurement::config_t hum_config;
            hum_config.measured_value = nullable<uint16_t>(5000);
            hum_config.min_measured_value = nullable<uint16_t>(0);
            hum_config.max_measured_value = nullable<uint16_t>(10000);
            cluster::relative_humidity_measurement::create(endpoint, &hum_config, CLUSTER_FLAG_SERVER);

            //-- Add Pressure Measurement cluster
            cluster::pressure_measurement::config_t press_config;
            // 1013.25 hPa = 10132.5 в единицах 0.1 hPa, округляем до 10133
            press_config.pressure_measured_value = nullable<int16_t>(10133);    // 1013.25 hPa (10133 * 0.1 = 1013.3)
            press_config.pressure_min_measured_value = nullable<int16_t>(3000); // 300 hPa (3000 * 0.1 = 300.0)
            press_config.pressure_max_measured_value = nullable<int16_t>(11000); // 1100 hPa (11000 * 0.1 = 1100.0)
            cluster::pressure_measurement::create(endpoint, &press_config, CLUSTER_FLAG_SERVER);

            break;
        }

        case SENSOR_TYPE_BME680: {
            // Create endpoint with multiple clusters
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", " Start creating endpoint for BME680");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", "");
            #endif
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
                        
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if (!endpoint) {
                #if DEBUG_MODE
                  ESP_LOGE(TAG_MIKE_APP, "Failed to create endpoint for BME680");
                #endif
                return ESP_FAIL;
            }

            //-- Add Humidity Measurement cluster
            cluster::relative_humidity_measurement::config_t hum_config;
            hum_config.measured_value = nullable<uint16_t>(5000);
            hum_config.min_measured_value = nullable<uint16_t>(0);
            hum_config.max_measured_value = nullable<uint16_t>(10000);
            cluster::relative_humidity_measurement::create(endpoint, &hum_config, CLUSTER_FLAG_SERVER);

            //-- Add Pressure Measurement cluster
            cluster::pressure_measurement::config_t press_config;
            // 1013.25 hPa = 10132.5 в единицах 0.1 hPa, округляем до 10133
            //press_config.pressure_measured_value = nullable<int16_t>(10133);    // 1013.25 hPa (10133 * 0.1 = 1013.3)
            press_config.pressure_measured_value = nullable<int16_t>(0);    // 1013.25 hPa (10133 * 0.1 = 1013.3)
            press_config.pressure_min_measured_value = nullable<int16_t>(3000); // 300 hPa (3000 * 0.1 = 300.0)
            press_config.pressure_max_measured_value = nullable<int16_t>(11000); // 1100 hPa (11000 * 0.1 = 1100.0)
            cluster::pressure_measurement::create(endpoint, &press_config, CLUSTER_FLAG_SERVER);
            
            //-- Add Gas Resistance Measurement cluster
            cluster::total_volatile_organic_compounds_concentration_measurement::config_t tvoc_config;
            //tvoc_config.measurement_medium = 0; // 0 = Air
            cluster_t *tvoc_cluster = cluster::total_volatile_organic_compounds_concentration_measurement::create(endpoint, &tvoc_config, CLUSTER_FLAG_SERVER);
            if (tvoc_cluster) {
                // MeasuredValue (обязательный)
                cluster::total_volatile_organic_compounds_concentration_measurement::attribute::create_measured_value(
                    tvoc_cluster, 
                    nullable<float>(0)  // или float, в зависимости от версии
                );
                
                // MinMeasuredValue (опциональный, но часто требуется)
                cluster::total_volatile_organic_compounds_concentration_measurement::attribute::create_min_measured_value(
                    tvoc_cluster, 
                    nullable<float>(0)
                );
                
                // MaxMeasuredValue (опциональный)
                cluster::total_volatile_organic_compounds_concentration_measurement::attribute::create_max_measured_value(
                    tvoc_cluster, 
                    nullable<float>(1000000)
                );
                
                /*
                // MeasurementMedium (обязательный fixed attribute)
                cluster::total_volatile_organic_compounds_concentration_measurement::attribute::create_measurement_medium(
                    tvoc_cluster, 
                    chip::app::Clusters::ConcentrationMeasurement::MeasurementMediumEnum::kAir
                );
                */
            } else {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Failed to create TVOC cluster");
              #endif
            }

            break;
        }

        case SENSOR_TYPE_DS18B20: {
            // Create temperature sensor endpoint
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "*************************************");
              ESP_LOGW("", " Start creating endpoint for DS18B20");
              ESP_LOGW("", "*************************************");
              ESP_LOGW("", "");
            #endif
            temperature_sensor::config_t sensor_config;
            
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-5500); // -55.00°C
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500); // 125.00°C
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            break;
        }

        #if CONFIG_DHT11_ENABLED
        case SENSOR_TYPE_DHT11: {
            // Create endpoint with multiple clusters
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "***********************************");
              ESP_LOGW("", " Start creating endpoint for DHT11");
              ESP_LOGW("", "***********************************");
              ESP_LOGW("", "");
            #endif
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
                        
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Failed to create endpoint for DHT11");
              #endif
              return ESP_FAIL;
            }

            //-- Add Humidity Measurement cluster
            cluster::relative_humidity_measurement::config_t hum_config;
            hum_config.measured_value = nullable<uint16_t>(5000);
            hum_config.min_measured_value = nullable<uint16_t>(0);
            hum_config.max_measured_value = nullable<uint16_t>(10000);
            cluster::relative_humidity_measurement::create(endpoint, &hum_config, CLUSTER_FLAG_SERVER);
            
            break;
        }
        #endif

        #if CONFIG_MQ135_ENABLED
        case SENSOR_TYPE_MQ135: {
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "***********************************");
              ESP_LOGW("", " Start creating endpoint for MQ135");
              ESP_LOGW("", "***********************************");
              ESP_LOGW("", "");
            #endif
            // Создание endpoint с типом Air Quality Sensor
            // Это стандартный тип устройства, который подходит для нашего датчика.
            air_quality_sensor::config_t sensor_config;
            
            endpoint = air_quality_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            if(!endpoint) {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Failed to create endpoint for MQ135");
              #endif
              return ESP_FAIL;
            }

            s_air_quality_cluster = cluster::get(endpoint, AirQuality::Id);
            if(s_air_quality_cluster == NULL) {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Не удалось получить Air Quality Cluster");
              #endif
            }
            // Включить все доступные уровни (FAIR | MOD | VPOOR | XPOOR)
            attribute_t *fm_attr = attribute::get(s_air_quality_cluster, AirQuality::Attributes::FeatureMap::Id);
            if(fm_attr) {
              esp_matter_attr_val_t fm_val = esp_matter_bitmap32(0x0F); // биты 0-3
              attribute::update(endpoint::get_id(endpoint), AirQuality::Id,
                            AirQuality::Attributes::FeatureMap::Id, &fm_val);
              #if DEBUG_MODE
                ESP_LOGW(TAG_MIKE_APP, "Все доступные уровни (FAIR | MOD | VPOOR | XPOOR) Air Quality Cluster включены!");
              #endif
            }
            
            // === 1. Кластер Carbon Dioxide Concentration Measurement (0x040D) ===
            cluster::carbon_dioxide_concentration_measurement::config_t co2_config;
            //co2_config.measurement_medium = 0; // 0 = Air
            s_co2_cluster = cluster::carbon_dioxide_concentration_measurement::create(endpoint, &co2_config, CLUSTER_FLAG_SERVER);
            if (s_co2_cluster) {
                // Включение функции NumericMeasurement (обязательна для MeasuredValue)
                cluster::carbon_dioxide_concentration_measurement::feature::numeric_measurement::config_t co2_feature_cfg;
                co2_feature_cfg.measured_value = 400.0f; // Начальное значение (примерно атмосферный уровень)
                co2_feature_cfg.min_measured_value = 0.0f;
                co2_feature_cfg.max_measured_value = 5000.0f;
                co2_feature_cfg.measurement_unit = 0; // 0 = PPM (согласно MeasurementUnitEnum)
                //co2_feature_cfg.measurement_medium = 0; // 0 = Air
                    
                esp_err_t err = cluster::carbon_dioxide_concentration_measurement::feature::numeric_measurement::add(s_co2_cluster, &co2_feature_cfg);
                if(err != ESP_OK) {
                  #if DEBUG_MODE
                    ESP_LOGE(TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер CO2: %s", esp_err_to_name(err));
                  #endif
                }
            }
            
            // === 2. Кластер Carbon Monoxide Concentration Measurement (0x040C) ===
            cluster::carbon_monoxide_concentration_measurement::config_t co_config;
            //co_config.measurement_medium = 0; // 0 = Air
            s_co_cluster = cluster::carbon_monoxide_concentration_measurement::create(endpoint, &co_config, CLUSTER_FLAG_SERVER);
            if (s_co_cluster) {
                cluster::carbon_monoxide_concentration_measurement::feature::numeric_measurement::config_t co_feature_cfg;
                co_feature_cfg.measured_value = 0.0f;
                co_feature_cfg.min_measured_value = 0.0f;
                co_feature_cfg.max_measured_value = 1000.0f;
                co_feature_cfg.measurement_unit = 0; // PPM
                //co_feature_cfg.measurement_medium = 0; // Air
                    
                esp_err_t err = cluster::carbon_monoxide_concentration_measurement::feature::numeric_measurement::add(s_co_cluster, &co_feature_cfg);
                if(err != ESP_OK) {
                  #if DEBUG_MODE
                    ESP_LOGE(TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер CO: %s", esp_err_to_name(err));
                  #endif
                }
            }
            
            // === 3. Кластер Total Volatile Organic Compounds Concentration Measurement (0x041E) ===
            cluster::total_volatile_organic_compounds_concentration_measurement::config_t tvoc_config;
            //tvoc_config.measurement_medium = 0; // 0 = Air
            s_tvoc_cluster = cluster::total_volatile_organic_compounds_concentration_measurement::create(endpoint, &tvoc_config, CLUSTER_FLAG_SERVER);
            if (s_tvoc_cluster) {
                cluster::total_volatile_organic_compounds_concentration_measurement::feature::numeric_measurement::config_t tvoc_feature_cfg;
                tvoc_feature_cfg.measured_value = 0.0f;
                tvoc_feature_cfg.min_measured_value = 0.0f;
                tvoc_feature_cfg.max_measured_value = 5000.0f;
                tvoc_feature_cfg.measurement_unit = 0; // PPM
                //tvoc_feature_cfg.measurement_medium = 0; // Air
                    
                esp_err_t err = cluster::total_volatile_organic_compounds_concentration_measurement::feature::numeric_measurement::add(s_tvoc_cluster, &tvoc_feature_cfg);
                if(err != ESP_OK) {
                  #if DEBUG_MODE
                    ESP_LOGE(TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер TVOC: %s", esp_err_to_name(err));
                  #endif
                }
            }
            
            break;
        }
        #endif

        #if CONFIG_SHT41_ENABLED
        case SENSOR_TYPE_SHT41: {
            // Create endpoint with multiple clusters
            #if DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", " Start creating endpoint for SHT41");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", "");
            #endif
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);  // 20.00°C
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000); // -40.00°C
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500); // 125.00°C
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              #if DEBUG_MODE
                ESP_LOGE(TAG_MIKE_APP, "Failed to create endpoint for SHT41");
              #endif
              return ESP_FAIL;
            }

            //-- Add Humidity Measurement cluster
            cluster::relative_humidity_measurement::config_t hum_config;
            hum_config.measured_value = nullable<uint16_t>(5000); // 50.00%
            hum_config.min_measured_value = nullable<uint16_t>(0); // 0.00%
            hum_config.max_measured_value = nullable<uint16_t>(10000); // 100.00%
            cluster::relative_humidity_measurement::create(endpoint, &hum_config, CLUSTER_FLAG_SERVER);

            break;
        }
        #endif

        default:
          #if DEBUG_MODE
            ESP_LOGE(TAG_MIKE_APP, "Unknown sensor type: %d, total sensors: %d", sensor_cfg, SENSOR_TYPE_MAX);
          #endif
          return ESP_ERR_INVALID_ARG;
    }

    if(!endpoint) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Matter endpoint creation failed for sensor type: %d", sensor_cfg);
      #endif
      return ESP_FAIL;
    }

    // Initialize sensor hardware
    err = app_driver_sensor_init(sensor_cfg);
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Failed to initialize sensor type: %d", sensor_cfg);
      #endif
      return err;
    }

    // Store mapping
    if (configured_sensors < SENSOR_TYPE_MAX) {
        sensor_mapping_list[configured_sensors].endpoint_id = endpoint::get_id(endpoint);
        sensor_mapping_list[configured_sensors].sensor_type = sensor_cfg;
        //sensor_mapping_list[configured_sensors].primary_gpio = sensor_cfg->sda_pin;
        //sensor_mapping_list[configured_sensors].secondary_gpio = sensor_cfg->scl_pin;
        
        // Store in sensors array
        sensors[configured_sensors].config = sensor_cfg;
        sensors[configured_sensors].last_temperature = 0;
        sensors[configured_sensors].last_humidity = 0;
        sensors[configured_sensors].last_pressure = 0;
        sensors[configured_sensors].last_gas_resistance = 0;
        sensors[configured_sensors].last_co2 = 0;
        sensors[configured_sensors].last_co = 0;
        sensors[configured_sensors].last_tvoc = 0;
        sensors[configured_sensors].last_read_time = 0;
        sensors[configured_sensors].num_sensors = 1;
        
        configured_sensors++;
        
        #if DEBUG_MODE
          ESP_LOGI(TAG_MIKE_APP, "Sensor %d created with endpoint_id %d", sensor_cfg, endpoint::get_id(endpoint));
        #endif
    } else {
        #if DEBUG_MODE
          ESP_LOGE(TAG_MIKE_APP, "Maximum sensors configuration limit exceeded!");
        #endif
        return ESP_FAIL;
    }

    return err;
}

void set_basic_attributes_esp_matter()
{
    uint16_t endpoint_id = 0x0000;

    // Set NodeLabel
    char node_label[] = "MultiClimate_ESP32H2";
    esp_matter_attr_val_t node_label_val = esp_matter_char_str(node_label, strlen(node_label));
    esp_err_t err = esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::NodeLabel::Id,
        &node_label_val
    );
    
    if(err == ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGI(TAG_MIKE_APP, "NodeLabel set via ESP-Matter API");
      #endif
    } else {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Failed to set NodeLabel: %d", err);
      #endif
    }
    
}

//--- Задача: раз в секунду мигать, пока открыто окно сопряжения ---
static void commissioning_window_blink_task(void *arg)
{
  uint32_t window_sec = (uint32_t)(uintptr_t)arg;
  uint32_t elapsed = 0;

  ESP_LOGW("#1# CommWindow", "");
  ESP_LOGW("#1# CommWindow", "##############################");
  ESP_LOGW("#1# CommWindow", "Blink task started, window = %lu s", window_sec);
  ESP_LOGW("#1# CommWindow", "##############################");
  ESP_LOGW("#1# CommWindow", "");

  while(elapsed < window_sec) {
    // Проверяем, не закрылось ли окно раньше (например, устройство уже сопряжено)
    auto &mgr = chip::Server::GetInstance().GetCommissioningWindowManager();
    if(!mgr.IsCommissioningWindowOpen()) {
      ESP_LOGW("#2# CommWindow", "");
      ESP_LOGW("#2# CommWindow", "##############################");
      ESP_LOGW("#2# CommWindow", "Window closed early at %lu s, stop blink", elapsed);
      ESP_LOGW("#2# CommWindow", "##############################");
      ESP_LOGW("#2# CommWindow", "");
      break;
    }

    #if USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_ORANGE, 60, 0);
    #endif

    vTaskDelay(pdMS_TO_TICKS(1000));
    elapsed++;
  }

  ESP_LOGW("#3# CommWindow", "");
  ESP_LOGW("#3# CommWindow", "##############################");
  ESP_LOGW("#3# CommWindow", "Blink task finished after %lu s", elapsed);
  ESP_LOGW("#3# CommWindow", "##############################");
  ESP_LOGW("#3# CommWindow", "");
  vTaskDelete(NULL);
}


// ==================================================
//  Создание Generic Switch endpoint для кнопки BOOT
// ==================================================
#if CONFIG_BOOT_SWITCH_ENABLED
static esp_err_t create_boot_switch_endpoint(node_t *node)
{
    if (node == NULL) {
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch: node is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    //-- 1. Создаём endpoint Generic Switch с пустой конфигурацией
    //--    (функции будут добавлены отдельно через add())
    generic_switch::config_t switch_config;
    endpoint_t *endpoint = generic_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, NULL);
    if (endpoint == NULL) {
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to create endpoint");
        return ESP_FAIL;
    }

    uint16_t ep_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG_MIKE_APP, "Boot Switch: endpoint created, id=%d", ep_id);

    //-- 2. Получаем кластер Switch для добавления функций
    cluster_t *switch_cluster = cluster::get(endpoint, Switch::Id);
    if (switch_cluster == NULL) {
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to get Switch cluster");
        return ESP_FAIL;
    }

    //-- 3. Добавляем функцию Momentary Switch (MS)
    //--    Обязательна для генерации событий InitialPress/ShortRelease.
    //--    Без неё остальные функции не имеют смысла.
    esp_err_t err = cluster::switch_cluster::feature::momentary_switch::add(switch_cluster);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to add momentary_switch: %d", err);
        return err;
    }

    //-- 4. Добавляем функцию Momentary Switch Release (MSR)
    //--    ИСПРАВЛЕНО: правильное имя — momentary_switch_release.
    //--    Необходима для ShortRelease, LongRelease и MultiPress.
    //--    Согласно спецификации Matter, MSR требует наличия MS[citation:2][citation:10].
    err = cluster::switch_cluster::feature::momentary_switch_release::add(switch_cluster);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "Boot Switch: failed to add momentary_switch_release: %d", err);
        return err;
    }

    //-- 5. Добавляем функцию Long Press (MSL)
    //--    Для генерации событий LongPress/LongRelease.
    //--    Требует наличия MSR[citation:5].
    err = cluster::switch_cluster::feature::momentary_switch_long_press::add(switch_cluster);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MIKE_APP, "Boot Switch: failed to add long_press: %d (продолжаем)", err);
        //-- Не критично: можно продолжить без LongPress
    }

    //-- 6. Добавляем функцию Multi Press (MSM)
    //--    Для генерации событий MultiPressOngoing/MultiPressComplete (двойной клик).
    //--    Требует наличия MSR[citation:5].
    cluster::switch_cluster::feature::momentary_switch_multi_press::config_t msm_config;
    msm_config.multi_press_max = 2;  //-- максимум 2 нажатия (двойной клик)
    err = cluster::switch_cluster::feature::momentary_switch_multi_press::add(switch_cluster, &msm_config);
    if (err != ESP_OK) {
        ESP_LOGW(TAG_MIKE_APP, "Boot Switch: failed to add multi_press: %d (продолжаем)", err);
        //-- Не критично: можно продолжить без двойного клика
    }

    //-- 7. Передаём endpoint_id и указатель на кластер в драйвер boot_switch
    //--    (функция объявлена в driver_boot_switch.cpp)
    boot_switch_set_endpoint(ep_id, switch_cluster);

    ESP_LOGI(TAG_MIKE_APP, "Boot Switch: endpoint fully configured (MS, MSR, LongPress, MultiPress)");
    return ESP_OK;
}
#endif

// ===========================================================
//  On/Off эндпоинты для управления экраном, лентой и ребутом
// ===========================================================

//-- Глобальные ID для On/Off эндпоинтов (чтобы app_driver_attribute_update мог их найти)
#if CONFIG_BOOT_SWITCH_ENABLED
  uint16_t s_ep_screen_onoff = 0;
  uint16_t s_ep_led_onoff = 0;
  uint16_t s_ep_reboot_onoff = 0;
#endif

#if CONFIG_BOOT_SWITCH_ENABLED
static esp_err_t create_onoff_endpoints(node_t *node)
{
    if (node == NULL) return ESP_ERR_INVALID_ARG;

    //-- 1. On/Off для экрана SSD1306
    #if CONFIG_SSD1306_ENABLED
    on_off_light::config_t screen_config;
    endpoint_t *screen_ep = on_off_light::create(node, &screen_config, ENDPOINT_FLAG_NONE, NULL);
    if(screen_ep) {
      s_ep_screen_onoff = endpoint::get_id(screen_ep);
      ESP_LOGW(TAG_MIKE_APP, "");
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_screen_onoff);
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "");
    } else {
    	ESP_LOGE(TAG_MIKE_APP, "");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, "");
    }
    #endif

    //-- 2. On/Off для LED ленты (блокировка/разблокировка)
    #if USE_DRIVER_LED_INDICATOR
    on_off_light::config_t led_config;
    // ============================================================
    // Начальное состояние ON — аналогично тому, что делает
    // физическая кнопка по doublepress (toggle_led_indicator_lock).
    // При старте прошивки LED Indicator должен быть РАЗБЛОКИРОВАН.
    // ============================================================
    led_config.on_off.on_off = true;
    endpoint_t *led_ep = on_off_light::create(node, &led_config, ENDPOINT_FLAG_NONE, NULL);
    if(led_ep) {
      s_ep_led_onoff = endpoint::get_id(led_ep);
      ESP_LOGW(TAG_MIKE_APP, "");
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_led_onoff);
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "");
    } else {
    	ESP_LOGE(TAG_MIKE_APP, "");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, "");
    }
    #endif

    //-- 3. On/Off для перезагрузки (On = перезагрузка, Off = игнор)
    //-- Используем on_off_plugin_unit как "кнопку" — HA увидит Switch
    on_off_plugin_unit::config_t reboot_config;
    endpoint_t *reboot_ep = on_off_plugin_unit::create(node, &reboot_config, ENDPOINT_FLAG_NONE, NULL);
    if(reboot_ep) {
      s_ep_reboot_onoff = endpoint::get_id(reboot_ep);
      ESP_LOGW(TAG_MIKE_APP, "");
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "On/Off Screen endpoint created: id=%d", s_ep_reboot_onoff);
      ESP_LOGW(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGW(TAG_MIKE_APP, "");
    } else {
    	ESP_LOGE(TAG_MIKE_APP, "");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, " On/Off Screen endpoint NOT created!");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, "");
    }

    return ESP_OK;
}
#endif

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;

  // 1. NVS — первым делом
  err = nvs_flash_init();
  if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    err = nvs_flash_erase();
    if(err != ESP_OK) {
    	return;
    }
  }
  err = nvs_flash_init();
  if(err != ESP_OK) {
  	#if DEBUG_MODE
  	  ESP_LOGE(TAG_MIKE_APP, "Failed to initialize NVS Flash: %d (%s)", err, esp_err_to_name(err));
  	#endif
  	return;
  }

  
  // 2. i2cdev — до I2C устройств
  err = i2cdev_init();
  if(err != ESP_OK) {
    #if DEBUG_MODE
      ESP_LOGE(TAG_MIKE_APP, "Failed to initialize i2cdev: %d (%s)", err, esp_err_to_name(err));
    #endif
    // Можно продолжить, но I2C устройства не будут работать.
  }

  
  // 3. LED indicator
  #if USE_DRIVER_LED_INDICATOR
    led_handle = configure_indicator();
    if(!led_handle) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Failed to configure LED indicator");
      #endif
    } else {
      //=== при старте LED выключен ===
      //-- Синхронизируем глобальный флаг с driver_led_indicator.cpp:
      //-- при загрузке функция get_led_indicator_blink_idx заблокирована,
      //-- поэтому любые попытки мигнуть будут игнорироваться.
      extern bool led_indicator_locked;   //-- определён в driver_led_indicator.cpp
      //led_indicator_locked = true;
      led_indicator_locked = false;

      //-- Гасим RGB-ленту через штатный шаг BLINK_OFF.
      //-- ВАЖНО: используем именно BLINK_OFF, а не BLINK_MAX —
      //-- BLINK_MAX это размер массива led_mode[], а не валидный индекс.
      led_indicator_start(led_handle, BLINK_OFF);

      ESP_LOGE("", "");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", " USE_DRIVER_LED_INDICATOR = Y");
      #if USE_RGB_LED
        ESP_LOGE("", " USE_RGB_LED (%d) = Y", LED_BLINK_GPIO);
      #endif
      ESP_LOGE("", " LED state at boot: OFF (locked)");
      ESP_LOGE("", "==============================");
      ESP_LOGE("", "");
    }
  #endif

  
  // 4. SSD1306
  #if CONFIG_SSD1306_ENABLED
    //-- Init LCD SSD1306
    err = ssd1306_init();
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "~~~ Error initialize SSD1306!");
      #endif
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      #endif
    } else {
      ssd1306_initialized = true;
      #if DEBUG_MODE
        ESP_LOGW(TAG_MIKE_APP, "~~~ SSD1306 Initialized!");
      #endif
      #if USE_DRIVER_LED_INDICATOR
        get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      #endif
    }
    //=== При загрузке экран ВЫКЛЮЧЕН ===
    //ssd1306_show_title();                 // было: сразу показываем заголовок
    ssd1306_display_enabled = false;        // стало: экран выключен
    ssd1306_clear_screen(&ssd1306dev, false);
    //ssd1306_display_off(&ssd1306dev);       // гасим панель
    ssd1306_contrast(&ssd1306dev, 0x00);    // программное выключение панели
  #endif

  
  // 5. Reboot button
  #if CONFIG_REBOOT_BUTTON_ENABLED
    xTaskCreate(reboot_button_task, "reboot_button_task", 4096, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  #endif

    
  // 6. Console
  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    ESP_LOGE("", "");
    ESP_LOGE("", "===========================================");
    ESP_LOGE("", " Free heap: %" PRIu32 ", Largest block: %d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
    ESP_LOGE("", "===========================================");
    ESP_LOGE("", "");
    err = esp_matter::console::init();
    if(err != ESP_OK) {
      #if DEBUG_MODE
        ESP_LOGE(TAG_MIKE_APP, "Console init failed: %d", err);
      #endif
    }
  #endif


  // 7. Matter node + endpoints
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MIKE_APP, "Failed to create Matter node"));
  #if CONFIG_BME280_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_BME280, node);
  #endif
  #if CONFIG_BME680_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_BME680, node);
  #endif
  #if CONFIG_DS18B20_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_DS18B20, node);
  #endif
  #if CONFIG_DHT11_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_DHT11, node);
  #endif
  #if CONFIG_MQ135_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_MQ135, node);
  #endif
  #if CONFIG_SHT41_ENABLED
    create_sensor_endpoint(SENSOR_TYPE_SHT41, node);
  #endif
  // =========================================
  //  Boot Switch endpoint и On/Off эндпоинты
  // =========================================
  #if CONFIG_BOOT_SWITCH_ENABLED
    create_boot_switch_endpoint(node);
      
      ESP_LOGE(TAG_MIKE_APP, "");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, " ??? On/Off ??? ");
      ESP_LOGE(TAG_MIKE_APP, "~~~~~~~~~~~~~~~~");
      ESP_LOGE(TAG_MIKE_APP, "");

    create_onoff_endpoints(node);
  #endif
  
  // 8. OpenThread config
  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif

  
  // 9. Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MIKE_APP, "Failed to start Matter, err:%d", err));


  // 9. Open commissioning windows after factory reset and reboot
  #if CONFIG_REBOOT_BUTTON_ENABLED
    //--> uses for CommissioningWindowOpen
    chip::DeviceLayer::PlatformMgr().ScheduleWork([](intptr_t context) {
      auto &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
      // Check if the pairing window is already open, avoid opening it again
      if(!commissionMgr.IsCommissioningWindowOpen()) {
        CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(
          // Window opens for CONFIG_COMMISSIONING_WINDOW_OPEN seconds
          chip::System::Clock::Seconds16(CONFIG_COMMISSIONING_WINDOW_OPEN)
        );
        if(err != CHIP_NO_ERROR) {
          ESP_LOGE("", "");
          ESP_LOGE("", "-------------------------------------------------");
          ESP_LOGE("", "Failed to open commissioning window: %" CHIP_ERROR_FORMAT, err.Format());
          ESP_LOGE("", "-------------------------------------------------");
          ESP_LOGE("", "");
        } else {
          ESP_LOGW("", "");
          ESP_LOGW("", "------------------------------------------------------------");
          ESP_LOGW("", " Commissioning window opened after factory reset for %d s   ", CONFIG_COMMISSIONING_WINDOW_OPEN);
          ESP_LOGW("", "------------------------------------------------------------");
          ESP_LOGW("", "");
        }
      } else {
      	// Запускаем задачу мигания (в стеке 2 KB достаточно)
        xTaskCreate(commissioning_window_blink_task, "comm_blink", 2048, (void *)(uintptr_t)CONFIG_COMMISSIONING_WINDOW_OPEN, CONFIG_COMMISSIONING_WINDOW_PRIORITY, NULL);
      }
    }, 0);
    //<-- uses for CommissioningWindowOpen
  #endif
  
  // 10. Get FabricId & NodeId
  ESP_LOGW("", "");
  ESP_LOGW("", "===========================================");
  chip::FabricTable &fabricTable = chip::Server::GetInstance().GetFabricTable();
  for(auto iter = fabricTable.begin(); iter != fabricTable.end(); ++iter) {
    ESP_LOGW("", "FabricId 0x%llx NodeId 0x%llx", iter->GetFabricId(), iter->GetNodeId());
  }
  ESP_LOGW("", "===========================================");
  ESP_LOGW("", "");


  // 10. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  // ==================================
  //  Запуск задачи опроса кнопки BOOT
  // ==================================
  #if CONFIG_BOOT_SWITCH_ENABLED
    //-- Инициализируем драйвер (передаём endpoint)
    boot_switch_driver_init();

    //-- Запускаем задачу (приоритет ниже, чем reboot_button_task, чтобы не мешать)
    xTaskCreate(boot_switch_task, "boot_switch", 3072, NULL, CONFIG_BOOT_SWITCH_TASK_PRIORITY, NULL);
    ESP_LOGI(TAG_MIKE_APP, "Boot Switch task created");
  #endif

  // =================================================
  // Явно выставляем OnOff = ON для LED эндпоинта,
  // чтобы HA сразу видел разблокированное состояние.
  // =================================================
  #if CONFIG_BOOT_SWITCH_ENABLED && USE_DRIVER_LED_INDICATOR
    if(s_ep_led_onoff != 0) {
      esp_matter_attr_val_t val = esp_matter_bool(true);
      esp_matter::attribute::update(
        s_ep_led_onoff,
        OnOff::Id,
        OnOff::Attributes::OnOff::Id,
        &val
      );
      ESP_LOGI(TAG_MIKE_APP, "LED OnOff attribute forced to ON after start");
    }
  #endif
  
  // 11. Polling task
  xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, CONFIG_SENSOR_POLL_TASK_PRIORITY, NULL);

  
  // 12. Log
  #if SHOW_DEVICE_LOG
    if(node) {
      vTaskDelay(pdMS_TO_TICKS(3000));
      log_device_structure(node);
    }
  #endif
}
