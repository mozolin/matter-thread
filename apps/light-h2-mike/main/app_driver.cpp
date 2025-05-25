
#include <esp_log.h>
#include <stdlib.h>
#include <string.h>

#include <esp_matter.h>
#include <app_priv.h>

#include <device.h>
#include <led_driver.h>
#include <button_gpio.h>

#if MIKE_GPIO_SETTINGS
  #include "driver/gpio.h"
#endif

using namespace chip::app::Clusters;
using namespace esp_matter;

static const char *TAG = "app_driver";
extern uint16_t light_endpoint_id;

#if MIKE_GPIO_SETTINGS
  bool gpio_val1 = false;
  bool gpio_val2 = false;
#endif

#if MIKE_GPIO_SETTINGS
//-- init GPIO as OUTPUT
esp_err_t app_driver_gpio_init(int GPIO)
{
  gpio_num_t gpio_pin = (gpio_num_t)GPIO;
  esp_err_t err = ESP_OK;

  gpio_reset_pin(gpio_pin);

  err = gpio_set_direction(gpio_pin, GPIO_MODE_OUTPUT);
  if(err != ESP_OK) {
    ESP_LOGW(TAG, "Unable to set GPIO OUTPUT mode");
    return ESP_FAIL;
  }
  err = gpio_set_level(gpio_pin, 0);
  if(err != ESP_OK) {
    ESP_LOGW(TAG, "Unable to set GPIO level");
  }

  if(err == ESP_OK) {
  	ESP_LOGW(TAG, "OK! GPIO %d set as OUTPUT!", GPIO);
  }
  return err;
}
//-- update GPIO value (set GPIO level)
static esp_err_t app_driver_update_gpio_value(int GPIO, bool value)
{
  gpio_num_t pin = (gpio_num_t)GPIO;
  
  esp_err_t err = ESP_OK;

  err = gpio_set_level(pin, value);
  if(err != ESP_OK) {
    ESP_LOGW(TAG, "Failed to set GPIO level");
    return ESP_FAIL;
  } else {
    ESP_LOGW(TAG, "GPIO pin : %d set to %d", pin, value);
  }
  return err;
}
#endif

static void app_driver_button_toggle_cb(void *arg, void *data)
{
  ESP_LOGI(TAG, "Toggle button (EP #%d) pressed", light_endpoint_id);
  uint16_t endpoint_id = light_endpoint_id;
  uint32_t cluster_id = OnOff::Id;
  uint32_t attribute_id = OnOff::Attributes::OnOff::Id;

  attribute_t *attribute = attribute::get(endpoint_id, cluster_id, attribute_id);

  esp_matter_attr_val_t val = esp_matter_invalid(NULL);
  attribute::get_val(attribute, &val);
  val.val.b = !val.val.b;
  attribute::update(endpoint_id, cluster_id, attribute_id, &val);
}

app_driver_handle_t app_driver_light_init()
{
  //-- Initialize led
  led_driver_config_t config = led_driver_get_config();
  led_driver_handle_t handle = led_driver_init(&config);
  return (app_driver_handle_t)handle;
}

app_driver_handle_t app_driver_button_init()
{
  //-- Initialize button
  button_handle_t handle = NULL;
  const button_config_t btn_cfg = {0};

  #if MIKE_GPIO_SETTINGS
  	//-- set GPIO as BUTTON
  	const button_gpio_config_t btn_gpio_cfg = {
			.gpio_num = GPIO_BUTTON_VALUE_1,
			.active_level = 0,
			.enable_power_save = true,
		};
  	ESP_LOGW(TAG, "OK! GPIO %d set as BUTTON!", GPIO_BUTTON_VALUE_1);
	#else
	  const button_gpio_config_t btn_gpio_cfg = button_driver_get_config();
	#endif

  if(iot_button_new_gpio_device(&btn_cfg, &btn_gpio_cfg, &handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to create button device");
    return NULL;
  }

  ESP_ERROR_CHECK(iot_button_register_cb(handle, BUTTON_PRESS_DOWN, NULL, app_driver_button_toggle_cb, NULL));
  return (app_driver_handle_t)handle;
}

esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  ESP_LOGW(TAG, "***** app_driver_attribute_update()");

  ESP_LOGW(TAG, "***** endpoint_id (%d) == light_endpoint_id (%d)", (int)endpoint_id, (int)light_endpoint_id);
  ESP_LOGW(TAG, "***** cluster_id (%d) == OnOff::Id (%d)", (int)cluster_id, (int)OnOff::Id);
  ESP_LOGW(TAG, "***** attribute_id (%d) == OnOff::Attributes::OnOff::Id (%d)", (int)attribute_id, (int)OnOff::Attributes::OnOff::Id);

  esp_err_t err = ESP_OK;
  if(endpoint_id == light_endpoint_id) {
    ESP_LOGW(TAG, "***** endpoint_id (%d) == light_endpoint_id (%d)", (int)endpoint_id, (int)light_endpoint_id);
    if(cluster_id == OnOff::Id) {
      ESP_LOGW(TAG, "***** cluster_id (%d) == OnOff::Id (%d)", (int)cluster_id, (int)OnOff::Id);
      if(attribute_id == OnOff::Attributes::OnOff::Id) {
        ESP_LOGW(TAG, "***** attribute_id (%d) == OnOff::Attributes::OnOff::Id (%d)", (int)attribute_id, (int)OnOff::Attributes::OnOff::Id);
        #if MIKE_GPIO_SETTINGS
        if(err == ESP_OK) {
        	//-- toggle GPIO value
        	gpio_val1 = !gpio_val1;
        	err = app_driver_update_gpio_value(GPIO_OUTPUT_VALUE_1, gpio_val1);
        	ESP_LOGW(TAG, "***** app_driver_update_gpio_value(%d, %d)", (int)GPIO_OUTPUT_VALUE_1, (int)gpio_val1);
        }
        #endif
      }
    }
  }


  if(endpoint_id == 2) {
    ESP_LOGW(TAG, "***** endpoint_id (%d) == light_endpoint_id (%d)", (int)endpoint_id, (int)light_endpoint_id);
    if(cluster_id == OnOff::Id) {
      ESP_LOGW(TAG, "***** cluster_id (%d) == OnOff::Id (%d)", (int)cluster_id, (int)OnOff::Id);
      if(attribute_id == OnOff::Attributes::OnOff::Id) {
        ESP_LOGW(TAG, "***** attribute_id (%d) == OnOff::Attributes::OnOff::Id (%d)", (int)attribute_id, (int)OnOff::Attributes::OnOff::Id);
        #if MIKE_GPIO_SETTINGS
        if(err == ESP_OK) {
        	//-- toggle GPIO value
        	gpio_val2 = !gpio_val2;
        	err = app_driver_update_gpio_value(GPIO_OUTPUT_VALUE_2, gpio_val2);
        	ESP_LOGW(TAG, "***** app_driver_update_gpio_value(%d, %d)", (int)GPIO_OUTPUT_VALUE_2, (int)gpio_val2);
        }
        #endif
      }
    }
  }
  return err;
}
