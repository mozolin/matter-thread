#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  bool is_api;
} uri_context_t;


/**
 * @brief Регистрирует обработчики OTA обновления
 * 
 * @param server Указатель на HTTP сервер
 */
void esp_br_register_ota_handlers(httpd_handle_t server);

// ДОБАВЛЕНО: Чтение файла из SPIFFS
/**
 * @brief Читает файл из SPIFFS
 * 
 * @param path Путь к файлу
 * @param content Указатель на буфер с содержимым
 * @param length Длина содержимого
 * @return esp_err_t Результат операции
 */
esp_err_t read_file_from_spiffs(const char *path, char **content, size_t *length);

#ifdef __cplusplus
}
#endif
