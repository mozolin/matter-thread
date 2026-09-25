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

esp_err_t ds18b20_init(ds18b20_dev_t *dev, gpio_num_t data_pin)
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
    gpio_set_pull_mode(SENSOR_GPIO, GPIO_PULLUP_ONLY);

    return ESP_OK;
}

esp_err_t ds18b20_read(ds18b20_dev_t *dev, int16_t *temperature)
{
    esp_err_t res;
    float temp;

    temp = 0;

    // Every RESCAN_INTERVAL samples, check to see if the sensors connected
    // to our bus have changed.
    res = ds18x20_scan_devices(SENSOR_GPIO, addrs, MAX_SENSORS, &sensor_count);
    if (res != ESP_OK)
    {
        ESP_LOGE(TAG_MULTI_SENSOR, "DS18B20: Sensors scan error %d (%s)", res, esp_err_to_name(res));
        return ESP_OK;
    }

    if (!sensor_count)
    {
        ESP_LOGE(TAG_MULTI_SENSOR, "DS18B20: No sensors detected!");
        return ESP_OK;
    }

    //ESP_LOGI(TAG_MULTI_SENSOR, "DS18B20: %d sensors detected", sensor_count);

    // If there were more sensors found than we have space to handle,
    // just report the first MAX_SENSORS..
    if (sensor_count > MAX_SENSORS)
        sensor_count = MAX_SENSORS;

    // Do a number of temperature samples, and print the results.
        //ESP_LOGI(TAG_MULTI_SENSOR, "Measuring...");

        res = ds18x20_measure_and_read_multi(SENSOR_GPIO, addrs, sensor_count, temps);
        if (res != ESP_OK)
        {
            ESP_LOGE(TAG_MULTI_SENSOR, "Sensors read error %d (%s)", res, esp_err_to_name(res));
            //continue;
            return ESP_OK;
        }

        for (int j = 0; j < sensor_count; j++)
        {
            float temp_c = temps[j];
            //float temp_f = (temp_c * 1.8) + 32;

            temp = temp_c;

            ESP_LOGD("| DS18B20", "Temp: %.2f °C (" "%08" PRIx32 "%08" PRIx32 ")",
                     temp_c,
                     (uint32_t)(addrs[j] >> 32), (uint32_t)addrs[j]);
        }

    
    *temperature = (int16_t)(temp * 100.0f);  // 0.01°C
    return ESP_OK;
}

esp_err_t ds18b20_reset(ds18b20_dev_t *dev)
{
    //if (!dev || !dev->initialized) return ESP_ERR_INVALID_STATE;
    return ESP_OK;  // DS18B20 не требует reset, просто перечитать
}
