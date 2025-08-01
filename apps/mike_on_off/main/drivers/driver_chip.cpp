//-- ESP32-H2: ~/esp-matter/connectedhomeip/connectedhomeip/third_party/mbed-mcu-boot/repo/boot/espressif/hal/esp-idf/components/driver/esp32h2
//-- ESP32-C6: https://docs.espressif.com/projects/esp-idf/en/v5.4/esp32c6/api-reference/peripherals/temp_sensor.html

#include <app_priv.h>

#if USE_INTERNAL_TEMPERATURE
	#include "driver/temperature_sensor.h"

	//-- Getting chip temperature
	float read_internal_temperature()
	{
    temperature_sensor_handle_t temp_handle = NULL;
    temperature_sensor_config_t temp_sensor_config = TEMPERATURE_SENSOR_CONFIG_DEFAULT(20, 50);
    ESP_ERROR_CHECK(temperature_sensor_install(&temp_sensor_config, &temp_handle));
    // Enable temperature sensor
    ESP_ERROR_CHECK(temperature_sensor_enable(temp_handle));
    // Get converted sensor data
    float tsens_out;
    ESP_ERROR_CHECK(temperature_sensor_get_celsius(temp_handle, &tsens_out));
    // Disable the temperature sensor if it is not needed and save the power
    ESP_ERROR_CHECK(temperature_sensor_disable(temp_handle));
    ESP_ERROR_CHECK(temperature_sensor_uninstall(temp_handle));
    return tsens_out;
	}
#endif

#if USE_INTERNAL_VOLTAGE
  #include "driver_chip.h"
  #include "esp_adc/adc_oneshot.h"
  #include "esp_adc/adc_cali.h"
  #include "esp_adc/adc_cali_scheme.h"
  //#include "esp_bit_defs.h"
  //#include "hal/adc_types.h"

  bool adc_initialized = false;
  adc_oneshot_unit_handle_t adc_handle = NULL;
  adc_cali_handle_t cali_handle = NULL;
  //-- GPIO1 - ADC_CHANNEL_1
  adc_channel_t adc_channel = ADC_CHANNEL_0;

  bool init_internal_voltage()
  {
    //-- ADC Unit Initialization
    adc_oneshot_unit_init_cfg_t unit_config = {
      .unit_id = ADC_UNIT_1,
      .ulp_mode = ADC_ULP_MODE_DISABLE,
    };
    ESP_ERROR_CHECK(adc_oneshot_new_unit(&unit_config, &adc_handle));
    
    //-- Channel configuration
    adc_oneshot_chan_cfg_t chan_config = {
      .atten = ADC_ATTEN_DB_12,
      .bitwidth = ADC_BITWIDTH_DEFAULT,
    };
    ESP_ERROR_CHECK(adc_oneshot_config_channel(adc_handle, adc_channel, &chan_config));
    
    //-- Calibration initializing 
    #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
      /*
      if(!adc_cali_check_scheme(ADC_CALI_SCHEME_VER_CURVE_FITTING)) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Curve fitting scheme not supported");
        return false;
      }
      */
      adc_cali_curve_fitting_config_t cali_config_curve = {
        .unit_id = ADC_UNIT_1,
        .chan = adc_channel,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
      };
      if(adc_cali_create_scheme_curve_fitting(&cali_config_curve, &cali_handle) != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create calibration scheme");
        return false;
      }
    #endif
    
    #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
      /*
      if(!adc_cali_check_scheme(ADC_CALI_SCHEME_VER_LINE_FITTING)) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Line fitting scheme not supported");
        return false;
      }
      */
      adc_cali_line_fitting_config_t cali_config_line = {
        .unit_id = ADC_UNIT_1,
        .atten = ADC_ATTEN_DB_12,
        .bitwidth = ADC_BITWIDTH_DEFAULT,
      };
      if(adc_cali_create_scheme_line_fitting(&cali_config_line, &cali_handle) != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to create calibration scheme");
        return false;
      }
    #endif
    
    adc_initialized = true;
    ESP_LOGW(TAG_MIKE_APP, "~~~ ADC initialized on channel %d", adc_channel);
    return true;
  }


  float read_internal_voltage()
  {
    if(!adc_initialized) {
      return 0.0f;
    }
    
    int raw_value;
    ESP_ERROR_CHECK(adc_oneshot_read(adc_handle, adc_channel, &raw_value));
    
    int voltage_mv;
    if(adc_cali_raw_to_voltage(cali_handle, raw_value, &voltage_mv) != ESP_OK) {
      ESP_LOGE(TAG_MIKE_APP, "~~~ Failed to convert ADC value");
      return 0.0f;
    } else {
      ESP_LOGW(TAG_MIKE_APP, "~~~ Channel %d, ADC value converted to %d mV", adc_channel, voltage_mv);
    }
    
    return voltage_mv / 1000.0f;
  }


  void deinit_internal_voltage()
  {
    if(cali_handle) {
      #if ADC_CALI_SCHEME_CURVE_FITTING_SUPPORTED
        adc_cali_delete_scheme_curve_fitting(cali_handle);
      #endif

      #if ADC_CALI_SCHEME_LINE_FITTING_SUPPORTED
        adc_cali_delete_scheme_line_fitting(cali_handle);
      #endif
    }
    if(adc_handle) {
      adc_oneshot_del_unit(adc_handle);
    }
  }
#endif
