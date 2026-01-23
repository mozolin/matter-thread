#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#include "sensor_driver.h"

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
//using namespace esp_matter::cluster;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;
uint16_t configured_sensors = 0;
sensor_endpoint_mapping_t sensor_mapping_list[CONFIG_NUM_SENSORS];
sensor_data_t sensors[CONFIG_NUM_SENSORS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

#if CONFIG_ENABLE_ENCRYPTED_OTA
extern const char decryption_key_start[] asm("_binary_esp_image_encryption_key_pem_start");
extern const char decryption_key_end[] asm("_binary_esp_image_encryption_key_pem_end");

static const char *s_decryption_key = decryption_key_start;
static const uint16_t s_decryption_key_len = decryption_key_end - decryption_key_start;
#endif // CONFIG_ENABLE_ENCRYPTED_OTA

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
                /* After removing last fabric, this example does not remove the Wi-Fi credentials
                 * and still has IP connectivity so, only advertising on DNS-SD.
                 */
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

// This callback is invoked when clients interact with the Identify Cluster.
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
{
    ESP_LOGI(TAG_MULTI_SENSOR, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
    return ESP_OK;
}

// This callback is called for every attribute update.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id,
                                         uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
    esp_err_t err = ESP_OK;

    if (type == PRE_UPDATE) {
        /* Driver update */
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
    }

    return err;
}

// Creates sensor-endpoint mapping for each sensor
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

    // Check if sensor is already configured
    for (int i = 0; i < configured_sensors; i++) {
        if (sensor_mapping_list[i].sensor_type == sensor_cfg->type) {
            ESP_LOGW(TAG_MULTI_SENSOR, "Sensor type %d already configured", sensor_cfg->type);
            return ESP_ERR_INVALID_STATE;
        }
    }

    // Create different endpoints based on sensor type
    endpoint_t *endpoint = NULL;
    
    switch (sensor_cfg->type) {
        case SENSOR_TYPE_PIR:

        case SENSOR_TYPE_MICROWAVE: {
            // Create occupancy sensor endpoint using occupancy_sensor namespace
            occupancy_sensor::config_t occupancy_config;
            // Configure occupancy_sensing cluster
            occupancy_config.occupancy_sensing.occupancy = 0; // Not occupied initially
            
            // Set occupancy sensor type
            if (sensor_cfg->type == SENSOR_TYPE_PIR) {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x00; // PIR
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 0); // PIR bit
            } else {
                occupancy_config.occupancy_sensing.occupancy_sensor_type = 0x01; // Ultrasonic
                occupancy_config.occupancy_sensing.occupancy_sensor_type_bitmap = (1 << 1); // Ultrasonic bit
            }
            
            occupancy_config.occupancy_sensing.features = 0x01; // Occupancy feature
            
            endpoint = occupancy_sensor::create(node, &occupancy_config, ENDPOINT_FLAG_NONE, sensor_cfg);
            break;
        }
        case SENSOR_TYPE_ULTRASONIC: {
            // Create temperature sensor endpoint for distance (repurposed)
            temperature_sensor::config_t temp_config;
            temp_config.temperature_measurement.min_measured_value = -50;
            temp_config.temperature_measurement.max_measured_value = 150;
            temp_config.temperature_measurement.measured_value = 25;
            endpoint = temperature_sensor::create(node, &temp_config, ENDPOINT_FLAG_NONE, sensor_cfg);
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

    // Initialize sensor hardware
    err = app_driver_sensor_init(sensor_cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MULTI_SENSOR, "Failed to initialize sensor type: %d", sensor_cfg->type);
        return err;
    }

    // Store mapping
    if (configured_sensors < CONFIG_NUM_SENSORS) {
        sensor_mapping_list[configured_sensors].endpoint_id = endpoint::get_id(endpoint);
        sensor_mapping_list[configured_sensors].sensor_type = sensor_cfg->type;
        sensor_mapping_list[configured_sensors].primary_gpio = sensor_cfg->trigger_pin;
        sensor_mapping_list[configured_sensors].secondary_gpio = sensor_cfg->echo_pin;
        
        // Store in sensors array
        sensors[configured_sensors].config = *sensor_cfg;
        sensors[configured_sensors].last_state = false;
        sensors[configured_sensors].last_distance_cm = 0;
        sensors[configured_sensors].last_detection_time = 0;
        
        configured_sensors++;
        
        ESP_LOGW(TAG_MULTI_SENSOR, "~~~ Sensor %s created with endpoint_id %d", 
                sensor_cfg->name, endpoint::get_id(endpoint));
    } else {
        ESP_LOGE(TAG_MULTI_SENSOR, "Maximum sensors configuration limit exceeded!");
        return ESP_FAIL;
    }

    return err;
}

extern "C" void app_main()
{
    esp_err_t err = ESP_OK;

    /* Initialize the ESP NVS layer */
    nvs_flash_init();

    /* Create a Matter node and add the mandatory Root Node device type on endpoint 0 */
    node::config_t node_config;

    // node handle can be used to add/modify other endpoints.
    node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
    ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to create Matter node"));

    // Configure sensors
    sensor_config_t pir_sensor = {
        .type = SENSOR_TYPE_PIR,
        .trigger_pin = (gpio_num_t)CONFIG_HCSR501_PIR_GPIO,
        .echo_pin = GPIO_NUM_NC,
        .endpoint_id = 0,
        .name = "PIR Sensor"
    };
    
    sensor_config_t microwave_sensor = {
        .type = SENSOR_TYPE_MICROWAVE,
        .trigger_pin = (gpio_num_t)CONFIG_RCWL0516_MICROWAVE_GPIO,
        .echo_pin = GPIO_NUM_NC,
        .endpoint_id = 0,
        .name = "Microwave Sensor"
    };
    
    sensor_config_t ultrasonic_sensor = {
        .type = SENSOR_TYPE_ULTRASONIC,
        .trigger_pin = (gpio_num_t)CONFIG_HCSR04_TRIG_GPIO,
        .echo_pin = (gpio_num_t)CONFIG_HCSR04_ECHO_GPIO,
        .endpoint_id = 0,
        .name = "Ultrasonic Sensor"
    };

    // Create sensor endpoints
    if (CONFIG_HCSR501_ENABLED) {
        create_sensor_endpoint(&pir_sensor, node);
    }
    
    if (CONFIG_RCWL0516_ENABLED) {
        create_sensor_endpoint(&microwave_sensor, node);
    }
    
    if (CONFIG_HCSR04_ENABLED) {
        create_sensor_endpoint(&ultrasonic_sensor, node);
    }

    /*
    // Initialize factory reset button
    app_driver_handle_t button_handle = app_driver_button_init(&reset_gpio);
    if (button_handle) {
        app_reset_button_register(button_handle);
    }
    */

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    /* Set OpenThread platform config */
    esp_openthread_platform_config_t config = {
        .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
        .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
        .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
    };
    set_openthread_platform_config(&config);
#endif

    /* Matter start */
    err = esp_matter::start(app_event_cb);
    ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MULTI_SENSOR, "Failed to start Matter, err:%d", err));

    // Start sensor polling task
    xTaskCreate(sensor_polling_task, "sensor_poll", 4096, NULL, 5, NULL);

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