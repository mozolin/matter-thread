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
#include "sensor_driver_dht11.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
sensor_data_t sensors[CONFIG_NUM_SENSORS];

#if USE_SSD1306_DRIVER
  //-- SSD1306 device instance
  //SSD1306_t ssd1306dev;
  ssd1306_config_t ssd1306_config = SSD1306_128x64_CONFIG_DEFAULT;
  ssd1306_handle_t ssd1306_handle;
  //-- Is SSD1306 initialized?
  bool ssd1306_initialized = false;
#endif

/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
/*
led_indicator_handle_t led_handle;

blink_step_t const *led_mode[] = {
  [BLINK_ON_YELLOW] = yellow_on,
  [BLINK_ON_ORANGE] = orange_on,
  [BLINK_DOUBLE_RED] = double_red_blink,
  [BLINK_TRIPLE_GREEN] = triple_green_blink,
  [BLINK_ONCE_RED] = red_once_blink,
  [BLINK_ONCE_GREEN] = green_once_blink,
  [BLINK_ONCE_BLUE] = blue_once_blink,
  [BLINK_ONCE_LIVE] = live_once_blink,
  [BLINK_WHITE_BREATHE_SLOW] = breath_white_slow_blink,
  [BLINK_WHITE_BREATHE_FAST] = breath_white_fast_blink,
  [BLINK_BLUE_BREATH] = breath_blue_blink,
  [BLINK_COLOR_HSV_RING] = color_hsv_ring_blink,
  [BLINK_COLOR_RGB_RING] = color_rgb_ring_blink,
  #if LED_NUMBERS > 1
    [BLINK_FLOWING] = flowing_blink,
  #endif
  [BLINK_MAX] = NULL,
};

uint8_t get_led_indicator_blink_idx(uint8_t blink_type, int start_delay, int stop_delay)
{
  uint8_t idx = 255;
  
  int size = sizeof(led_mode)/sizeof(led_mode[0]);

  auto item = led_mode[blink_type];
  for(int i=0; i<size; i++) {
    if(led_mode[i] == item) {
      //ESP_LOGW(TAG_MULTI_SENSOR, "~~~ ###!!!@@@ FOUND: %d", i);
      idx = i;

      if(start_delay > 0) {
        led_indicator_start(led_handle, idx);
        //vTaskDelay(pdMS_TO_TICKS(start_delay));
        vTaskDelay(start_delay / portTICK_PERIOD_MS);

        led_indicator_stop(led_handle, idx);
        if(stop_delay > 0) {
          //vTaskDelay(pdMS_TO_TICKS(stop_delay));
          vTaskDelay(stop_delay / portTICK_PERIOD_MS);
        }
      }

      break;
    }
  }

  return idx;
}
*/

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG_MULTI_SENSOR, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG_MULTI_SENSOR, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG_MULTI_SENSOR, "Fabric removed successfully");
        if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
            chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
            constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
            if (!commissionMgr.IsCommissioningWindowOpen()) {
                CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                                            chip::CommissioningWindowAdvertisement::kDnssdOnly);
                if (err != CHIP_NO_ERROR) {
                    ESP_LOGE(TAG_MULTI_SENSOR, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                }
            }
        }
        break;
    }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG_MULTI_SENSOR, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG_MULTI_SENSOR, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG_MULTI_SENSOR, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG_MULTI_SENSOR, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG_MULTI_SENSOR, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
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
        ESP_LOGW(TAG_MULTI_SENSOR, "Sensor type %d already configured", sensor_cfg);
        return ESP_ERR_INVALID_STATE;
      }
    }

    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg) {
        case SENSOR_TYPE_BME280: {
            // Create endpoint with multiple clusters
            ESP_LOGW("", "");
            ESP_LOGW("", "************************************");
            ESP_LOGW("", " Start creating endpoint for BME280");
            ESP_LOGW("", "************************************");
            ESP_LOGW("", "");
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create endpoint for BME280");
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
            ESP_LOGW("", "");
            ESP_LOGW("", "************************************");
            ESP_LOGW("", " Start creating endpoint for BME680");
            ESP_LOGW("", "************************************");
            ESP_LOGW("", "");
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
                        
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if (!endpoint) {
                ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create endpoint for BME680");
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
                ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create TVOC cluster");
            }

            break;
        }

        case SENSOR_TYPE_DS18B20: {
            // Create temperature sensor endpoint
            ESP_LOGW("", "");
            ESP_LOGW("", "*************************************");
            ESP_LOGW("", " Start creating endpoint for DS18B20");
            ESP_LOGW("", "*************************************");
            ESP_LOGW("", "");
            temperature_sensor::config_t sensor_config;
            
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-5500); // -55.00°C
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(12500); // 125.00°C
            
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            break;
        }

        case SENSOR_TYPE_DHT11: {
            // Create endpoint with multiple clusters
            ESP_LOGW("", "");
            ESP_LOGW("", "***********************************");
            ESP_LOGW("", " Start creating endpoint for DHT11");
            ESP_LOGW("", "***********************************");
            ESP_LOGW("", "");
            esp_matter::endpoint::temperature_sensor::config_t sensor_config;
            sensor_config.temperature_measurement.measured_value = nullable<int16_t>(2000);
            sensor_config.temperature_measurement.min_measured_value = nullable<int16_t>(-4000);
            sensor_config.temperature_measurement.max_measured_value = nullable<int16_t>(8500);
                        
            endpoint = temperature_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, NULL);
            
            if(!endpoint) {
              ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create endpoint for DHT11");
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

        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown sensor type: %d", sensor_cfg);
            return ESP_ERR_INVALID_ARG;
    }

    if(!endpoint) {
      ESP_LOGE(TAG_MULTI_SENSOR, "Matter endpoint creation failed for sensor type: %d", sensor_cfg);
      return ESP_FAIL;
    }

    // Initialize sensor hardware
    err = app_driver_sensor_init(sensor_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to initialize sensor type: %d", sensor_cfg);
        return err;
    }

    // Store mapping
    if (configured_sensors < CONFIG_NUM_SENSORS) {
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
        sensors[configured_sensors].last_read_time = 0;
        
        configured_sensors++;
        
        ESP_LOGI(TAG_MULTI_SENSOR, "Sensor %d created with endpoint_id %d", 
                sensor_cfg, endpoint::get_id(endpoint));
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Maximum sensors configuration limit exceeded!");
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
        ESP_LOGI(TAG_MULTI_SENSOR, "NodeLabel set via ESP-Matter API");
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set NodeLabel: %d", err);
    }
}

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;

  // Start reboot button task
  xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);

  #if USE_SSD1306_DRIVER
    //-- Init LCD SSD1306
    err = ssd1306_i2c_init();
    if(err != ESP_OK) {
      ESP_LOGE(TAG_MULTI_SENSOR, "~~~ Error initialize SSD1306!");
      //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    } else {
      ssd1306_initialized = true;
      ESP_LOGW(TAG_MULTI_SENSOR, "~~~ SSD1306 Initialized!");
    }
    ssd1306_show_title();
  #endif
    
  /* Initialize the ESP NVS layer */
  nvs_flash_init();

  /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
  node::config_t node_config;
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create Matter node"));
  

  if(CONFIG_BME280_ENABLED || CONFIG_BME680_ENABLED) {
    ESP_ERROR_CHECK(i2cdev_init());
  }


  // Create sensor endpoints based on configuration
  if(CONFIG_BME280_ENABLED) {
    create_sensor_endpoint(SENSOR_TYPE_BME280, node);
  }
  
  if(CONFIG_BME680_ENABLED) {
    create_sensor_endpoint(SENSOR_TYPE_BME680, node);
  }
  
  if(CONFIG_DS18B20_ENABLED) {
    create_sensor_endpoint(SENSOR_TYPE_DS18B20, node);
  }
  
  if(CONFIG_DHT11_ENABLED) {
    create_sensor_endpoint(SENSOR_TYPE_DHT11, node);
  }

  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    //-- Set OpenThread platform config
    esp_openthread_platform_config_t config = {
      .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
      .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
      .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
  #endif

  //-- Matter start
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to start Matter, err:%d", err));

  //-- Setting BasicInformationCluster attributes
  vTaskDelay(pdMS_TO_TICKS(3000));
  set_basic_attributes_esp_matter();

  //-- Start sensor polling task
  xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, CONFIG_SENSOR_POLL_TASK_PRIORITY, NULL);

  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    esp_matter::console::init();
  #endif
}
