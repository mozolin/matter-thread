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
#include "drivers/sensor_common.h"

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
sensor_data_t sensors[CONFIG_NUM_SENSORS];

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

static esp_err_t create_sensor_endpoint(sensor_config_t* sensor_cfg, node_t* node)
{
    esp_err_t err = ESP_OK;

    if (!node) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Matter node cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!sensor_cfg) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Sensor config cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].sensor_type == sensor_cfg->type) {
            ESP_LOGW(TAG_MULTI_SENSOR, "Sensor type %d already configured", sensor_cfg->type);
            return ESP_ERR_INVALID_STATE;
        }
    }

    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg->type) {
        case SENSOR_TYPE_PIR:
        case SENSOR_TYPE_MICROWAVE: {
            occupancy_sensor::config_t occupancy_config;
            occupancy_config.occupancy_sensing.occupancy = 0;
            
            if (sensor_cfg->type == SENSOR_TYPE_PIR) {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x00; // PIR
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 0);
            } else {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x01; // Ultrasonic
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 1);
            }
            
            occupancy_config.occupancy_sensing.features = 0x01;
            
            endpoint = occupancy_sensor::create(node, &occupancy_config, ENDPOINT_FLAG_NONE, sensor_cfg);
            break;
        }

        case SENSOR_TYPE_ULTRASONIC: {
            occupancy_sensor::config_t sensor_config;
            sensor_config.occupancy_sensing.occupancy = 0;
            sensor_config.occupancy_sensing.occupancy_sensor_type = 0x01; // Ultrasonic
            sensor_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 1);
            sensor_config.occupancy_sensing.features = 0x01;
            
            endpoint = occupancy_sensor::create(node, &sensor_config, ENDPOINT_FLAG_NONE, sensor_cfg);

            if (!endpoint) {
                ESP_LOGW(TAG_MULTI_SENSOR, "Failed to create endpoint for ultrasonic sensor");
                return ESP_FAIL;
            }
            
            cluster::pressure_measurement::config_t pressure_config;
            
            const int16_t BASE_PRESSURE = 1000;
            const int16_t PRESSURE_PER_CM = 10;
            
            int16_t min_pressure = BASE_PRESSURE + (2 * PRESSURE_PER_CM);
            int16_t max_pressure = BASE_PRESSURE + (400 * PRESSURE_PER_CM);
            
            pressure_config.pressure_min_measured_value = nullable<int16_t>(min_pressure);
            pressure_config.pressure_max_measured_value = nullable<int16_t>(max_pressure);
            pressure_config.pressure_measured_value = BASE_PRESSURE + (100 * PRESSURE_PER_CM);
            
            esp_matter::cluster_t *pressure_cluster = 
                esp_matter::cluster::pressure_measurement::create(endpoint, &pressure_config, MATTER_CLUSTER_FLAG_SERVER);
            
            if (!pressure_cluster) {
                ESP_LOGW(TAG_MULTI_SENSOR, "Failed to create Pressure Measurement cluster");
            }
            
            esp_matter::cluster::occupancy_sensing::config_t occupancy_config;
            
            occupancy_config.occupancy = 0x00;
            occupancy_config.occupancy_sensor_type = 0x01;
            occupancy_config.occupancy_sensor_type_bitmap = (1 << 1);
            occupancy_config.features = 0x01;
            
            esp_matter::cluster_t *occupancy_cluster = 
                esp_matter::cluster::occupancy_sensing::create(endpoint, &occupancy_config, MATTER_CLUSTER_FLAG_SERVER);
            
            if (!occupancy_cluster) {
                ESP_LOGW(TAG_MULTI_SENSOR, "Failed to create Occupancy Sensing cluster");
            }
            
            esp_matter::cluster::identify::config_t identify_config;
            identify_config.identify_time = 0;
            
            esp_matter::cluster_t *identify_cluster = 
                esp_matter::cluster::identify::create(endpoint, &identify_config, MATTER_CLUSTER_FLAG_SERVER);
            
            break;
        }

        default:
            ESP_LOGE(TAG_MULTI_SENSOR, "Unknown sensor type: %d", sensor_cfg->type);
            return ESP_ERR_INVALID_ARG;
    }

    if (!endpoint) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Matter endpoint creation failed for sensor type: %d", sensor_cfg->type);
        return ESP_FAIL;
    }

    err = app_driver_sensor_init(sensor_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to initialize sensor type: %d", sensor_cfg->type);
        return err;
    }

    if (configured_sensors < CONFIG_NUM_SENSORS) {
        sensor_mapping_list[configured_sensors].endpoint_id = endpoint::get_id(endpoint);
        sensor_mapping_list[configured_sensors].sensor_type = sensor_cfg->type;
        sensor_mapping_list[configured_sensors].primary_gpio = sensor_cfg->trigger_pin;
        sensor_mapping_list[configured_sensors].secondary_gpio = sensor_cfg->echo_pin;
        
        sensors[configured_sensors].config = *sensor_cfg;
        sensors[configured_sensors].last_state = false;
        sensors[configured_sensors].last_distance_cm = 0;
        sensors[configured_sensors].last_detection_time = 0;
        
        configured_sensors++;
        
        ESP_LOGI(TAG_MULTI_SENSOR, "Sensor %s created with endpoint_id %d", 
                sensor_cfg->name, endpoint::get_id(endpoint));
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Maximum sensors configuration limit exceeded!");
        return ESP_FAIL;
    }

    return err;
}

void set_basic_attributes_esp_matter()
{
    uint16_t endpoint_id = 0x0000;

    char node_label[] = CONFIG_CUSTOM_DEVICE_NODE_LABEL;
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
  
    char location[] = CONFIG_CUSTOM_DEVICE_LOCATION;
    esp_matter_attr_val_t location_val = esp_matter_char_str(location, strlen(location));
    
    err = esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::BasicInformation::Id,
        chip::app::Clusters::BasicInformation::Attributes::Location::Id,
        &location_val
    );
    if(err == ESP_OK) {
        ESP_LOGI(TAG_MULTI_SENSOR, "Location set via ESP-Matter API");
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to set Location: %d", err);
    }
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, CONFIG_REBOOT_BUTTON_TASK_PRIORITY, NULL);
  
    nvs_flash_init();

    node::config_t node_config;
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create Matter node"));

#if CONFIG_HCSR501_ENABLED
    sensor_config_t pir_sensor = {
        .type = SENSOR_TYPE_PIR,
        .trigger_pin = (gpio_num_t)CONFIG_HCSR501_PIR_GPIO,
        .echo_pin = GPIO_NUM_NC,
        .endpoint_id = 0,
        .name = "PIR Sensor (HC-SR501)"
    };
    
    if (CONFIG_HCSR501_ENABLED) {
        create_sensor_endpoint(&pir_sensor, node);
    }
#endif

#if CONFIG_RCWL0516_ENABLED
    sensor_config_t microwave_sensor = {
        .type = SENSOR_TYPE_MICROWAVE,
        .trigger_pin = (gpio_num_t)CONFIG_RCWL0516_MICROWAVE_GPIO,
        .echo_pin = GPIO_NUM_NC,
        .endpoint_id = 0,
        .name = "Microwave Sensor (RCWL-0516)"
    };
  
    if (CONFIG_RCWL0516_ENABLED) {
        create_sensor_endpoint(&microwave_sensor, node);
    }
#endif

#if CONFIG_HCSR04_ENABLED
    sensor_config_t ultrasonic_sensor = {
        .type = SENSOR_TYPE_ULTRASONIC,
        .trigger_pin = (gpio_num_t)CONFIG_HCSR04_TRIG_GPIO,
        .echo_pin = (gpio_num_t)CONFIG_HCSR04_ECHO_GPIO,
        .endpoint_id = 0,
        .name = "Ultrasonic Sensor (HC-SR04)"
    };
  
    if (CONFIG_HCSR04_ENABLED) {
        create_sensor_endpoint(&ultrasonic_sensor, node);
    }
#endif

    #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
    #endif

    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to start Matter, err:%d", err));

    vTaskDelay(pdMS_TO_TICKS(3000));
    set_basic_attributes_esp_matter();

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
