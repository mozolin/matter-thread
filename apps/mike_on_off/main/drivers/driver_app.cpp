
#include <bsp/esp-bsp.h>
#include <app_priv.h>

using namespace chip::app::Clusters;

esp_err_t app_driver_update_gpio_value(gpio_num_t pin, bool value)
{
  esp_err_t err = ESP_OK;

  err = gpio_set_level(pin, value);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "~~~ Failed to set GPIO level");
    return ESP_FAIL;
  } else {
    ESP_LOGW(TAG, "~~~ GPIO pin : %d set to %d", pin, value);
  }
  return err;
}

esp_err_t app_driver_plugin_unit_init(const gpio_plug* plug)
{
  esp_err_t err = ESP_OK;

  gpio_reset_pin(plug->GPIO_PIN_VALUE);

  //client::set_request_callback(app_driver_client_callback, app_driver_client_group_invoke_command_callback, NULL);

  err = gpio_set_direction(plug->GPIO_PIN_VALUE, GPIO_MODE_OUTPUT);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "~~~ Unable to set GPIO OUTPUT mode");
    return ESP_FAIL;
  }

  err = gpio_set_level(plug->GPIO_PIN_VALUE, 0);
  if(err != ESP_OK) {
    ESP_LOGE(TAG, "~~~ Unable to set GPIO level");
  }
  return err;
}

//-- Return GPIO pin from plug-endpoint mapping list
gpio_num_t get_gpio(uint16_t endpoint_id)
{
  gpio_num_t gpio_pin = GPIO_NUM_NC;
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].endpoint_id == endpoint_id) {
      gpio_pin = plugin_unit_list[i].plug;
    }
  }
  return gpio_pin;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  esp_err_t err = ESP_OK;

  if(cluster_id == OnOff::Id) {
    if(attribute_id == OnOff::Attributes::OnOff::Id) {
      gpio_num_t gpio_pin = get_gpio(endpoint_id);
      if(gpio_pin != GPIO_NUM_NC) {
        bool state = val->val.b;
        err = relay_set_on_off(endpoint_id, state);
      } else {
        ESP_LOGE(TAG, "~~~ GPIO pin mapping for endpoint_id: %d not found", endpoint_id);
        return ESP_FAIL;
      }
    }
  }
  return err;
}
