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

#include "app_main_driver.h"
#include "i2c_adv.h"

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
  //TaskHandle_t s_update_task_handle = NULL;
#endif
#if CONFIG_SHT4X_ENABLED
  #include "sensor_driver_sht4x.h"
#endif
#if CONFIG_SCD4X_ENABLED
  #include "sensor_driver_scd4x.h"
#endif


#if CONFIG_BOOT_SWITCH_ENABLED
  #include "driver_boot_switch.h"
#endif

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

//--> uses for CommissioningWindowOpen
//constexpr auto k_timeout_seconds = CONFIG_COMMISSIONING_WINDOW_OPEN;
//<-- uses for CommissioningWindowOpen
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[SENSOR_TYPE_MAX];
sensor_data_t sensors[SENSOR_TYPE_MAX];


/*
#if CONFIG_SSD1306_ENABLED
  //-- SSD1306 device instance
  SSD1306_t ssd1306dev;
  //-- Is SSD1306 initialized?
  bool ssd1306_initialized = false;
  //=== состояние дисплея (по умолчанию — ВЫКЛЮЧЕН) ===
  bool ssd1306_display_enabled = false;
  uint64_t ssd1306_last_on_time_ms = 0;
#endif
*/

#if USE_DRIVER_LED_INDICATOR
  led_indicator_handle_t led_handle;
#endif

#if CONFIG_SHOW_DEVICE_LOG
  #include "plugin_device_structure.h"
#endif

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
        #if CONFIG_DEBUG_MODE
          ESP_LOGW(CONFIG_TAG_MIKE_APP, "Sensor type %d already configured", sensor_cfg);
        #endif
        return ESP_ERR_INVALID_STATE;
      }
    }

    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg) {
        case SENSOR_TYPE_BME280: {
            // Create endpoint with multiple clusters
            #if CONFIG_DEBUG_MODE
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
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for BME280");
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
            #if CONFIG_DEBUG_MODE
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
                #if CONFIG_DEBUG_MODE
                  ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for BME680");
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
            } else {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create TVOC cluster");
              #endif
            }

            break;
        }

        case SENSOR_TYPE_DS18B20: {
            // Create temperature sensor endpoint
            #if CONFIG_DEBUG_MODE
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

        //#if CONFIG_DHT11_ENABLED
        case SENSOR_TYPE_DHT11: {
            // Create endpoint with multiple clusters
            #if CONFIG_DEBUG_MODE
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
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for DHT11");
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
        //#endif

        //#if CONFIG_MQ135_ENABLED
        case SENSOR_TYPE_MQ135: {
            #if CONFIG_DEBUG_MODE
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
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for MQ135");
              #endif
              return ESP_FAIL;
            }

            s_air_quality_cluster = cluster::get(endpoint, AirQuality::Id);
            if(s_air_quality_cluster == NULL) {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Не удалось получить Air Quality Cluster");
              #endif
            }
            // Включить все доступные уровни (FAIR | MOD | VPOOR | XPOOR)
            attribute_t *fm_attr = attribute::get(s_air_quality_cluster, AirQuality::Attributes::FeatureMap::Id);
            if(fm_attr) {
              esp_matter_attr_val_t fm_val = esp_matter_bitmap32(0x0F); // биты 0-3
              attribute::update(endpoint::get_id(endpoint), AirQuality::Id,
                            AirQuality::Attributes::FeatureMap::Id, &fm_val);
              #if CONFIG_DEBUG_MODE
                ESP_LOGW(CONFIG_TAG_MIKE_APP, "Все доступные уровни (FAIR | MOD | VPOOR | XPOOR) Air Quality Cluster включены!");
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
                  #if CONFIG_DEBUG_MODE
                    ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер CO2: %s", esp_err_to_name(err));
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
                  #if CONFIG_DEBUG_MODE
                    ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер CO: %s", esp_err_to_name(err));
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
                  #if CONFIG_DEBUG_MODE
                    ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер TVOC: %s", esp_err_to_name(err));
                  #endif
                }
            }
            
            break;
        }
        //#endif

        //#if CONFIG_SHT4X_ENABLED
        case SENSOR_TYPE_SHT4X: {
            // Create endpoint with multiple clusters
            #if CONFIG_DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", " Start creating endpoint for SHT4X");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", "");
            #endif
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);  // 20.00°C
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000); // -40.00°C
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500); // 125.00°C
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for SHT4X");
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
        //#endif


        //#if CONFIG_SCD4X_ENABLED
        case SENSOR_TYPE_SCD4X: {
            // Create endpoint with multiple clusters
            #if CONFIG_DEBUG_MODE
              ESP_LOGW("", "");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", " Start creating endpoint for SCD4X");
              ESP_LOGW("", "************************************");
              ESP_LOGW("", "");
            #endif
            
            // Создание endpoint с типом Air Quality Sensor
            // Это стандартный тип устройства, который подходит для нашего датчика.
            air_quality_sensor::config_t sensor_config;
            
            endpoint = air_quality_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            if(!endpoint) {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create endpoint for SCD4X");
              #endif
              return ESP_FAIL;
            }

            cluster_t *scd4x_aq_cluster = cluster::get(endpoint, AirQuality::Id);
            if(scd4x_aq_cluster == NULL) {
              #if CONFIG_DEBUG_MODE
                ESP_LOGE(CONFIG_TAG_MIKE_APP, "Не удалось получить Air Quality Cluster");
              #endif
            }
            // Включить все доступные уровни (FAIR | MOD | VPOOR | XPOOR)
            attribute_t *fm_attr = attribute::get(scd4x_aq_cluster, AirQuality::Attributes::FeatureMap::Id);
            if(fm_attr) {
              esp_matter_attr_val_t fm_val = esp_matter_bitmap32(0x0F); // биты 0-3
              attribute::update(endpoint::get_id(endpoint), AirQuality::Id,
                            AirQuality::Attributes::FeatureMap::Id, &fm_val);
              #if CONFIG_DEBUG_MODE
                ESP_LOGW(CONFIG_TAG_MIKE_APP, "Все доступные уровни (FAIR | MOD | VPOOR | XPOOR) Air Quality Cluster включены!");
              #endif
            }
            
            // === 1. Кластер Carbon Dioxide Concentration Measurement (0x040D) ===
            cluster::carbon_dioxide_concentration_measurement::config_t co2_config;
            cluster_t *scd4x_co2_cluster = cluster::carbon_dioxide_concentration_measurement::create(endpoint, &co2_config, CLUSTER_FLAG_SERVER);
            if(scd4x_co2_cluster) {
              // Включение функции NumericMeasurement (обязательна для MeasuredValue)
              cluster::carbon_dioxide_concentration_measurement::feature::numeric_measurement::config_t co2_feature_cfg;
              co2_feature_cfg.measured_value = 400.0f; // Начальное значение (примерно атмосферный уровень)
              co2_feature_cfg.min_measured_value = 0.0f;
              co2_feature_cfg.max_measured_value = 5000.0f;
              co2_feature_cfg.measurement_unit = 0; // 0 = PPM (согласно MeasurementUnitEnum)
                  
              esp_err_t err = cluster::carbon_dioxide_concentration_measurement::feature::numeric_measurement::add(scd4x_co2_cluster, &co2_feature_cfg);
              if(err != ESP_OK) {
                #if CONFIG_DEBUG_MODE
                  ESP_LOGE(CONFIG_TAG_MIKE_APP, "Ошибка добавления фичи NumericMeasurement в кластер CO2: %s", esp_err_to_name(err));
                #endif
              }
            }

            //-- Add Humidity Measurement cluster
            cluster::temperature_measurement::config_t temp_config;
            temp_config.measured_value = nullable<int16_t>(2000);  // 20.00°C
            temp_config.min_measured_value = nullable<int16_t>(-4000); // -40.00°C
            temp_config.max_measured_value = nullable<int16_t>(12500); // 125.00°C
            cluster::temperature_measurement::create(endpoint, &temp_config, CLUSTER_FLAG_SERVER);
            
            //-- Add Humidity Measurement cluster
            cluster::relative_humidity_measurement::config_t hum_config;
            hum_config.measured_value = nullable<uint16_t>(5000); // 50.00%
            hum_config.min_measured_value = nullable<uint16_t>(0); // 0.00%
            hum_config.max_measured_value = nullable<uint16_t>(10000); // 100.00%
            cluster::relative_humidity_measurement::create(endpoint, &hum_config, CLUSTER_FLAG_SERVER);

            break;
        }
        //#endif

        default:
          #if CONFIG_DEBUG_MODE
            ESP_LOGE(CONFIG_TAG_MIKE_APP, "Unknown sensor type: %d, total sensors: %d", sensor_cfg, SENSOR_TYPE_MAX);
          #endif
          return ESP_ERR_INVALID_ARG;
    }

    if(!endpoint) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Matter endpoint creation failed for sensor type: %d", sensor_cfg);
      #endif
      return ESP_FAIL;
    }

    // Initialize sensor hardware
    err = app_driver_sensor_init(sensor_cfg);
    if(err != ESP_OK) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize sensor type: %d", sensor_cfg);
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
        
        #if CONFIG_DEBUG_MODE
          ESP_LOGI(CONFIG_TAG_MIKE_APP, "Sensor %d created with endpoint_id %d", sensor_cfg, endpoint::get_id(endpoint));
        #endif
    } else {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "Maximum sensors configuration limit exceeded!");
        #endif
        return ESP_FAIL;
    }

    return err;
}


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
  	#if CONFIG_DEBUG_MODE
  	  ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize NVS Flash: %d (%s)", err, esp_err_to_name(err));
  	#endif
  	return;
  }

  
  // 2. i2cdev — до I2C устройств
  err = i2cdev_init();
  if(err != ESP_OK) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to initialize i2cdev: %d (%s)", err, esp_err_to_name(err));
    #endif
    // Можно продолжить, но I2C устройства не будут работать.
  }

  
  // 3. LED indicator
  #if USE_DRIVER_LED_INDICATOR
    led_indicator_init_main();
  #endif

  
  // 4. SSD1306
  #if CONFIG_SSD1306_ENABLED
    //-- Init LCD SSD1306
    ssd1306_init_app_main();
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
    #if CONFIG_DEBUG_MODE
      ESP_LOGE("", "");
      ESP_LOGE("", "===========================================");
      ESP_LOGE("", " Free heap: %" PRIu32 ", Largest block: %d", esp_get_free_heap_size(), heap_caps_get_largest_free_block(MALLOC_CAP_8BIT));
      ESP_LOGE("", "===========================================");
      ESP_LOGE("", "");
    #endif
    err = esp_matter::console::init();
    if(err != ESP_OK) {
      #if CONFIG_DEBUG_MODE
        ESP_LOGE(CONFIG_TAG_MIKE_APP, "Console init failed: %d", err);
      #endif
    }
  #endif


  // 7. Matter node + endpoints
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to create Matter node"));

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//!!                                                              !!
//!!  Key endpoints must be created unconditionally -             !!
//!!  so as not to disrupt their order within the node structure  !!
//!!                                                              !!
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  create_sensor_endpoint(SENSOR_TYPE_BME280, node);
  create_sensor_endpoint(SENSOR_TYPE_BME680, node);
  create_sensor_endpoint(SENSOR_TYPE_DS18B20, node);
  create_sensor_endpoint(SENSOR_TYPE_DHT11, node);
  create_sensor_endpoint(SENSOR_TYPE_MQ135, node);
  create_sensor_endpoint(SENSOR_TYPE_SHT4X, node);
  create_sensor_endpoint(SENSOR_TYPE_SCD4X, node);

  
  // =========================================
  //  Boot Switch endpoint и On/Off эндпоинты
  // =========================================
  #if CONFIG_BOOT_SWITCH_ENABLED
    create_boot_switch_endpoint(node);
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
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(CONFIG_TAG_MIKE_APP, "Failed to start Matter, err:%d", err));


  // 10. Open commissioning windows after factory reset and reboot
  #if CONFIG_REBOOT_BUTTON_ENABLED
    open_commissioning_window_after_factory_reset();
  #endif

  
  // 11. Get FabricId & NodeId
  #if CONFIG_DEBUG_MODE
    if(node) {
      ESP_LOGW("", "");
      ESP_LOGW("", "=== FabricId & NodeId =====================");
      chip::FabricTable &fabricTable = chip::Server::GetInstance().GetFabricTable();
      for(auto iter = fabricTable.begin(); iter != fabricTable.end(); ++iter) {
        ESP_LOGW("", "FabricId 0x%llx NodeId 0x%llx", iter->GetFabricId(), iter->GetNodeId());
      }
      ESP_LOGW("", "===========================================");
      ESP_LOGW("", "");
    }
  #endif


  // 12. Basic attributes (лучше — по событию)
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();


  // 13. Запуск задачи опроса кнопки BOOT
  #if CONFIG_BOOT_SWITCH_ENABLED
    start_boot_switch_polling_task();
  #endif

  
  // 14. Выставляем OnOff статус
  // ====================================================
  // Явно выставляем OnOff = ON (true) для LED эндпоинта,
  // чтобы HA сразу видел разблокированное состояние.
  // ====================================================
  #if CONFIG_BOOT_SWITCH_ENABLED && USE_DRIVER_LED_INDICATOR
    set_led_onoff_attribute(true);
  #endif
  

  // 15. Polling task
  xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, CONFIG_SENSOR_POLL_TASK_PRIORITY, NULL);

  
  // 16. Log
  #if CONFIG_SHOW_DEVICE_LOG
    if(node) {
      vTaskDelay(pdMS_TO_TICKS(3000));
      log_device_structure(node);
    }
  #endif

  // 17. Check I2C ports
  #if CONFIG_CHECK_I2C_PORTS
    check_i2c_ports();
  #endif

}
