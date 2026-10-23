#include "sensor_driver_temp.h"
#include "app_priv.h"

esp_err_t temp_init()
{
  return ESP_OK;
}

esp_err_t temp_read(int16_t *temperature)
{
  float temp = 26.12;
    
  *temperature = (int16_t)(temp * 100.0f);  // 0.01°C
  return ESP_OK;
}

esp_err_t temp_reset()
{
  return ESP_OK;
}
