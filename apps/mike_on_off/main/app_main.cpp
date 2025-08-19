/*
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
!!                                                                  !!
!! The ESP32-H2, like other ESP32 variants, can experience panics   !!
!! when using floating-point operations, especially in interrupt    !!
!! service routines (ISRs). This is because the ESP32-H2 doesn't    !!
!! have a hardware floating-point unit (FPU). When float operations !!
!! are performed outside of the main application context (like in   !!
!! an ISR), it can lead to unexpected behavior and crashes.         !!
!!                                                                  !!
!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
*/

#include <esp_err.h>
#include <esp_log.h>
#include <nvs_flash.h>

#include <esp_matter.h>
#include <esp_matter_console.h>
#include <esp_matter_ota.h>
#include "driver/gpio.h"
#include "soc/gpio_num.h"

#include <common_macros.h>
#include <app_priv.h>
#include <app_reset.h>
#include <platform/ESP32/OpenthreadLauncher.h>

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>


#define CREATE_ALL_GPIO_ONOFF_PLUGS(node) \
  for(const auto &relay : relays) { \
    struct gpio_plug plug; \
    plug.GPIO_PIN_VALUE = (gpio_num_t)relay.gpio_pin; \
    create_plug(&plug, node); \
  }
    
using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;


constexpr auto k_timeout_seconds = 300;
uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
static gpio_num_t reset_gpio = gpio_num_t::GPIO_NUM_NC;

led_indicator_handle_t led_handle;

#if USE_SSD1306_DRIVER
  //-- SSD1306 device instance
  SSD1306_t ssd1306dev;
  //-- Is SSD1306 initialized?
  bool ssd1306_initialized = false;
#endif


/*********************
 *                   *
 *   LED INDICATOR   *
 *                   *
 *********************/
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
      //ESP_LOGW(TAG_MIKE_APP, "~~~ ###!!!@@@ FOUND: %d", i);
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

static void app_event_cb(const ChipDeviceEvent *event, intptr_t arg)
{
  switch(event->Type) {
    case chip::DeviceLayer::DeviceEventType::kInterfaceIpAddressChanged:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Interface IP Address changed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningComplete:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning complete");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFailSafeTimerExpired:
      ESP_LOGE(TAG_MIKE_APP, "~~~ Commissioning failed, fail safe timer expired");
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStarted:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session started");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningSessionStopped:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning session stopped");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowOpened:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window opened");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kCommissioningWindowClosed:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Commissioning window closed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kBindingsChangedViaCluster:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Binding entry changed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric removed successfully");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      if(chip::Server::GetInstance().GetFabricTable().FabricCount() == 0) {
        chip::CommissioningWindowManager &commissionMgr = chip::Server::GetInstance().GetCommissioningWindowManager();
        constexpr auto kTimeoutSeconds = chip::System::Clock::Seconds16(k_timeout_seconds);
        if(!commissionMgr.IsCommissioningWindowOpen()) {
          //-- After removing last fabric, this example does not remove the Wi-Fi credentials
          //-- and still has IP connectivity so, only advertising on DNS-SD.
          CHIP_ERROR err = commissionMgr.OpenBasicCommissioningWindow(kTimeoutSeconds, chip::CommissioningWindowAdvertisement::kDnssdOnly);
          if(err != CHIP_NO_ERROR) {
            ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to open commissioning window, err:%" CHIP_ERROR_FORMAT, err.Format());
            get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
          }
        }
      }
      break;
    }
    
    case chip::DeviceLayer::DeviceEventType::kFabricWillBeRemoved:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric will be removed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricUpdated:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is updated");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kFabricCommitted:
      ESP_LOGW(TAG_MIKE_APP, "~~~ Fabric is committed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    case chip::DeviceLayer::DeviceEventType::kBLEDeinitialized:
      ESP_LOGW(TAG_MIKE_APP, "~~~ BLE deinitialized and memory reclaimed");
      get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
      break;
    
    default:
      break;
  }
}

// This callback is invoked when clients interact with the Identify Cluster.
// In the callback implementation, an endpoint can identify itself. (e.g., by flashing an LED or light).
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id, uint8_t effect_variant, void *priv_data)
{
  ESP_LOGW(TAG_MIKE_APP, "~~~ Identification callback: type: %u, effect: %u, variant: %u", type, effect_id, effect_variant);
  get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
  return ESP_OK;
}

#if ADD_CUSTOM_CLUSTERS
	//-- Callback for reading custom attributes
	esp_err_t read_custom_attribute_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
	{
    ESP_LOGW(TAG_MIKE_APP, "~~~ read_custom_attribute_cb():type=%d,ep=%d,cluster=%d,attr=%d", (int)type, (int)endpoint_id, (int)cluster_id, (int)attribute_id);
    
    if(type == attribute::READ) {
		#if USE_INTERNAL_TEMPERATURE
      if(cluster_id == CLUSTER_ID_CHIP_TEMP && attribute_id == 0x0000) {
        //-- Reading chip temperature
        float temp = (float)read_internal_temperature();
        if(temp > 0) {
        	//-- Temperature at 0.01°C
        	val->val.i16 = (int16_t)(temp * 100);
          val->type = ESP_MATTER_VAL_TYPE_INT16;
          ESP_LOGW(TAG_MIKE_APP, "~~~ READ TEMPERATURE: %.0f°C", temp);
        } else {
          ESP_LOGE(TAG_MIKE_APP, "TEMPERATURE FAILED!");
          return ESP_FAIL;
        }
      } else if (cluster_id == CLUSTER_ID_UPTIME && attribute_id == 0x0000) {
        //-- Reading uptime in seconds
        uint32_t uptime = (uint32_t)(esp_timer_get_time() / 1000000);
        val->val.u32 = uptime;
        val->type = ESP_MATTER_VAL_TYPE_UINT32;
        ESP_LOGW(TAG_MIKE_APP, "~~~ READ UPTIME: %lu", uptime);
      }
      #endif
    }
    return ESP_OK;
	}
#endif

static esp_matter::cluster_t *create_temperature_measurement_cluster(esp_matter::endpoint_t *endpoint)
{
    // Создаем кластер измерения температуры
    esp_matter::cluster::temperature_measurement::config_t temp_config;
    temp_config.measured_value = 0 * 100; // Начальное значение температуры (25°C, умноженное на 100)
    temp_config.min_measured_value = -40 * 100; // Минимальная измеряемая температура (-40°C)
    temp_config.max_measured_value = 80 * 100; // Максимальная измеряемая температура (80°C)
    
    return esp_matter::cluster::temperature_measurement::create(endpoint, &temp_config, esp_matter::cluster_flags::CLUSTER_FLAG_SERVER);
}

void update_temperature_value(uint16_t endpoint_id, int16_t temperature_value)
{
	uint32_t
		cluster_id = chip::app::Clusters::TemperatureMeasurement::Id,
		attribute_id = chip::app::Clusters::TemperatureMeasurement::Attributes::MeasuredValue::Id;

	ESP_LOGI("", "");
	ESP_LOGI("", "###   TEMPERATURE   ###");
	ESP_LOGI("", "");
    esp_matter_attr_val_t val = esp_matter_int16(temperature_value * 100); // Значение в 0.01°C
    
    esp_matter::attribute::update(endpoint_id, cluster_id, attribute_id, &val);
	ESP_LOGW(TAG_MIKE_APP, "~~~ Update Temperature: endpoint:0x%04" PRIX16 "|cluster:0x%08" PRIX32 "|attribute:0x%08" PRIX32 "|value:%d", endpoint_id, cluster_id, attribute_id, temperature_value);
}

/*
#define CUSTOM_UPTIME_CLUSTER_ID 0xFFFE
#define CUSTOM_UPTIME_ATTRIBUTE_ID 0x0000
static esp_matter::cluster_t *create_uptime_cluster(esp_matter::endpoint_t *endpoint)
{
    esp_matter::cluster_t *cluster = esp_matter::cluster::create(endpoint, CUSTOM_UPTIME_CLUSTER_ID, esp_matter::cluster_flags::CLUSTER_FLAG_SERVER);
    if (!cluster) {
            ESP_LOGE("Uptime", "Failed to create custom uptime cluster");
            return nullptr;
        }
    
        // Добавляем атрибут времени работы (в секундах)
        esp_matter::attribute::create(
            cluster,
            CUSTOM_UPTIME_ATTRIBUTE_ID,
            MATTER_ATTRIBUTE_FLAG_NONVOLATILE, // Сохраняется между перезагрузками
            esp_matter_uint32(0) // Начальное значение
        );
        
        return cluster;
}

// Функция для обновления времени работы
void update_uptime_value(uint16_t endpoint_id, uint32_t uptime_seconds)
{
    esp_matter_attr_val_t val = esp_matter_uint32(uptime_seconds);
    
	ESP_LOGI("", "");
	ESP_LOGI("", "###   UPTIME   ###");
	ESP_LOGI("", "");
    esp_matter::attribute::update(endpoint_id, CUSTOM_UPTIME_CLUSTER_ID, CUSTOM_UPTIME_ATTRIBUTE_ID, &val);
    ESP_LOGW(TAG_MIKE_APP, "~~~ Update Uptime: endpoint:0x%04" PRIX16 "|cluster:0x%08" PRIX32 "|attribute:0x%08" PRIX32 "|value:%lu", endpoint_id, (uint32_t)CUSTOM_UPTIME_CLUSTER_ID, (uint32_t)CUSTOM_UPTIME_ATTRIBUTE_ID, uptime_seconds);
}
*/

//-- This callback is called for every attribute update. The callback implementation shall
//-- handle the desired attributes and return an appropriate error code. If the attribute
//-- is not of your interest, please do not return an error code and strictly return ESP_OK.
static esp_err_t app_attribute_update_cb(attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data)
{
  ESP_LOGE(TAG_MIKE_APP, "~~~ app_attribute_update_cb():type=%d,ep=0x%04" PRIX16 ",cluster=0x%08" PRIX32 ",attr=0x%08" PRIX32, (int)type, endpoint_id, cluster_id, attribute_id);
  
  esp_err_t err = ESP_OK;

  switch(type) {
    case PRE_UPDATE: {
    	ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::PRE_UPDATE");
        //-- Driver update
        bool state = val->val.b;
        app_driver_handle_t driver_handle = (app_driver_handle_t)priv_data;
        err = app_driver_attribute_update(driver_handle, endpoint_id, cluster_id, attribute_id, val);
        if(err == ESP_OK) {
          ESP_LOGW(TAG_MIKE_APP, "~~~ Updated: endpoint:0x%04" PRIX16 "|cluster:0x%08" PRIX32 "|attribute:0x%08" PRIX32 "|state:%d", endpoint_id, cluster_id, attribute_id, (int)state);
          //-- Blink...
          get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
        } else {
          ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to update attribute: 0x%04" PRIX16 "|0x%08" PRIX32 "|0x%08" PRIX32 "|%d", endpoint_id, cluster_id, attribute_id, (int)state);
          get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
        }
      }
      break;
  
    case POST_UPDATE:
      ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::POST_UPDATE");
      break;
  
    case READ:
      ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::READ");
      #if ADD_CUSTOM_CLUSTERS
	      //-- Callback for reading custom attributes
  	    read_custom_attribute_cb(type, endpoint_id, cluster_id, attribute_id, val, priv_data);
      #endif
      break;
  
    case WRITE:
    	ESP_LOGW(TAG_MIKE_APP, "~~~ app_attribute_update_cb()::WRITE");
      break;
  }
  
  return err;
}

static esp_matter::cluster_t *create_time_sync_cluster(esp_matter::endpoint_t *endpoint)
{
    // Конфигурация кластера времени
    esp_matter::cluster::time_synchronization::config_t time_config;
    /*
    time_config.utc_time = 0;            // Начальное значение UTC времени (эпоха Unix)
    time_config.time_zone = {            // Часовой пояс
        .offset = 0,                     // Смещение от UTC в секундах
        .valid_at = 0                    // Время, когда этот часовой пояс стал актуальным
    };
    time_config.dst_offset = {           // Летнее время
        .offset = 0,                     // Смещение для летнего времени
        .valid_starting_at = 0,          // Начало действия
        .valid_until = 0                 // Конец действия
    };
    time_config.local_time = 0;          // Локальное время
    */
    
    return esp_matter::cluster::time_synchronization::create(
        endpoint, 
        &time_config, 
        esp_matter::cluster_flags::CLUSTER_FLAG_SERVER
    );
}

void update_time_values(uint32_t endpoint_id)
{
    // Получаем текущее время
    time_t now;
    time(&now);
    struct tm timeinfo;
    localtime_r(&now, &timeinfo);

    ESP_LOGW(TAG_MIKE_APP, "~~~ UPDATE TIME #1: %d (%s)", (int)now, asctime(&timeinfo));

    // Обновляем UTC время
    esp_matter_attr_val_t utc_val = esp_matter_uint64(now);
    esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::TimeSynchronization::Id,
        chip::app::Clusters::TimeSynchronization::Attributes::UTCTime::Id,
        &utc_val
    );

    // Обновляем локальное время (пример для UTC+3)
    esp_matter_attr_val_t local_val = esp_matter_uint64(now + 3 * 3600);
    esp_matter::attribute::update(
        endpoint_id,
        chip::app::Clusters::TimeSynchronization::Id,
        chip::app::Clusters::TimeSynchronization::Attributes::LocalTime::Id,
        &local_val
    );

    //val->val.i16
    ESP_LOGW(TAG_MIKE_APP, "~~~ UPDATE TIME #2: utc-%d, local-%d", (int)utc_val.val.u64, (int)local_val.val.u64);
}

//-- Creates plug-endpoint mapping for each GPIO pin configured.
static esp_err_t create_plug(gpio_plug* plug, node_t* node)
{
  esp_err_t err = ESP_OK;

  if(!node) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter node cannot be NULL");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_ARG;
  }

  if(!plug) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Plug cannot be NULL");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_ARG;
  }

  //-- Check if plug's IO pin is already used by reset button
  if((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_ERR_INVALID_STATE;
  }

  //-- Check for plug if already configured.
  for(int i = 0; i < configure_plugs; i++) {
    if(plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
      return ESP_ERR_INVALID_STATE;
    }
  }

  on_off_plugin_unit::config_t plugin_unit_config;
  plugin_unit_config.on_off.on_off = false;
  endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

  if(!endpoint) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Matter endpoint creation failed");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_FAIL;
  }

  //-- GPIO pin Initialization
  err = app_driver_plugin_unit_init(plug);

  if(err != ESP_OK) {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to initialize plug");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
  }

  //-- Add temperature cluster to endpoint 1
  if(configure_plugs == 0) {
    create_temperature_measurement_cluster(endpoint);
    //create_uptime_cluster(endpoint);
    create_time_sync_cluster(endpoint);
  }

  //-- Check for maximum plugs that can be configured.
  if(configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
    plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
    plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
    configure_plugs++;
  } else {
    ESP_LOGE(TAG_MIKE_APP, "~~~ Maximum plugs configuration limit exceeded!");
    get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    return ESP_FAIL;
  }

  uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
  ESP_LOGW(TAG_MIKE_APP, "~~~ Plug created with endpoint_id %d", plug_endpoint_id);
  get_led_indicator_blink_idx(BLINK_ONCE_GREEN, 60, 0);
  return err;
}

void update_matter_values()
{
	float temp = 0;
	
	float volt = 0;
	char* short_uptime_buf = "";
	struct tm timeinfo;
	int16_t temp_val_int = 0;

	//-- empty value of buffer
  //snprintf(bufUP, sizeof(bufUP), "%s", "");
  //strncpy(short_uptime_buf, bufUP, OT_UPTIME_STRING_SIZE);

  //-- !!! This block is used to update the value of matter clusters !!!
	while(1) {
    #if USE_INTERNAL_TEMPERATURE
      temp = (float)read_internal_temperature();
      temp_val_int = (int16_t)temp;
			/*
			idx++;
			if(idx % 2 == 0) {
				temp = 12.0f;
			} else {
				temp = 34.0f;
			}
			*/
      update_temperature_value(1, temp_val_int);
      ESP_LOGW(TAG_MIKE_APP, "~~~ Internal Temperature: %.0f°C", temp);
    #endif
    
    #if ADD_CUSTOM_CLUSTERS
      esp_matter_attr_val_t temp_val = {
        .type = ESP_MATTER_VAL_TYPE_INT16,
        .val = {
          .i16 = temp_val_int // 0.01°C
        }
      };
      update_custom_attribute(CUSTOM_ENDPOINT_ID, CLUSTER_ID_CHIP_TEMP, 0x0000, temp_val);
      ESP_LOGW(TAG_MIKE_APP, "~~~ Custom Cluster Temperature: %d°C", temp_val_int);
    #endif
    
    #if USE_INTERNAL_UPTIME
      update_time_values(1);
      
      char bufUP[OT_UPTIME_STRING_SIZE];
      //-- long uptime
      //uptime_buf = ot_get_thread_uptime(bufUP);
      //-- short uptime
      short_uptime_buf = ot_get_thread_short_uptime(bufUP);
      
      short_uptime_buf = get_system_clock_uptime(bufUP);
    #endif
    
    #if USE_TIME_DRIVER
      time_t now;
      time(&now);
    
      ESP_LOGW(TAG_MIKE_APP, "~~~ Time Now - 2: %d", (int)now);
    
      if(ot_get_current_thread_time(&timeinfo)) {
        ESP_LOGW(TAG_MIKE_APP, "~~~ OpenThread Time is synchronized!");
      } else if(chip_get_current_matter_time(&timeinfo)) {
        ESP_LOGW(TAG_MIKE_APP, "~~~ Matter Time is synchronized!");
    	} else if(now < 3600*24*365) {
        timeinfo.tm_hour = 12;
        timeinfo.tm_min = 0;
        timeinfo.tm_sec = 0;
        ESP_LOGE(TAG_MIKE_APP, "~~~ Time is not synchronized, using RTC fallback");
      } else {
        // Fallback на системное время
        now = time(nullptr);
        ESP_LOGE(TAG_MIKE_APP, "~~~ Time is not synchronized, using default value");
      }
      
      localtime_r(&now, &timeinfo);
      ESP_LOGW(TAG_MIKE_APP, "~~~ Current time: %s", asctime(&timeinfo));
    #endif
    
    #if USE_INTERNAL_VOLTAGE
      volt = (float)read_internal_voltage();
    #endif
    
    #if USE_SSD1306_DRIVER
    	//-- show all updated values of matter clusters
    	ssd1306_show_matter_updates(temp, volt, short_uptime_buf, timeinfo);
    #endif

    vTaskDelay(pdMS_TO_TICKS(UPDATE_ATTRIBUTES_TIME_MS));
  }
}

extern "C" void app_main()
{
  esp_err_t err = ESP_OK;
  
  //-- Start reboot button task
  xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, 5, NULL);
  
  //-- Init LED indicator
  led_handle = configure_indicator();
  
  #if USE_SSD1306_DRIVER
    //-- Init LCD SSD1306
    err = ssd1306_init();
    if(err != ESP_OK) {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Error initialize SSD1306!");
      get_led_indicator_blink_idx(BLINK_ONCE_RED, 60, 0);
    } else {
      ssd1306_initialized = true;
    }
    ssd1306_show_title();
  #endif

  //-- Initialize the ESP NVS (Non-Volatile Storage) layer
  err = nvs_flash_init();
  if(err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    //ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_erase();
    if(err == ESP_OK) {
      err = nvs_flash_init();
    }
  }
  //ESP_ERROR_CHECK(err);
  if(err == ESP_OK) {
    relay_init();
  }

  //-- Create a Matter node and add the mandatory Root Node device type on endpoint 0
  node::config_t node_config;

  //-- node handle can be used to add/modify other endpoints.
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create Matter node"));

  //-- Creates plug-endpoint mapping for all GPIO pins configured.
  CREATE_ALL_GPIO_ONOFF_PLUGS(node);

  //-- Set OpenThread platform config
  esp_openthread_platform_config_t config = {
    .radio_config = ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG(),
    .host_config = ESP_OPENTHREAD_DEFAULT_HOST_CONFIG(),
    .port_config = ESP_OPENTHREAD_DEFAULT_PORT_CONFIG(),
  };
  set_openthread_platform_config(&config);

  //-- Matter starts
  err = esp_matter::start(app_event_cb);
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to start Matter, err:%d", err));

	#if ADD_CUSTOM_CLUSTERS
  	//-- Temperature cluster
  	if(create_custom_cluster(CUSTOM_ENDPOINT_ID, CLUSTER_ID_CHIP_TEMP, 0x0000, esp_matter_int16(0))) {
  		ESP_LOGW(TAG_MIKE_APP, "~~~ Custom Temperature cluster created successfully");
  	} else {
  		ESP_LOGE(TAG_MIKE_APP, "~~~ Error creating Custom Temperature cluster");
  	}
  	//-- Uptime cluster
  	if(create_custom_cluster(CUSTOM_ENDPOINT_ID, CLUSTER_ID_UPTIME, 0x0000, esp_matter_uint32(0))) {
  		ESP_LOGW(TAG_MIKE_APP, "~~~ Custom Uptime cluster created successfully");
  	} else {
  		ESP_LOGE(TAG_MIKE_APP, "~~~ Error creating Custom Uptime cluster");
  	}
  #endif

  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    esp_matter::console::init();
  #endif

  if(node) {
  	//-- Device structure log
    log_device_structure(node);
  }


  #if LIVE_BLINK_TIME_MS > 0
	  //-- Start init indicator task
  	xTaskCreate(init_indicator_task, "init_indicator_task", 2048, NULL, 6, NULL);
  #endif

  #if USE_TIME_DRIVER
     //-- Initialization of time
     //sntp_initialize_time();
     netif_sntp_initialize_time();
     chip_initialize_time();
     show_time();
  #endif

  #if USE_INTERNAL_UPTIME
     //-- Initialization of Thread
     ot_init_thread_time_sync();
  #endif
  
  #if USE_INTERNAL_VOLTAGE
    //-- ADC initialization
    if(!init_internal_voltage()) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to init power monitor!");
    }
  #endif

  update_matter_values();

}
