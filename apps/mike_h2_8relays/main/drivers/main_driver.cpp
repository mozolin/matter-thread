
#include <app_priv.h>
#include "app_main_driver.h"

using namespace chip::app::Clusters;

esp_err_t app_driver_update_gpio_value(gpio_num_t pin, bool value)
{
  esp_err_t err = ESP_OK;

  #if CONFIG_RELAY_INVERSE_LEVEL
    err = gpio_set_level(pin, !value);
  #else
    err = gpio_set_level(pin, value);
  #endif
  if(err != ESP_OK) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Failed to set GPIO level");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_FAIL;
  } else {
    #if CONFIG_DEBUG_MODE
      ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ GPIO pin : %d set to %d", pin, value);
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
    #endif
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
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Unable to set GPIO OUTPUT mode");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
    return ESP_FAIL;
  }

  #if CONFIG_RELAY_INVERSE_LEVEL
    err = gpio_set_level(plug->GPIO_PIN_VALUE, 1);
  #else
    err = gpio_set_level(plug->GPIO_PIN_VALUE, 0);
  #endif
  if(err != ESP_OK) {
    #if CONFIG_DEBUG_MODE
      ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Unable to set GPIO level");
    #endif
    #if CONFIG_USE_DRIVER_LED_INDICATOR
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    #endif
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


//-- This callback is called for every attribute update. The callback implementation shall
//-- handle the desired attributes and return an appropriate error code. If the attribute
//-- is not of your interest, please do not return an error code and strictly return ESP_OK.
esp_err_t app_attribute_update_main_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  #if CONFIG_DEBUG_MODE
    ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ app_attribute_update_main_cb():type=%d,ep=0x%04" PRIX16 ",cluster=0x%08" PRIX32 ",attr=0x%08" PRIX32, (int)type, endpoint_id, cluster_id, attribute_id);
  #endif
  
  esp_err_t err = ESP_OK;

  switch(type) {
    case PRE_UPDATE: {
    	#if CONFIG_DEBUG_MODE
    	  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ app_attribute_update_main_cb()::PRE_UPDATE");
    	#endif
      //-- Driver update
      bool state = val->val.b;
      app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
      err = app_driver_attribute_update_main(driver_handle, endpoint_id, cluster_id, attribute_id, val);
      if(err == ESP_OK) {
        #if CONFIG_DEBUG_MODE
          ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ Updated: endpoint:0x%04" PRIX16 "|cluster:0x%08" PRIX32 "|attribute:0x%08" PRIX32 "|state:%d", endpoint_id, cluster_id, attribute_id, (int)state);
        #endif
        //-- Blink...
        #if CONFIG_USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
        #endif
      } else {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ Failed to update attribute: 0x%04" PRIX16 "|0x%08" PRIX32 "|0x%08" PRIX32 "|%d", endpoint_id, cluster_id, attribute_id, (int)state);
        #endif
        #if CONFIG_USE_DRIVER_LED_INDICATOR
          //get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        #endif
      }
      break;
    }
  
    case POST_UPDATE: {
      #if CONFIG_DEBUG_MODE
        ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ app_attribute_update_main_cb()::POST_UPDATE");
      #endif
      break;
    }
  
    case READ: {
      #if CONFIG_DEBUG_MODE
        ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ app_attribute_update_main_cb()::READ");
      #endif

      #if ADD_CUSTOM_CLUSTERS
	      //-- Callback for reading custom attributes
  	    read_custom_attribute_cb(type, endpoint_id, cluster_id, attribute_id, val, priv_data);
      #endif
      break;
    }

    case WRITE: {
    	#if CONFIG_DEBUG_MODE
    	  ESP_LOGW(CONFIG_TAG_MIKE_APP, "~~~ app_attribute_update_main_cb()::WRITE");
    	#endif
      break;
    }
  }

  err = app_attribute_update_cb(type, endpoint_id, cluster_id, attribute_id, val, priv_data);
  
  return err;
}


esp_err_t app_driver_attribute_update_main(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val)
{
  esp_err_t err = ESP_OK;

  if(cluster_id == OnOff::Id) {
    if(attribute_id == OnOff::Attributes::OnOff::Id) {
      gpio_num_t gpio_pin = get_gpio(endpoint_id);
      if(gpio_pin != GPIO_NUM_NC) {
        bool state = val->val.b;
        err = relay_set_on_off(endpoint_id, state);
      } else {
        #if CONFIG_DEBUG_MODE
          ESP_LOGE(CONFIG_TAG_MIKE_APP, "~~~ GPIO pin mapping for endpoint_id: %d not found", endpoint_id);
        #endif
        #if CONFIG_USE_DRIVER_LED_INDICATOR
          get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        #endif
        return ESP_FAIL;
      }
    }
  }
  
  err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
  
  return err;
}
