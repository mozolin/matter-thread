
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
#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include <platform/ESP32/OpenthreadLauncher.h>
#endif

#include <app/server/CommissioningWindowManager.h>
#include <app/server/Server.h>

#define CREATE_PLUG(node, plug_id) \
    struct gpio_plug plug##plug_id; \
    plug##plug_id.GPIO_PIN_VALUE = (gpio_num_t) CONFIG_GPIO_PLUG_##plug_id; \
    create_plug(&plug##plug_id, node);

static const char *TAG = "app_main";
#if APP_USE_LED_STRIP
  static const char *TAG_LED = "LED";
#endif

using namespace esp_matter;
using namespace esp_matter::attribute;
using namespace esp_matter::endpoint;
using namespace chip::app::Clusters;

constexpr auto k_timeout_seconds = 300;
uint16_t configure_plugs = 0;
plugin_endpoint plugin_unit_list[CONFIG_NUM_VIRTUAL_PLUGS];
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

    case chip::DeviceLayer::DeviceEventType::kFabricRemoved: {
        ESP_LOGI(TAG, "Fabric removed successfully");
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
static esp_err_t app_identification_cb(identification::callback_type_t type, uint16_t endpoint_id, uint8_t effect_id,
                                       uint8_t effect_variant, void *priv_data)
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

// Creates plug-endpoint mapping for each GPIO pin configured.
static esp_err_t create_plug(gpio_plug* plug, node_t* node)
{
    esp_err_t err = ESP_OK;

    if (!node) {
        ESP_LOGE(TAG, "Matter node cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    if (!plug) {
        ESP_LOGE(TAG, "Plug cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGW(TAG, "Reset button gpio pin : %d", plug->GPIO_PIN_VALUE);
    
    // Check if plug's IO pin is already used by reset button
    if ((reset_gpio != gpio_num_t::GPIO_NUM_NC) && (reset_gpio == plug->GPIO_PIN_VALUE)) {
        ESP_LOGE(TAG, "Reset button already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
        return ESP_ERR_INVALID_STATE;
    }

    // Check for plug if already configured.
    for (int i = 0; i < configure_plugs; i++) {
        if (plugin_unit_list[i].plug == plug->GPIO_PIN_VALUE) {
            ESP_LOGE(TAG, "Plug already configured for gpio pin : %d", plug->GPIO_PIN_VALUE);
            return ESP_ERR_INVALID_STATE;
        }
    }

    on_off_plugin_unit::config_t plugin_unit_config;
    plugin_unit_config.on_off.on_off = false;
    endpoint_t *endpoint = on_off_plugin_unit::create(node, &plugin_unit_config, ENDPOINT_FLAG_NONE, plug);

    if (!endpoint) {
        ESP_LOGE(TAG, "Matter endpoint creation failed");
        return ESP_FAIL;
    }

    // GPIO pin Initialization
    err = app_driver_plugin_unit_init(plug);

    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize plug");
    }

    // Check for maximum plugs that can be configured.
    if (configure_plugs < CONFIG_NUM_VIRTUAL_PLUGS) {
        plugin_unit_list[configure_plugs].plug = plug->GPIO_PIN_VALUE;
        plugin_unit_list[configure_plugs].endpoint_id = endpoint::get_id(endpoint);
        configure_plugs++;
    } else {
        ESP_LOGE(TAG, "Maximum plugs configuration limit exceeded!!!");
        return ESP_FAIL;
    }

    uint16_t plug_endpoint_id = endpoint::get_id(endpoint);
    ESP_LOGI(TAG, "Plug created with endpoint_id %d", plug_endpoint_id);
    return err;
}



#include "esp_matter_core.h"

#define REBOOT_BUTTON_GPIO 9
//-- Hold for 10 seconds to reboot
#define REBOOT_HOLD_TIME_MS 3000
//-- Hold for 10 seconds to factory reset
#define FACTORY_RESET_HOLD_TIME_MS 10000

void reboot_button_task(void *pvParameter)
{
  uint32_t button_pressed_time = 0;
  bool button_pressed = false;

  //-- Configure Reset GPIO
  gpio_config_t io_conf = {
    .pin_bit_mask = (1ULL << (gpio_num_t)REBOOT_BUTTON_GPIO),
    .mode = GPIO_MODE_INPUT,
    .pull_up_en = GPIO_PULLUP_ENABLE,
    .pull_down_en = GPIO_PULLDOWN_DISABLE,
    .intr_type = GPIO_INTR_DISABLE,
  };
  gpio_config(&io_conf);

  while(1) {
    //-- Reset button pressed
    if(gpio_get_level((gpio_num_t)REBOOT_BUTTON_GPIO) == 0) {
      if(!button_pressed) {
        button_pressed_time = esp_log_timestamp();
        button_pressed = true;
        //ESP_LOGI("RebootButton", "Button pressed");
      }
    } else {
      //-- Reset button released
      if(button_pressed) {
        //ESP_LOGI("RebootButton", "Button released");

        uint32_t pressed_duration = esp_log_timestamp() - button_pressed_time;
        
        if(pressed_duration >= FACTORY_RESET_HOLD_TIME_MS) {
          ESP_LOGE("RebootButton", "\nFactory reset triggered\n");
          esp_matter::factory_reset();
          esp_restart();
        } else if (pressed_duration >= REBOOT_HOLD_TIME_MS) {
          ESP_LOGW("RebootButton", "\nReboot triggered\n");
          esp_restart();
        }

      }
      button_pressed = false;
    }
    vTaskDelay(pdMS_TO_TICKS(10));
  }
}



#if APP_USE_LED_INDICATOR

#include "led_indicator.h"

#define WS2812_GPIO_NUM       8
#define WS2812_STRIPS_NUM     1

#define LED_STRIP_RMT_RES_HZ  (10 * 1000 * 1000)

static const char *TAG_INDICATOR = "led_strips";
static led_indicator_handle_t led_handle = NULL;


/**
 * @brief Define blinking type and priority.
 *
 */
enum {
    BLINK_DOUBLE_RED = 0,
    BLINK_TRIPLE_GREEN,
    BLINK_WHITE_BREATHE_SLOW,
    BLINK_WHITE_BREATHE_FAST,
    BLINK_BLUE_BREATH,
    BLINK_COLOR_HSV_RING,
    BLINK_COLOR_RGB_RING,
#if WS2812_STRIPS_NUM > 1
    BLINK_FLOWING,
#endif
    BLINK_MAX,
};

/**
 * @brief Blinking twice times in red has a priority level of 0 (highest).
 *
 */
static const blink_step_t double_red_blink[] = {
    /*!< Set color to red by R:255 G:0 B:0 */
    {LED_BLINK_RGB, SET_RGB(255, 0, 0), 0},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Blinking three times in green with a priority level of 1.
 *
 */
static const blink_step_t triple_green_blink[] = {
    /*!< Set color to green by R:0 G:255 B:0 */
    {LED_BLINK_RGB, SET_RGB(0, 255, 0), 0},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_HOLD, LED_STATE_ON, 500},
    {LED_BLINK_HOLD, LED_STATE_OFF, 500},
    {LED_BLINK_STOP, 0, 0},
};

/**
 * @brief Slow breathing in white with a priority level of 2.
 *
 */
static const blink_step_t breath_white_slow_blink[] = {
    /*!< Set Color to white and brightness to zero by H:0 S:0 V:0 */
    {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Fast breathing in white with a priority level of 3.
 *
 */
static const blink_step_t breath_white_fast_blink[] = {
    /*!< Set Color to white and brightness to zero by H:0 S:0 V:0 */
    {LED_BLINK_HSV, SET_HSV(0, 0, 0), 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 500},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 500},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Breathing in green with a priority level of 4.
 *
 */
static const blink_step_t breath_blue_blink[] = {
    /*!< Set Color to blue and brightness to zero by H:240 S:255 V:0 */
    {LED_BLINK_HSV, SET_HSV(240, MAX_SATURATION, 0), 0},
    {LED_BLINK_BREATHE, LED_STATE_ON, 1000},
    {LED_BLINK_BREATHE, LED_STATE_OFF, 1000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Color gradient by HSV with a priority level of 5.
 *
 */
static const blink_step_t color_hsv_ring_blink[] = {
    /*!< Set Color to RED */
    {LED_BLINK_HSV, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
    {LED_BLINK_HSV_RING, SET_HSV(240, MAX_SATURATION, 127), 2000},
    {LED_BLINK_HSV_RING, SET_HSV(0, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
    {LED_BLINK_LOOP, 0, 0},
};

/**
 * @brief Color gradient by RGB with a priority level of 5.
 *
 */
static const blink_step_t color_rgb_ring_blink[] = {
    /*!< Set Color to Green */
    {LED_BLINK_RGB, SET_RGB(0, 255, 0), 0},
    {LED_BLINK_RGB_RING, SET_RGB(255, 0, 255), 2000},
    {LED_BLINK_RGB_RING, SET_RGB(0, 255, 0), 2000},
    {LED_BLINK_LOOP, 0, 0},
};

#if WS2812_STRIPS_NUM > 1
/**
 * @brief Flowing lights with a priority level of 6(lowest).
 *        Insert the index:MAX_INDEX to control all the strips
 *
 */
static const blink_step_t flowing_blink[] = {
    {LED_BLINK_HSV, SET_IHSV(MAX_INDEX, 0, MAX_SATURATION, MAX_BRIGHTNESS), 0},
    {LED_BLINK_HSV_RING, SET_IHSV(MAX_INDEX, MAX_HUE, MAX_SATURATION, MAX_BRIGHTNESS), 2000},
    {LED_BLINK_LOOP, 0, 0},
};
#endif

blink_step_t const *led_mode[] = {
    [BLINK_DOUBLE_RED] = double_red_blink,
    [BLINK_TRIPLE_GREEN] = triple_green_blink,
    [BLINK_WHITE_BREATHE_SLOW] = breath_white_slow_blink,
    [BLINK_WHITE_BREATHE_FAST] = breath_white_fast_blink,
    [BLINK_BLUE_BREATH] = breath_blue_blink,
    [BLINK_COLOR_HSV_RING] = color_hsv_ring_blink,
    [BLINK_COLOR_RGB_RING] = color_rgb_ring_blink,
#if WS2812_STRIPS_NUM > 1
    [BLINK_FLOWING] = flowing_blink,
#endif
    [BLINK_MAX] = NULL,
};

#if CONFIG_EXAMPLE_ENABLE_CONSOLE_CONTROL
static void led_cmd_cb(cmd_type_t cmd_type, uint32_t mode_index)
{
    switch (cmd_type) {
    case START:
        led_indicator_start(led_handle, mode_index);
        ESP_LOGI(TAG_INDICATOR, "start blink: %"PRIu32"", mode_index);
        break;
    case STOP:
        led_indicator_stop(led_handle, mode_index);
        ESP_LOGI(TAG_INDICATOR, "stop blink: %"PRIu32"", mode_index);
        break;
    case PREEMPT_START:
        led_indicator_preempt_start(led_handle, mode_index);
        ESP_LOGI(TAG_INDICATOR, "preempt start blink: %"PRIu32"", mode_index);
        break;
    case PREEMPT_STOP:
        led_indicator_preempt_stop(led_handle, mode_index);
        ESP_LOGI(TAG_INDICATOR, "preempt stop blink: %"PRIu32"", mode_index);
        break;
    default:
        ESP_LOGE(TAG_INDICATOR, "unknown cmd type: %d", cmd_type);
        break;
    }
}
#endif


#endif



extern "C" void app_main()
{
  //-- Start reboot button task
  xTaskCreate(reboot_button_task, "reboot_button_task", 2048, NULL, 5, NULL);
  //-- Start led indicato task

  esp_err_t err = ESP_OK;

  //-- Initialize the ESP NVS layer
  nvs_flash_init();

  //-- Create a Matter node and add the mandatory Root Node device type on endpoint 0
  node::config_t node_config;

  //-- node handle can be used to add/modify other endpoints
  node_t *node = node::create(&node_config, app_attribute_update_cb, app_identification_cb);
  ABORT_APP_ON_FAILURE(node != nullptr, ESP_LOGE(TAG, "Failed to create Matter node"));

  #ifdef CONFIG_GPIO_PLUG_1
    CREATE_PLUG(node, 1)
  #endif

  #ifdef CONFIG_GPIO_PLUG_2
    CREATE_PLUG(node, 2)
  #endif

  #ifdef CONFIG_GPIO_PLUG_3
    CREATE_PLUG(node, 3)
  #endif

  #ifdef CONFIG_GPIO_PLUG_4
    CREATE_PLUG(node, 4)
  #endif

  #ifdef CONFIG_GPIO_PLUG_5
    CREATE_PLUG(node, 5)
  #endif

  #ifdef CONFIG_GPIO_PLUG_6
    CREATE_PLUG(node, 6)
  #endif

  #ifdef CONFIG_GPIO_PLUG_7
    CREATE_PLUG(node, 7)
  #endif

  #ifdef CONFIG_GPIO_PLUG_8
    CREATE_PLUG(node, 8)
  #endif

  #ifdef CONFIG_GPIO_PLUG_9
    CREATE_PLUG(node, 9)
  #endif

  #ifdef CONFIG_GPIO_PLUG_10
    CREATE_PLUG(node, 10)
  #endif

  #ifdef CONFIG_GPIO_PLUG_11
    CREATE_PLUG(node, 11)
  #endif

  #ifdef CONFIG_GPIO_PLUG_12
    CREATE_PLUG(node, 12)
  #endif

  #ifdef CONFIG_GPIO_PLUG_13
    CREATE_PLUG(node, 13)
  #endif

  #ifdef CONFIG_GPIO_PLUG_14
    CREATE_PLUG(node, 14)
  #endif

  #ifdef CONFIG_GPIO_PLUG_15
    CREATE_PLUG(node, 15)
  #endif

  #ifdef CONFIG_GPIO_PLUG_16
    CREATE_PLUG(node, 16)
  #endif

  //-- Initialize factory reset button
  app_driver_handle_t button_handle = app_driver_button_init(&reset_gpio);
  if (button_handle) {
      app_reset_button_register(button_handle);
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
  ABORT_APP_ON_FAILURE(err == ESP_OK, ESP_LOGE(TAG, "Failed to start Matter, err:%d", err));

  #if CONFIG_ENABLE_CHIP_SHELL
    esp_matter::console::diagnostics_register_commands();
    esp_matter::console::wifi_register_commands();
    esp_matter::console::factoryreset_register_commands();
    #if CONFIG_OPENTHREAD_CLI
      esp_matter::console::otcli_register_commands();
    #endif
    esp_matter::console::init();
  #endif

  #if APP_USE_LED_STRIP
    /*****************
     *               *
     *   LED STRIP   *
     *               *
     *****************/
    
    led_strip_handle_t led_strip = configure_led();
    uint8_t led_on_off = 0, r = 0, g = 0, b = 0;
    
    ESP_LOGI(TAG_LED, "Start blinking LED strip");
    while (1) {
      switch(led_on_off) {
        case 0:
          r = 8; g = 8; b = 8;
          ESP_LOGI(TAG_LED, "color = WHITE!");
          break;
        case 1:
          r = 8; g = 0; b = 0;
          ESP_LOGE(TAG_LED, "color = RED!");
          break;
        case 2:
          r = 0; g = 8; b = 0;
          ESP_LOGI(TAG_LED, "color = GREEN!");
          break;
        case 3:
          r = 0; g = 0; b = 8;
          ESP_LOGW(TAG_LED, "color = BLUE!");
          break;
        default:
          //-- Set all LED off to clear all pixels
          ESP_ERROR_CHECK(led_strip_clear(led_strip));
          ESP_LOGI(TAG_LED, "LED OFF!");
          break;
      }
      //-- Set the LED pixel using RGB from 0 (0%) to 255 (100%) for each color
      for(int i = 0; i < LED_STRIP_LED_NUMBERS; i++) {
        ESP_ERROR_CHECK(led_strip_set_pixel(led_strip, i, r, g, b));
      }
      //-- Refresh the strip to send data
      ESP_ERROR_CHECK(led_strip_refresh(led_strip));
    
      led_on_off++;
      if(led_on_off > 3) {
        led_on_off = 0;
      }
      
      vTaskDelay(pdMS_TO_TICKS(1500));
    }
  #endif

  #if CHIP_DEVICE_CONFIG_ENABLE_THREAD
    ESP_LOGW(TAG, "*** CHIP_DEVICE_CONFIG_ENABLE_THREAD ***");
  #endif


  #if APP_USE_LED_INDICATOR
    /*********************
     *                   *
     *   LED INDICATOR   *
     *                   *
     *********************/

    //-- LED strip general initialization, according to your led board design
    const led_strip_config_t strip_config = {
      .strip_gpio_num = WS2812_GPIO_NUM,   // The GPIO that connected to the LED strip's data line
      .max_leds = WS2812_STRIPS_NUM,        // The number of LEDs in the strip,
      .led_pixel_format = LED_PIXEL_FORMAT_GRB, // Pixel format of your LED strip
      .led_model = LED_MODEL_WS2812,            // LED strip model
      {
        .invert_out = false,                    // whether to invert the output signal
      },
    };
  
    //-- LED strip backend configuration: RMT
    const led_strip_rmt_config_t rmt_config = {
      .clk_src = RMT_CLK_SRC_DEFAULT,        // different clock source can lead to different power consumption
      .resolution_hz = LED_STRIP_RMT_RES_HZ, // RMT counter clock frequency
      .mem_block_symbols = 0,
      {
        .with_dma = false,                   // DMA feature is available on ESP target like ESP32-S3
      },
    };
    
    
    led_indicator_strips_config_t strips_config = {
        .led_strip_cfg = strip_config,
        .led_strip_driver = LED_STRIP_RMT,
        .led_strip_rmt_cfg = rmt_config,
    };

    const led_indicator_config_t config_i = {
        .mode = LED_STRIPS_MODE,
        .led_indicator_strips_config = &strips_config,
        .blink_lists = led_mode,
        .blink_list_num = BLINK_MAX,
    };

    led_handle = led_indicator_create(&config_i);
    assert(led_handle != NULL);

#if CONFIG_EXAMPLE_ENABLE_CONSOLE_CONTROL
    cmd_led_indicator_t cmd_led_indicator = {
        .cmd_cb = led_cmd_cb,
        .mode_count = BLINK_MAX,
    };

    ESP_ERROR_CHECK(cmd_led_indicator_init(&cmd_led_indicator));
#else
    while (1) {
        for (int i = 0; i < BLINK_MAX; i++) {
            led_indicator_start(led_handle, i);
            ESP_LOGI(TAG_INDICATOR, "start blink: %d", i);
            vTaskDelay(4000 / portTICK_PERIOD_MS);
            led_indicator_stop(led_handle, i);
            ESP_LOGI(TAG_INDICATOR, "stop blink: %d", i);
            vTaskDelay(1000 / portTICK_PERIOD_MS);
        }
    }
#endif
	#endif

}
