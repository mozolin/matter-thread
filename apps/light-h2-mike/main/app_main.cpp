
#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

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

static const char *TAG = "app_main";
uint16_t light_endpoint_id = 1;

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;


static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
    switch (event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
        ESP_LOGI(TAG, "Interface IP Address changed");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
        ESP_LOGI(TAG, "Commissioning complete");
        break;

    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
        ESP_LOGI(TAG, "Commissioning failed, fail safe timer expired");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
        ESP_LOGI(TAG, "Commissioning session started");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
        ESP_LOGI(TAG, "Commissioning session stopped");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
        ESP_LOGI(TAG, "Commissioning window opened");
        break;

    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
        ESP_LOGI(TAG, "Commissioning window closed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved:
        {
            ESP_LOGI(TAG, "Fabric removed successfully");
            if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
            {
                chip::CommissioningWindowManager & commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
                constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
                if (!commissionMgr.IsCommissioningWindowOpen())
                {
                    /* After removing last fabric, this example does not remove the Wi-Fi credentials
                     * and still has IP connectivity so, only advertising on DNS-SD.
                     */
                    CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds,
                                                    chip::CommissioningWindowAdvertisement::kDnssdOnly);
                    if (err != CHIP_NO_ERROR)
                    {
                        ESP_LOGE(TAG, "Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
                    }
                }
            }
        break;
        }

    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
        ESP_LOGI(TAG, "Fabric will be removed");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
        ESP_LOGI(TAG, "Fabric is updated");
        break;

    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
        ESP_LOGI(TAG, "Fabric is committed");
        break;

    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
        ESP_LOGI(TAG, "BLE deinitialized and memory reclaimed");
        break;

    default:
        break;
    }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGI(TAG, "Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  return ESP_OK;
}

// This callback is called for every attribute update. The callback implementation shall
// handle the desired attributes and return an appropriate error code. If the attribute
// is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  esp_err_t err = ESP_OK;

  if(type == PRE_UPDATE) {
    //-- Driver update
    app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
    err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
  }

  return err;
}


extern "C" void app_main()
{
  esp_err_t err = ESP_OK;

  //-- Initialize the ESP NVS layer
  nvs_flash_init();

  //-- Initialize driver
  app_driver_handle_t light_handle = app_driver_light_init();
  app_driver_handle_t button_handle = app_driver_button_init();
  app_reset_button_register(button_handle);

  #if MIKE_GPIO_SETTINGS
    //-- init GPIO as OUTPUT
    err = app_driver_gpio_init(GPIO_OUTPUT_VALUE_1);
    err = app_driver_gpio_init(GPIO_OUTPUT_VALUE_2);
  #endif

  //-- Create a Matter node and add the mandatory Root Node device type on endpoint 0
  node::config_t node_config;

  //-- node handle can be used to add/modify other endpoints.
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));


  /*************
   *           *
   *   LIGHT   *
   *           *
   *************/
  on_off_light::config_t light_config;
  endpoint_t *endpoint1 = on_off_light::create(node, &light_config, ENDPOINT_FLAG_NONE, light_handle);
  ABORT_APP_ON_FAILURE(endpoint1 != nullptr, ESP_LOGE(TAG, "Failed to create extended color light endpoint"));

  on_off_light::config_t light_config2;
  endpoint_t *endpoint3 = on_off_light::create(node, &light_config2, ENDPOINT_FLAG_NONE, light_handle);
  ABORT_APP_ON_FAILURE(endpoint3 != nullptr, ESP_LOGE(TAG, "Failed to create extended color light endpoint"));

  
  /**************
   *            *
   *   SWITCH   *
   *            *
   **************/
  on_off_switch::config_t switch_config;
  endpoint_t *endpoint2 = on_off_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, light_handle);
  ABORT_APP_ON_FAILURE(endpoint2 != nullptr, ESP_LOGE(TAG, "Failed to create on-off switch endpoint"));
  
  /*
  on_off_switch::config_t switch_config;
  endpoint_t *endpoint_2 = on_off_switch::create(node, &switch_config, ENDPOINT_FLAG_NONE, button_handle_2);
  ABORT_APP_ON_FAILURE(endpoint_2 != nullptr, ESP_LOGE(TAG, "Failed to create on/off switch endpoint"));
  //-- Add group cluster to the switch endpoint
  cluster::groups::config_t groups_config;
  cluster::groups::create(endpoint_2, &groups_config, CLUSTER_FLAG_SERVER | CLUSTER_FLAG_CLIENT);

  switch_endpoint_id = endpoint::get_id(endpoint_2);
  ESP_LOGW(TAG, "ON/OFF Switch created with endpoint_id %d", switch_endpoint_id);
  */
  
  /*
  uint32_t on_off_cluster_id = 6;
  cluster_t *on_off_cluster = cluster::create(endpoint::get(node, switch_endpoint_id), on_off_cluster_id, CLUSTER_FLAG_SERVER);
  ABORT_APP_ON_FAILURE(on_off_cluster != nullptr, ESP_LOGE(TAG, "Failed to create ON-OFF cluster"));
  */


  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD && CHIP_DEVICE_CONFIG_ENABLE_WIFI_STATION
    //-- Enable secondary network interface
    secondary_network_interface::config_t secondary_network_interface_config;
    endpoint = endpoint::secondary_network_interface::create(node, &secondary_network_interface_config, ENDPOINT_FLAG_NONE, nullptr);
    ABORT_APP_ON_FAILURE(endpoint != nullptr, ESP_LOGE(TAG, "Failed to create secondary network interface endpoint"));
  #endif


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
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

  //-- Starting driver with default values
  //app_driver_light_set_defaults(light_endpoint_id);

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
