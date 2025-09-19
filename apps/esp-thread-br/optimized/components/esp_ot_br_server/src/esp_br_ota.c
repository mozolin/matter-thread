#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_br_ota.h"

static const char *TAG = "BR_OTA";

uri_context_t api_ctx = {
  .is_api = true
};

// Функция для чтения файлов из SPIFFS
esp_err_t read_file_from_spiffs(const char *path, char **content, size_t *length)
{
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        ESP_LOGE(TAG, "Failed to open file: %s", path);
        return ESP_FAIL;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    *content = malloc(file_size + 1);
    if (*content == NULL) {
        fclose(file);
        ESP_LOGE(TAG, "Failed to allocate memory for file content");
        return ESP_FAIL;
    }

    size_t read_size = fread(*content, 1, file_size, file);
    fclose(file);

    if (read_size != file_size) {
        free(*content);
        ESP_LOGE(TAG, "Failed to read file: %s", path);
        return ESP_FAIL;
    }

    (*content)[file_size] = '\0';
    *length = file_size;

    return ESP_OK;
}

// Обработчик страницы OTA
static esp_err_t ota_page_handler(httpd_req_t *req)
{
    char *html_content = NULL;
    size_t html_length = 0;

    esp_err_t ret = read_file_from_spiffs("/spiffs/ota.html", &html_content, &html_length);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to read OTA HTML file");
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load OTA page");
    }

    httpd_resp_set_type(req, "text/html");
    httpd_resp_send(req, html_content, html_length);
    free(html_content);

    return ESP_OK;
}

// Обработчик информации об устройстве
static esp_err_t device_info_handler(httpd_req_t *req)
{
    char response[256];
    int length = snprintf(response, sizeof(response),
                        "{\"id\":\"%s\",\"version\":\"%s\",\"free_heap\":%lu,"
                        "\"project_name\":\"%s\",\"sdk_version\":\"%s\"}",
                        "ESP32-Border-Router", 
                        "1.0.0", 
                        esp_get_free_heap_size(),
                        "esp-thread-br",
                        IDF_VER);

    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, response, length);
}

// Обработчик OTA обновления (заглушка)
static esp_err_t ota_update_handler(httpd_req_t *req)
{
    ESP_LOGI(TAG, "OTA update requested");
    
    // В этой версии OTA не реализовано из-за отсутствия esp_ota_ops.h
    // Можно реализовать простую файловую загрузку или использовать альтернативные методы
    
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_sendstr(req, "{\"status\":\"error\",\"message\":\"OTA not available in current configuration\"}");
}

// Функция регистрации OTA обработчиков
void esp_br_register_ota_handlers(httpd_handle_t server)
{
    httpd_uri_t ota_uri = {
        .uri = "/ota",
        .method = HTTP_GET,
        .handler = ota_page_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &ota_uri);

    httpd_uri_t device_info_uri = {
        .uri = "/api/device-info",
        .method = HTTP_GET,
        .handler = device_info_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &device_info_uri);

    httpd_uri_t ota_update_uri = {
        .uri = "/api/ota-update",
        .method = HTTP_POST,
        .handler = ota_update_handler,
        .user_ctx = NULL
    };
    httpd_register_uri_handler(server, &ota_update_uri);

    ESP_LOGI(TAG, "OTA handlers registered (basic version)");
}
