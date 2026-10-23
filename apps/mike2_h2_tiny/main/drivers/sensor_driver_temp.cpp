//#include <stdlib.h>
//#include <time.h>
#include "esp_random.h"

#include "sensor_driver_temp.h"
#include "app_priv.h"

esp_err_t temp_init()
{
  srand(time(NULL));
  
  return ESP_OK;
}

esp_err_t temp_read(int16_t *temperature)
{
  //float temp = 26.12;
  
  // случайное целое от 2000 до 3000 (в тысячных долях градуса)
  //int random_milli = 2000 + rand() % 1001;   // 2000..3000
  //float temp = random_milli / 1000.0f;        // 2.000f..3.000f → 20.000..30.000
  
  // нормализованное значение [0.0, 1.0)
  float rnd = (float)esp_random() / (float)UINT32_MAX;
  float temp = 20.0f + rnd * 10.0f;   // 20.00..30.00
    
  *temperature = (int16_t)(temp * 100.0f);  // 0.01°C
  return ESP_OK;
}

esp_err_t temp_reset()
{
  return ESP_OK;
}
