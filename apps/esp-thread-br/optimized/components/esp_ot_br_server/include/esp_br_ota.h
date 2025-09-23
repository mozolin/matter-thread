#pragma once

#include "esp_err.h"
#include "esp_http_server.h"

#define HANDLER_TYPE_API      456

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Регистрирует обработчики OTA обновления
 * 
 * @param server Указатель на HTTP сервер
 */
void esp_br_register_ota_handlers(httpd_handle_t server);

#ifdef __cplusplus
}
#endif
