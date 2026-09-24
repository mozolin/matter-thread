#include "i2c_bus.h"
#include "app_priv.h"
#include <esp_log.h>

static const char *TAG = "I2C_BUS";
static i2c_master_bus_handle_t s_bus_handle = NULL;
static int s_bus_users = 0;

esp_err_t i2c_bus_init(gpio_num_t sda_pin, gpio_num_t scl_pin, i2c_port_t port)
{
    if (s_bus_handle != NULL) {
        s_bus_users++;
        ESP_LOGI(TAG, "I2C bus already initialized, users: %d", s_bus_users);
        return ESP_OK;
    }
    
    i2c_master_bus_config_t bus_config = {
        .i2c_port = port,
        .sda_io_num = sda_pin,
        .scl_io_num = scl_pin,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags = {
            .enable_internal_pullup = true,
        },
    };
    
    esp_err_t err = i2c_new_master_bus(&bus_config, &s_bus_handle);
    if (err == ESP_OK) {
        s_bus_users = 1;
        ESP_LOGI(TAG, "I2C bus initialized on port %d", port);
    } else {
        ESP_LOGE(TAG, "Failed to initialize I2C bus: %d", err);
    }
    
    return err;
}

void i2c_bus_deinit(void)
{
    if (s_bus_handle != NULL) {
        s_bus_users--;
        ESP_LOGI(TAG, "I2C bus user released, remaining users: %d", s_bus_users);
        
        if (s_bus_users <= 0) {
            i2c_del_master_bus(s_bus_handle);
            s_bus_handle = NULL;
            ESP_LOGI(TAG, "I2C bus deinitialized");
        }
    }
}

esp_err_t i2c_bus_add_device(uint8_t dev_addr, uint32_t clk_speed_hz, i2c_master_dev_handle_t *dev_handle)
{
    if (s_bus_handle == NULL) {
        ESP_LOGE(TAG, "I2C bus not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = dev_addr,
        .scl_speed_hz = clk_speed_hz,
    };
    
    return i2c_master_bus_add_device(s_bus_handle, &dev_config, dev_handle);
}

i2c_master_bus_handle_t i2c_bus_get_handle(void)
{
    return s_bus_handle;
}