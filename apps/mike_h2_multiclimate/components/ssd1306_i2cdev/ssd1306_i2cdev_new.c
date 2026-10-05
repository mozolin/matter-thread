/**
 * @file ssd1306_i2c_new.c
 * @brief Реализация I2C драйвера для SSD1306 с использованием esp-idf-lib/i2cdev.
 * 
 * ИЗМЕНЕНИЯ:
 * - Удалены прямые вызовы нового драйвера I2C ESP-IDF (i2c_master_bus_config_t, i2c_new_master_bus и т.д.).
 * - Добавлено использование i2c_dev_t из компонента i2cdev для управления шиной и устройством.
 * - Все операции записи переведены на i2c_dev_write_reg.
 * - Добавлена инициализация i2cdev через i2c_dev_create_mutex.
 */

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ssd1306_i2cdev.h"
#include "i2cdev.h" // ДОБАВЛЕНО: Заголовок компонента i2cdev

#define TAG_MIKE_APP "Mike's App"

// УДАЛЕНО: Определения I2C_NUM, I2C_MASTER_FREQ_HZ, I2C_TICKS_TO_WAIT
// Они больше не нужны, так как i2cdev управляет параметрами шины.

// ДОБАВЛЕНО: Статический дескриптор устройства i2cdev для SSD1306
static i2c_dev_t ssd1306_i2c_dev;

/**
 * @brief Инициализация I2C для SSD1306 через i2cdev.
 * 
 * ИЗМЕНЕНО: Функция теперь принимает SSD1306_t для обратной совместимости с существующим кодом,
 * но инициализирует статический ssd1306_i2c_dev и сохраняет его в dev->_i2c_dev_handle для
 * использования в других функциях.
 */
esp_err_t i2c_master_init(SSD1306_t * dev, i2c_port_t port, int16_t sda, int16_t scl, int16_t reset)
{
    esp_err_t err = ESP_OK;
    
    ESP_LOGW(TAG_MIKE_APP, "~~~ Инициализация i2cdev для SSD1306");

    // ДОБАВЛЕНО: Инициализация подсистемы i2cdev (мьютексы, внутреннее состояние)
    // Должна вызываться один раз перед созданием любых устройств.
    // Если i2cdev_init() уже был вызван в app_main, повторный вызов безопасен (он идемпотентен).
    err = i2cdev_init();
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ i2cdev_init() failed! %d (%s)", err, esp_err_to_name(err));
        return err;
    }

    // ДОБАВЛЕНО: Настройка дескриптора i2cdev
    memset(&ssd1306_i2c_dev, 0, sizeof(i2c_dev_t));
    ssd1306_i2c_dev.port = port;//SSD1306_I2C_NUM; // ИЗМЕНЕНО: Используем порт 1 (как в оригинале)
    ssd1306_i2c_dev.addr = SSD1306_I2C_ADDRESS; // 0x3C
    ssd1306_i2c_dev.cfg.sda_io_num = sda;
    ssd1306_i2c_dev.cfg.scl_io_num = scl;
    ssd1306_i2c_dev.cfg.sda_pullup_en = GPIO_PULLUP_ENABLE;
    ssd1306_i2c_dev.cfg.scl_pullup_en = GPIO_PULLUP_ENABLE;
    ssd1306_i2c_dev.cfg.master.clk_speed = 400000; // 400 кГц

    // ДОБАВЛЕНО: Создание мьютекса и регистрация устройства в i2cdev
    err = i2c_dev_create_mutex(&ssd1306_i2c_dev);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ i2c_dev_create_mutex() failed! %d (%s)", err, esp_err_to_name(err));
        return err;
    }

    // Обработка пина сброса (без изменений)
    if (reset >= 0) {
        gpio_reset_pin(reset);
        gpio_set_direction(reset, GPIO_MODE_OUTPUT);
        gpio_set_level(reset, 0);
        vTaskDelay(50 / portTICK_PERIOD_MS);
        gpio_set_level(reset, 1);
    }

    // ИЗМЕНЕНО: Сохраняем указатель на статический дескриптор в SSD1306_t.
    // Это позволяет другим функциям (i2c_init, i2c_display_image) использовать i2cdev.
    dev->_address = SSD1306_I2C_ADDRESS;
    dev->_flip = false;
    dev->_i2c_num = port;//SSD1306_I2C_NUM;
    // Приводим указатель к типу i2c_master_dev_handle_t (который в i2cdev может быть просто void* или i2c_dev_t*)
    // ВАЖНО: В оригинальном коде _i2c_dev_handle использовался для i2c_master_transmit.
    // Теперь мы будем использовать ssd1306_i2c_dev напрямую, но сохраним его для передачи.
    dev->_i2c_dev_handle = (i2c_master_dev_handle_t)&ssd1306_i2c_dev;

    return err;
}

// УДАЛЕНО: Функция i2c_device_add больше не нужна, так как i2cdev управляет устройствами.
// Вместо нее можно использовать i2c_master_init, которая уже создает устройство.

/**
 * @brief Инициализация SSD1306 (отправка команд конфигурации).
 * 
 * ИЗМЕНЕНО: Использование i2c_dev_write_reg для отправки команд.
 */
esp_err_t i2c_init(SSD1306_t * dev, int width, int height)
{
    dev->_width = width;
    dev->_height = height;
    dev->_pages = 8;
    if (dev->_height == 32) dev->_pages = 4;

    uint8_t out_buf[27];
    int out_index = 0;
    out_buf[out_index++] = OLED_CONTROL_BYTE_CMD_STREAM;
    out_buf[out_index++] = OLED_CMD_DISPLAY_OFF;
    out_buf[out_index++] = OLED_CMD_SET_MUX_RATIO;
    if (dev->_height == 64) out_buf[out_index++] = 0x3F;
    if (dev->_height == 32) out_buf[out_index++] = 0x1F;
    out_buf[out_index++] = OLED_CMD_SET_DISPLAY_OFFSET;
    out_buf[out_index++] = 0x00;
    out_buf[out_index++] = OLED_CMD_SET_DISPLAY_START_LINE;
    if (dev->_flip) {
        out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_0;
    } else {
        out_buf[out_index++] = OLED_CMD_SET_SEGMENT_REMAP_1;
    }
    out_buf[out_index++] = OLED_CMD_SET_COM_SCAN_MODE;
    out_buf[out_index++] = OLED_CMD_SET_DISPLAY_CLK_DIV;
    out_buf[out_index++] = 0x80;
    out_buf[out_index++] = OLED_CMD_SET_COM_PIN_MAP;
    if (dev->_height == 64) out_buf[out_index++] = 0x12;
    if (dev->_height == 32) out_buf[out_index++] = 0x02;
    out_buf[out_index++] = OLED_CMD_SET_CONTRAST;
    out_buf[out_index++] = 0xFF;
    out_buf[out_index++] = OLED_CMD_DISPLAY_RAM;
    out_buf[out_index++] = OLED_CMD_SET_VCOMH_DESELCT;
    out_buf[out_index++] = 0x40;
    out_buf[out_index++] = OLED_CMD_SET_MEMORY_ADDR_MODE;
    out_buf[out_index++] = OLED_CMD_SET_PAGE_ADDR_MODE;
    out_buf[out_index++] = 0x00;
    out_buf[out_index++] = 0x10;
    out_buf[out_index++] = OLED_CMD_SET_CHARGE_PUMP;
    out_buf[out_index++] = 0x14;
    out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL;
    out_buf[out_index++] = OLED_CMD_DISPLAY_NORMAL;
    out_buf[out_index++] = OLED_CMD_DISPLAY_ON;

    // ИЗМЕНЕНО: Используем i2c_dev_write_reg вместо i2c_master_transmit.
    // Первый байт в out_buf (OLED_CONTROL_BYTE_CMD_STREAM = 0x00) является "регистром" (control byte).
    // i2cdev ожидает указатель на данные после регистра.
    esp_err_t err = i2c_dev_write_reg(&ssd1306_i2c_dev, out_buf[0], &out_buf[1], out_index - 1);
    
    if (err == ESP_OK) {
        ESP_LOGW(TAG_MIKE_APP, "~~~ Конфигурация OLED прошла успешно");
    } else {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Не удалось записать в устройство [0x%02x]: %d (%s)", dev->_address, err, esp_err_to_name(err));
        err = ESP_FAIL;
    }
    return err;
}

/**
 * @brief Отображение изображения на SSD1306.
 * 
 * ИЗМЕНЕНО: Использование i2c_dev_write_reg для отправки команд и данных.
 */
void i2c_display_image(SSD1306_t * dev, int page, int seg, const uint8_t * images, int width) {
    if (page >= dev->_pages) return;
    if (seg >= dev->_width) return;

    int _seg = seg + CONFIG_OFFSETX;
    uint8_t columLow = _seg & 0x0F;
    uint8_t columHigh = (_seg >> 4) & 0x0F;

    int _page = page;
    if (dev->_flip) {
        _page = (dev->_pages - page) - 1;
    }

    // Отправка команд установки адреса (control byte 0x00)
    uint8_t cmd_buf[3];
    cmd_buf[0] = (0x00 + columLow);
    cmd_buf[1] = (0x10 + columHigh);
    cmd_buf[2] = 0xB0 | _page;

    esp_err_t err = i2c_dev_write_reg(&ssd1306_i2c_dev, OLED_CONTROL_BYTE_CMD_STREAM, cmd_buf, 3);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Не удалось записать команды адреса: %d (%s)", err, esp_err_to_name(err));
    }

    // Отправка данных (control byte 0x40)
    err = i2c_dev_write_reg(&ssd1306_i2c_dev, OLED_CONTROL_BYTE_DATA_STREAM, images, width);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Не удалось записать данные изображения: %d (%s)", err, esp_err_to_name(err));
    }
}

/**
 * @brief Установка контраста.
 * 
 * ИЗМЕНЕНО: Использование i2c_dev_write_reg.
 */
void i2c_contrast(SSD1306_t * dev, int contrast) {
    uint8_t _contrast = contrast;
    if (contrast < 0x0) _contrast = 0;
    if (contrast > 0xFF) _contrast = 0xFF;

    uint8_t cmd_buf[2];
    cmd_buf[0] = OLED_CMD_SET_CONTRAST;
    cmd_buf[1] = _contrast;

    esp_err_t err = i2c_dev_write_reg(&ssd1306_i2c_dev, OLED_CONTROL_BYTE_CMD_STREAM, cmd_buf, 2);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Не удалось установить контраст: %d (%s)", err, esp_err_to_name(err));
    }
}

/**
 * @brief Аппаратная прокрутка.
 * 
 * ИЗМЕНЕНО: Использование i2c_dev_write_reg.
 */
void i2c_hardware_scroll(SSD1306_t * dev, ssd1306_scroll_type_t scroll) {
    uint8_t out_buf[11];
    int out_index = 0;

    if (scroll == SCROLL_RIGHT) {
        out_buf[out_index++] = OLED_CMD_HORIZONTAL_RIGHT;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0xFF;
        out_buf[out_index++] = OLED_CMD_ACTIVE_SCROLL;
    } else if (scroll == SCROLL_LEFT) {
        out_buf[out_index++] = OLED_CMD_HORIZONTAL_LEFT;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0xFF;
        out_buf[out_index++] = OLED_CMD_ACTIVE_SCROLL;
    } else if (scroll == SCROLL_DOWN) {
        out_buf[out_index++] = OLED_CMD_CONTINUOUS_SCROLL;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x3F;
        out_buf[out_index++] = OLED_CMD_VERTICAL;
        out_buf[out_index++] = 0x00;
        if (dev->_height == 64) out_buf[out_index++] = 0x40;
        if (dev->_height == 32) out_buf[out_index++] = 0x20;
        out_buf[out_index++] = OLED_CMD_ACTIVE_SCROLL;
    } else if (scroll == SCROLL_UP) {
        out_buf[out_index++] = OLED_CMD_CONTINUOUS_SCROLL;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x07;
        out_buf[out_index++] = 0x00;
        out_buf[out_index++] = 0x01;
        out_buf[out_index++] = OLED_CMD_VERTICAL;
        out_buf[out_index++] = 0x00;
        if (dev->_height == 64) out_buf[out_index++] = 0x40;
        if (dev->_height == 32) out_buf[out_index++] = 0x20;
        out_buf[out_index++] = OLED_CMD_ACTIVE_SCROLL;
    } else if (scroll == SCROLL_STOP) {
        out_buf[out_index++] = OLED_CMD_DEACTIVE_SCROLL;
    }

    esp_err_t err = i2c_dev_write_reg(&ssd1306_i2c_dev, OLED_CONTROL_BYTE_CMD_STREAM, out_buf, out_index);
    if (err != ESP_OK) {
        ESP_LOGE(TAG_MIKE_APP, "~~~ Не удалось выполнить прокрутку: %d (%s)", err, esp_err_to_name(err));
    }
}
