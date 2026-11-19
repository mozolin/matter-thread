#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "drivers/sensor_common.h"


#define CONFIG_MOCK_SENSORS_BEHAVIOR           false


/***********************
 *                     *
 *   TASK PRIORITIES   *
 *                     *
 ***********************/
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5


/**********************
 *                    *
 *   USE OF SENSORS   *
 *                    *
 **********************/
//-- Sensors configuration
#define CONFIG_BME280_ENABLED                  true
#define CONFIG_BME680_ENABLED                  true
#define CONFIG_DS18B20_ENABLED                 true
#define CONFIG_DHT11_ENABLED                   true
#define CONFIG_MQ135_ENABLED                   true
#define CONFIG_SHT4X_ENABLED                   true
#define CONFIG_SCD4X_ENABLED                   false


/****************************************
 *                                      *
 *   I2C BUSES (0 and 1 for ESP32-H2)   *
 *                                      *
 ****************************************/
#include "i2c_adv.h"


//-- Reboot button handlers
#if CONFIG_REBOOT_BUTTON_ENABLED
  #include <app/server/CommissioningWindowManager.h>
  #include <app/server/Server.h>
  #include "driver_reboot_button.h"
#endif


//-- LED Indicator
#include "driver_led_esp32h2.h"
#if CONFIG_USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
#endif

//-- Boot Switch
#if CONFIG_BOOT_SWITCH_ENABLED
  #include "driver_boot_switch.h"
#endif


/************************
 *                      *
 *   SENSORS SETTINGS   *
 *                      *
 ************************/
//-- BMP280 sensor (i2c bus=0,i2c_addr=0x76)
#define CONFIG_BME280_SDA_GPIO                 CONFIG_I2C_0_SDA_GPIO
#define CONFIG_BME280_SCL_GPIO                 CONFIG_I2C_0_SCL_GPIO
#define CONFIG_BME280_I2C_PORT                 CONFIG_I2C_0_PORT_NUM
//-- BME680 sensor (i2c bus=1,i2c_addr=0x77)
#define CONFIG_BME680_SDA_GPIO                 CONFIG_I2C_1_SDA_GPIO
#define CONFIG_BME680_SCL_GPIO                 CONFIG_I2C_1_SCL_GPIO
#define CONFIG_BME680_I2C_PORT                 CONFIG_I2C_1_PORT_NUM
//-- DS18B20 sensor
#define CONFIG_DS18B20_GPIO                    10
//-- DHT11 sensor
#define CONFIG_DHT11_GPIO                      11
//-- MQ135 sensor (ESP32-H2:GPIO4,ADC_UNIT_1,ADC_CHANNEL_3)
#define CONFIG_MQ135_GPIO                      4
#define CONFIG_MQ135_ADC_UNIT                  ADC_UNIT_1
#define CONFIG_MQ135_ADC_CHANNEL               ADC_CHANNEL_3
#define CONFIG_MQ135_3V3_CIRCUIT               true
#define CONFIG_MQ135_CALIBRATE                 false
//-- SHT4X sensor (i2c bus=1,i2c_addr=0x44)
//#define CONFIG_SHT4X_SDA_GPIO                  CONFIG_I2C_0_SDA_GPIO
//#define CONFIG_SHT4X_SCL_GPIO                  CONFIG_I2C_0_SCL_GPIO
//#define CONFIG_SHT4X_I2C_PORT                  CONFIG_I2C_0_PORT_NUM
#define CONFIG_SHT4X_SDA_GPIO                  CONFIG_I2C_1_SDA_GPIO
#define CONFIG_SHT4X_SCL_GPIO                  CONFIG_I2C_1_SCL_GPIO
#define CONFIG_SHT4X_I2C_PORT                  CONFIG_I2C_1_PORT_NUM
//-- SCD4X sensor (i2c bus=1,i2c_addr=0x62)
#define CONFIG_SCD4X_SDA_GPIO                  CONFIG_I2C_1_SDA_GPIO
#define CONFIG_SCD4X_SCL_GPIO                  CONFIG_I2C_1_SCL_GPIO
#define CONFIG_SCD4X_I2C_PORT                  CONFIG_I2C_1_PORT_NUM



#if CONFIG_ESP_EMULATOR_MODE
  #define CONFIG_SENSOR_POLL_PERIOD_MS         15000
#else
  #define CONFIG_SENSOR_POLL_PERIOD_MS         5000
#endif


#if CONFIG_MQ135_ENABLED
  #include <esp_matter.h>
  #if CONFIG_MQ135_3V3_CIRCUIT
    #include "sensor_driver_mq135_3v3.h"
  #else
    #include "sensor_driver_mq135.h"
  #endif
  
  extern mq135_handle_t s_mq135_handle;
  extern cluster_t *s_air_quality_cluster;
  extern cluster_t *s_co2_cluster;
  extern cluster_t *s_co_cluster;
  extern cluster_t *s_tvoc_cluster;

  //-- AirQuality:
  //-- 0 = Unknown / Неизвестно
  #define CONFIG_MQ135_AQ0_UNKNOWN             -1
  //-- 1 = Good / Хорошее
  #define CONFIG_MQ135_AQ1_GOOD                0
  //-- 2 = Fair / Удовлетворительное
  #define CONFIG_MQ135_AQ2_FAIR                1000
  //-- 3 = Moderate / Умеренное
  #define CONFIG_MQ135_AQ3_MODERATE            2000
  //-- 4 = Poor / Плохое
  #define CONFIG_MQ135_AQ4_POOR                3000
  //-- 5 = VeryPoor / Очень плохое
  #define CONFIG_MQ135_AQ5_VERYPOOR            4000
  //-- 6 = ExtremelyPoor / Крайне плохое
  #define CONFIG_MQ135_AQ6_EXTREMELYPOOR       5000
#endif

#if CONFIG_SCD4X_ENABLED
  //-- AirQuality:
  //-- 0 = Unknown / Неизвестно
  #define CONFIG_SCD4X_AQ0_UNKNOWN             -1
  //-- 1 = Good / Хорошее
  #define CONFIG_SCD4X_AQ1_GOOD                0
  //-- 2 = Fair / Удовлетворительное
  #define CONFIG_SCD4X_AQ2_FAIR                1000
  //-- 3 = Moderate / Умеренное
  #define CONFIG_SCD4X_AQ3_MODERATE            2000
  //-- 4 = Poor / Плохое
  #define CONFIG_SCD4X_AQ4_POOR                3000
  //-- 5 = VeryPoor / Очень плохое
  #define CONFIG_SCD4X_AQ5_VERYPOOR            4000
  //-- 6 = ExtremelyPoor / Крайне плохое
  #define CONFIG_SCD4X_AQ6_EXTREMELYPOOR       5000
#endif

#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306_common.h"
  #include "main_driver_ssd1306.h"
  #include "ssd1306_i2cdev.h"
#endif


#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
  #include "esp_openthread_types.h"
#endif

extern sensor_endpoint_mapping_t sensor_mapping_list[SENSOR_TYPE_MAX];
extern uint16_t configured_sensors;
extern sensor_data_t sensors[SENSOR_TYPE_MAX];

typedef void *app_driver_handle_t;

/** Sensor initialization
 *
 * This API is called to initialize sensors
 *
 * @param[in] sensor_config Sensor configuration.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_sensor_init(sensor_type_t sensor_config);

/** Read sensor data
 *
 * This reads the current values from the sensor
 *
 * @param[in] sensor_idx Index of the sensor in sensors array
 *
 * @return ESP_OK on success
 */
esp_err_t app_driver_read_sensor_data(uint8_t sensor_idx);

/** Task to poll sensors
 *
 * This task periodically reads sensor values and updates Matter attributes
 */
void sensor_polling_task(void *pvParameters);

#if CONFIG_DEBUG_MODE || CONFIG_ESP_EMULATOR_MODE
  void app_driver_log_sensor_statistics(void);
#endif

#if CHIP_DEVICE_CONFIG_ENABLE_THREAD
#define ESP_OPENTHREAD_DEFAULT_RADIO_CONFIG()                                           \
    {                                                                                   \
        .radio_mode = RADIO_MODE_NATIVE,                                                \
    }

#define ESP_OPENTHREAD_DEFAULT_HOST_CONFIG()                                            \
    {                                                                                   \
        .host_connection_mode = HOST_CONNECTION_MODE_NONE,                              \
    }

#define ESP_OPENTHREAD_DEFAULT_PORT_CONFIG()                                            \
    {                                                                                   \
        .storage_partition_name = "nvs", .netif_queue_size = 10, .task_queue_size = 10, \
    }
#endif

#if CONFIG_SSD1306_ENABLED
  #ifdef CONFIG_SSD1306_APP_RUN_CB
    //-- callback function specific to this application
    //-- (can be used within the SSD1306 common driver)
    extern void ssd1306_app_run_cb(void);
  #endif
#endif

esp_err_t app_attribute_update_main_cb(esp_matter::attribute::callback_type_t type, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val, void *priv_data);
esp_err_t app_driver_attribute_update_main(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id, uint32_t attribute_id, esp_matter_attr_val_t *val);
