#include "sensor_driver_ds18b20.h"
#include "app_priv.h"

#define CONFIG_EXAMPLE_DS18X20_MAX_SENSORS 8

static const gpio_num_t SENSOR_GPIO = (gpio_num_t)CONFIG_DS18B20_GPIO;
static const int MAX_SENSORS = CONFIG_EXAMPLE_DS18X20_MAX_SENSORS;
static const int RESCAN_INTERVAL = 8;
static const uint32_t LOOP_DELAY_MS = 500;

onewire_addr_t addrs[MAX_SENSORS];
float temps[MAX_SENSORS];
size_t sensor_count = 0;

esp_err_t ds18b20_init()
{
    // There is no special initialization required before using the ds18x20
    // routines.  However, we make sure that the internal pull-up resistor is
    // enabled on the GPIO pin so that one can connect up a sensor without
    // needing an external pull-up (Note: The internal (~47k) pull-ups of the
    // ESP do appear to work, at least for simple setups (one or two sensors
    // connected with short leads), but do not technically meet the pull-up
    // requirements from the ds18x20 datasheet and may not always be reliable.
    // For a real application, a proper 4.7k external pull-up resistor is
    // recommended instead!)
    #if !MOCK_SENSORS_BEHAVIOR
      gpio_set_pull_mode(SENSOR_GPIO, GPIO_PULLUP_ONLY);
    #endif

    return ESP_OK;
}

esp_err_t ds18b20_read(int16_t *temperature)
{
    float temp;

    temp = 0;

    #if !MOCK_SENSORS_BEHAVIOR
      esp_err_t res;
      
      // Every RESCAN_INTERVAL samples, check to see if the sensors connected
      // to our bus have changed.
      res = ds18x20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS, &sensor_count);
      if(res != ESP_OK)
      {
        ESP_LOGE(TAG_MULTI_CLIMATE, "DS18B20: Sensors scan error %d (%s)", res, esp_err_to_name(res));
        return ESP_OK;
      }
      
      if(!sensor_count)
      {
        ESP_LOGE(TAG_MULTI_CLIMATE, "DS18B20: No sensors detected!");
        return ESP_OK;
      }
      
      #if DO_DEBUG
        ESP_LOGW(TAG_MULTI_CLIMATE, "DS18B20: %d sensors detected", sensor_count);
      #endif
      
      // If there were more sensors found than we have space to handle,
      // just report the first MAX_SENSORS..
      if(sensor_count > MAX_SENSORS) {
        sensor_count = MAX_SENSORS;
      }
      
      // Do a number of temperature samples, and print the results.
      //ESP_LOGI(TAG_MULTI_CLIMATE, "Measuring...");
      
      res = ds18x20_measure_and_read_multi(SENSOR_GPIO, addrs, sensor_count, temps);
      if(res != ESP_OK)
      {
        ESP_LOGE(TAG_MULTI_CLIMATE, "Sensors read error %d (%s)", res, esp_err_to_name(res));
        return ESP_OK;
      }
      
      for(int j = 0; j < sensor_count; j++)
      {
        float temp_c = temps[j];
        //float temp_f = (temp_c * 1.8) + 32;
      
        temp = temp_c;
      
        #if DO_DEBUG
          ESP_LOGW("| DS18B20", "Temp: %.2f °C (" "%08" PRIx32 "%08" PRIx32 ")", temp_c, (uint32_t)(addrs[j] >> 32), (uint32_t)addrs[j]);
        #else
          ESP_LOGD("| DS18B20", "Temp: %.2f °C (" "%08" PRIx32 "%08" PRIx32 ")", temp_c, (uint32_t)(addrs[j] >> 32), (uint32_t)addrs[j]);
        #endif
      }
    #else
      sensor_count = 4;
      temp = 26.12;
      
      #if DO_DEBUG
        ESP_LOGW(TAG_MULTI_CLIMATE, "DS18B20: %d sensors detected", sensor_count);
      #endif
      
      #if DO_DEBUG
        ESP_LOGW("| DS18B20", "Temp: 26.06 °C (35b7464e0b646128)");
        ESP_LOGW("| DS18B20", "Temp: 26.75 °C (ffa80a4e5f646328)");
        ESP_LOGW("| DS18B20", "Temp: 26.56 °C (f5e1ab495f646328)");
        ESP_LOGW("| DS18B20", "Temp: 26.12 °C (fd64324b5f646328)");
      #else
        ESP_LOGD("| DS18B20", "Temp: 26.06 °C (35b7464e0b646128)");
        ESP_LOGD("| DS18B20", "Temp: 26.75 °C (ffa80a4e5f646328)");
        ESP_LOGD("| DS18B20", "Temp: 26.56 °C (f5e1ab495f646328)");
        ESP_LOGD("| DS18B20", "Temp: 26.12 °C (fd64324b5f646328)");
      #endif
    #endif
    
    *temperature = (int16_t)(temp * 100.0f);  // 0.01°C
    return ESP_OK;
}

esp_err_t ds18b20_reset()
{
    //if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return ESP_OK;  // DS18B20 не требует reset, просто перечитать
}
