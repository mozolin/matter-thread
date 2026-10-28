#pragma once

#include <esp_err.h>
#include <esp_matter.h>
#include "soc/gpio_num.h"
#include "driver/gpio.h"
#include <button_gpio.h>
#include "plugin_device_structure.h"


//-- Debug options
#define MOCK_SENSORS_BEHAVIOR                  false
#define DEBUG_MODE                             false
#define SHOW_DEVICE_LOG                        false
#define TAG_MIKE_APP                           "MIKE MULTICLIMATE H2"


//-- task priorities
// ============================================================
// Задача опроса Boot Switch
// Приоритет 3 (ниже reboot_button=4 и sensor_poll=5),
// потому что:
//   • кнопка — событие для человека, задержка 20–50 мс не заметна
//   • reboot_button (4) уже опрашивает тот же GPIO — boot_switch
//     не должен его вытеснять
//   • sensor_poll (5) имеет более строгие тайминги (BME680/DS18B20)
//   • Matter-события уходят в HA без задержек
// ============================================================
#define CONFIG_COMMISSIONING_WINDOW_PRIORITY   6
#define CONFIG_SENSOR_POLL_TASK_PRIORITY       5
#define CONFIG_REBOOT_BUTTON_TASK_PRIORITY     4
#define CONFIG_BOOT_SWITCH_TASK_PRIORITY       3


//-- SSD1306 LED Display
#define CONFIG_SSD1306_ENABLED                 true


//-- Sensors configuration
#define CONFIG_BME280_ENABLED                  true
#define CONFIG_BME680_ENABLED                  true
#define CONFIG_DS18B20_ENABLED                 true
#define CONFIG_DHT11_ENABLED                   true
#define CONFIG_MQ135_ENABLED                   true
#define CONFIG_SHT41_ENABLED                   true


// ============================================================
// Boot Switch (Generic Switch для Home Assistant)
// Включает экспорт кнопки BOOT (GPIO 9) как Matter Generic Switch
// с событиями: одиночное нажатие, двойное нажатие, длинное нажатие (3 сек)
// ============================================================
#define CONFIG_BOOT_SWITCH_ENABLED             true
#if CONFIG_BOOT_SWITCH_ENABLED
  // Драйвер boot switch (реализация в driver_boot_switch.cpp)
  #include "driver_boot_switch.h"
#endif


//-- Reboot button handlers
#define CONFIG_REBOOT_BUTTON_ENABLED           true
#if CONFIG_REBOOT_BUTTON_ENABLED
  //--> uses for CommissioningWindowOpen
  #include <app/server/CommissioningWindowManager.h>
  #include <app/server/Server.h>
  //-- Window opens for NNN seconds
  #define CONFIG_COMMISSIONING_WINDOW_OPEN     60//300
  //<-- uses for CommissioningWindowOpen
  
  #include "driver_reboot_button.h"
  //-- Single-click: toggle SSD1306
  //-- Double-click: toggle LED Indicator
  #define REBOOT_BUTTON_GPIO                   9
  //-- Hold to reboot
  #define REBOOT_HOLD_TIME_MS                  3000
  //-- Hold to factory reset
  #define FACTORY_RESET_HOLD_TIME_MS           10000
  //=== параметры детекции двойного нажатия кнопки Reboot ===
  //-- интервал между первым и вторым нажатием для распознавания двойного клика (мс)
  #define DOUBLE_CLICK_INTERVAL_MS             400
  //-- максимальная длительность короткого нажатия (мс) — иначе это не "клик"
  #define SHORT_CLICK_MAX_MS                   350
#endif


//-- LED Indicator
#include "led_config.h"
#if USE_DRIVER_LED_INDICATOR
  #include "driver_led_indicator.h"
  extern led_indicator_handle_t led_handle;
#endif


//-- BMP280 sensor (i2c bus=0,i2c_addr=0x76)
#define CONFIG_BME280_SDA_GPIO                 1
#define CONFIG_BME280_SCL_GPIO                 2
#define CONFIG_BME280_I2C_PORT                 I2C_NUM_0
//-- BME680 sensor (i2c bus=1,i2c_addr=0x77)
#define CONFIG_BME680_SDA_GPIO                 3
#define CONFIG_BME680_SCL_GPIO                 5
#define CONFIG_BME680_I2C_PORT                 I2C_NUM_1
//-- DS18B20 sensor
#define CONFIG_DS18B20_GPIO                    10
//-- DHT11 sensor
#define CONFIG_DHT11_GPIO                      11
//-- MQ135 sensor (ESP32-H2:GPIO4,ADC_UNIT_1,ADC_CHANNEL_3)
#define CONFIG_MQ135_GPIO                      4
#define CONFIG_MQ135_ADC_UNIT                  ADC_UNIT_1
#define CONFIG_MQ135_ADC_CHANNEL               ADC_CHANNEL_3
#define CONFIG_MQ135_3V3_CIRCUIT               true
#define CONFIG_MQ135_CALIBRATE                 true
//-- SHT41 sensor (i2c bus=1,i2c_addr=0x44)
#define CONFIG_SHT41_SDA_GPIO                  3
#define CONFIG_SHT41_SCL_GPIO                  5
#define CONFIG_SHT41_I2C_PORT                  I2C_NUM_1

#define LIVE_BLINK_TIME_MS                     0
#define CONFIG_SENSOR_POLL_PERIOD_MS           5000

#if SHOW_DEVICE_LOG
	//-- what part of tree to show (-1 means: SHOW ALL)
  #define SHOW_DEVICE_LOG_ONE_CLUSTER   -      1
  #define SHOW_DEVICE_LOG_CLUSTERS             true
  #define SHOW_DEVICE_LOG_ATTRIBUTES           false
  #define SHOW_DEVICE_LOG_COMMANDS             false
#endif


#if CONFIG_MQ135_ENABLED
  #include <esp_matter.h>
  using namespace esp_matter;
  
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


#if CONFIG_SSD1306_ENABLED
  #include "driver_ssd1306.h"
  #include "ssd1306_i2cdev.h"
  /**********************************************************
   *
   *  This driver initializes the I2C bus
   *  on port CONFIG_SSD1306_I2C_PORT;
   *  therefore, any sensors using the I2C interface
   *  must be connected to the same GPIO pins (SDA and SCL)
   *  if they use this port.
   *
   **********************************************************/
  #define CONFIG_SSD1306_SDA_GPIO              1
  #define CONFIG_SSD1306_SCL_GPIO              2
  #define CONFIG_SSD1306_RESET_GPIO            -1
  #define CONFIG_SSD1306_I2C_ADDRESS           0x3C
  #define CONFIG_SSD1306_I2C_PORT              I2C_NUM_0
  
  extern bool ssd1306_initialized;
  extern SSD1306_t ssd1306dev;
  
  //=== флаг состояния дисплея и таймер авто-выключения ===
  extern bool ssd1306_display_enabled;       // true = экран включён
  extern uint64_t ssd1306_last_on_time_ms;   // момент последнего включения (ms)
  #define SSD1306_AUTO_OFF_TIMEOUT_MS          (5 * 60 * 1000)  // 5 минут
  // Рисует во второй строке экрана (y_pos = 1) оставшееся время в формате "MM:SS".
  extern void ssd1306_update_timeout_line(void);
#endif


// Sensor types
typedef enum {
  SENSOR_TYPE_BME280  = 0,
  SENSOR_TYPE_BME680  = 1,
  SENSOR_TYPE_DS18B20 = 2,
  SENSOR_TYPE_DHT11   = 3,
  SENSOR_TYPE_MQ135   = 4,
  SENSOR_TYPE_SHT41   = 5,
  SENSOR_TYPE_MAX,
} sensor_type_t;

// Sensor data structure
typedef struct {
  sensor_type_t config;
  float last_temperature;
  float last_humidity;
  float last_pressure;
  float last_gas_resistance;  // For BME680
  float last_co2;             // For MQ135
  float last_co;              // For MQ135
  float last_tvoc;            // For MQ135
  uint64_t last_read_time;
  uint8_t num_sensors;        // For DS18B20
} sensor_data_t;

// Endpoint-sensor mapping
typedef struct {
  uint16_t endpoint_id;
  sensor_type_t sensor_type;
  gpio_num_t primary_gpio;
  gpio_num_t secondary_gpio;
} sensor_endpoint_mapping_t;

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

/** Driver Update
 *
 * This API should be called to update the driver for the attribute being updated.
 * This is usually called from the common `app_attribute_update_cb()`.
 *
 * @param[in] endpoint_id Endpoint ID of the attribute.
 * @param[in] cluster_id Cluster ID of the attribute.
 * @param[in] attribute_id Attribute ID of the attribute.
 * @param[in] val Pointer to `esp_matter_attr_val_t`. Use appropriate elements as per the value type.
 *
 * @return ESP_OK on success.
 * @return error in case of failure.
 */
esp_err_t app_driver_attribute_update(app_driver_handle_t driver_handle, uint16_t endpoint_id, uint32_t cluster_id,
                                      uint32_t attribute_id, esp_matter_attr_val_t *val);

/** Read sensor data
 *
 * This reads the current values from the sensor
 *
 * @param[in] sensor_idx Index of the sensor in sensors array
 *
 * @return ESP_OK on success
 */
esp_err_t app_driver_read_sensor_data(uint8_t sensor_idx);

// ============================================================
// Задача опроса Boot Switch
// Генерирует Matter события для одиночного/двойного/длинного нажатия
// ============================================================
#if CONFIG_BOOT_SWITCH_ENABLED
  void boot_switch_task(void *pvParameters);
#endif

/** Task to poll sensors
 *
 * This task periodically reads sensor values and updates Matter attributes
 */
void sensor_polling_task(void *pvParameters);

void app_driver_log_sensor_statistics(void);

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
