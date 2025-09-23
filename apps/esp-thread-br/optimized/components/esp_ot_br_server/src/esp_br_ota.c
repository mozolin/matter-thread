#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "esp_http_server.h"
#include "esp_spiffs.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_system.h"
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_br_ota.h"

#define TAG_MIKE_OTA "Mike's OTBR OTA"

//-- Read file from SPIFFS
static esp_err_t read_file_from_spiffs(const char *path, char **content, size_t *length)
{
  FILE *file = fopen(path, "r");
  if (file == NULL) {
    ESP_LOGE(TAG_MIKE_OTA, "~~~ Failed to open file: %s", path);
    return ESP_FAIL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  *content = malloc(file_size + 1);
  if (*content == NULL) {
    fclose(file);
    ESP_LOGE(TAG_MIKE_OTA, "~~~ Failed to allocate memory for file content");
    return ESP_FAIL;
  }

  size_t read_size = fread(*content, 1, file_size, file);
  fclose(file);

  if (read_size != file_size) {
    free(*content);
    ESP_LOGE(TAG_MIKE_OTA, "~~~ Failed to read file: %s", path);
    return ESP_FAIL;
  }

  (*content)[file_size] = '\0';
  *length = file_size;

  return ESP_OK;
}

//-- OTA page handler
static esp_err_t ota_page_handler(httpd_req_t *req)
{
  char *html_content = NULL;
  size_t html_length = 0;

  esp_err_t ret = read_file_from_spiffs("/spiffs/ota.html", &html_content, &html_length);
  if (ret != ESP_OK) {
    ESP_LOGE(TAG_MIKE_OTA, "~~~ Failed to read OTA HTML file");
    return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to load OTA page");
  }

  httpd_resp_set_type(req, "text/html");
  httpd_resp_send(req, html_content, html_length);
  free(html_content);

  return ESP_OK;
}

//-- Device Info page handler
static esp_err_t device_info_handler(httpd_req_t *req) {
  const esp_app_desc_t *app_desc = esp_app_get_description();
  const esp_partition_t *running = esp_ota_get_running_partition();
  const esp_partition_t *update_partition = esp_ota_get_next_update_partition(NULL);
  
  char running_partition[32] = "unknown";
  char update_partition_str[32] = "unknown";
  
  if (running) {
    snprintf(running_partition, sizeof(running_partition), "%s", running->label);
  }
  
  if (update_partition) {
    snprintf(update_partition_str, sizeof(update_partition_str), "%s", update_partition->label);
  }

  char response[512];
  int length = snprintf(response, sizeof(response),
            "{\"id\":\"%s\",\"version\":\"%s\",\"free_heap\":%lu,"
            "\"project_name\":\"%s\",\"running_partition\":\"%s\","
            "\"update_partition\":\"%s\",\"secure_version\":%lu,"
            "\"sdk_version\":\"%s\"}",
            #ifdef CONFIG_MIKE_DEVICE_ID
              CONFIG_MIKE_DEVICE_ID,
            #else
              "ESP32-Thread-BR",
            #endif
            #ifdef CONFIG_MIKE_FIRMWARE_VERSION
              CONFIG_MIKE_FIRMWARE_VERSION,
            #else
              app_desc->version,
            #endif
            esp_get_free_heap_size(),
            app_desc->project_name,
            running_partition,    // Это поле должно быть!
            update_partition_str,
            app_desc->secure_version,
            app_desc->idf_ver);

  //-- Logging the response for debugging
  //ESP_LOGW(TAG_MIKE_OTA, "~~~ API response: %s", response);

  httpd_resp_set_type(req, "application/json");
  return httpd_resp_send(req, response, length);
}

#include <ctype.h>

//-- Structure for multipart parsing
typedef struct {
  char boundary[128];
  int boundary_len;
  bool in_header;
  bool in_data;
  int data_start;
} multipart_parser_t;

//-- Function for finding a boundary in data
static int find_boundary(const char *data, int data_len, const char *boundary, int boundary_len) {
  for (int i = 0; i <= data_len - boundary_len; i++) {
    if (memcmp(data + i, boundary, boundary_len) == 0) {
      return i;
    }
  }
  return -1;
}

//-- Function to extract boundary from Content-Type
static void extract_boundary(const char *content_type, char *boundary, int max_len) {
  const char *boundary_start = strstr(content_type, "boundary=");
  if (boundary_start) {
    boundary_start += 9; // "boundary="
    const char *boundary_end = strchr(boundary_start, ';');
    if (!boundary_end) boundary_end = boundary_start + strlen(boundary_start);
    
    int len = boundary_end - boundary_start;
    if (len > max_len - 1) len = max_len - 1;
    
    strncpy(boundary, boundary_start, len);
    boundary[len] = '\0';
    
    //-- Adding "--" to the beginning of boundary
    memmove(boundary + 2, boundary, len + 1);
    boundary[0] = '-';
    boundary[1] = '-';
  }
}

//-- OTA update handler with multipart support
static esp_err_t ota_update_handler(httpd_req_t *req) {
  esp_ota_handle_t ota_handle = 0;
  const esp_partition_t *update_partition = NULL;
  int total_received = 0;
  int total_written = 0;
  esp_err_t err = ESP_OK;
  multipart_parser_t parser = {0};
  bool ota_active = false;

  ESP_LOGW(TAG_MIKE_OTA, "~~~ Starting OTA update process");

  //-- Getting Content-Type
  char content_type[128] = {0};
  if (httpd_req_get_hdr_value_len(req, "Content-Type") > 0) {
    httpd_req_get_hdr_value_str(req, "Content-Type", content_type, sizeof(content_type));
    ESP_LOGW(TAG_MIKE_OTA, "~~~ Content-Type: %s", content_type);
  }

  //-- Getting Content-Length
  size_t content_length = 0;
  char content_len_str[32];
  if (httpd_req_get_hdr_value_len(req, "Content-Length") > 0) {
    httpd_req_get_hdr_value_str(req, "Content-Length", content_len_str, sizeof(content_len_str));
    content_length = atoi(content_len_str);
    ESP_LOGW(TAG_MIKE_OTA, "~~~ Content-Length: %d", content_length);
  }

  //-- Initializing the parser for multipart
  if (strstr(content_type, "multipart/form-data") != NULL) {
    extract_boundary(content_type, parser.boundary, sizeof(parser.boundary));
    parser.boundary_len = strlen(parser.boundary);
    parser.in_header = true;
    ESP_LOGW(TAG_MIKE_OTA, "~~~ Multipart boundary: %s", parser.boundary);
  }

  //-- Read buffer
  char buffer[4096];

  //-- Reading the data
  while (1) {
    int received = httpd_req_recv(req, buffer, sizeof(buffer));
    
    if (received > 0) {
      total_received += received;
      
      if (parser.boundary_len > 0) {
        //-- Processing multipart data
        if (!ota_active) {
          //-- Finding the beginning of file data
          if (parser.in_header) {
            //-- Finding the end of headers (empty line)
            char *header_end = memmem(buffer, received, "\r\n\r\n", 4);
            if (header_end) {
              int header_size = header_end - buffer + 4;
              parser.in_header = false;
              parser.in_data = true;
              
              //-- The remaining data is the beginning of the file.
              int data_size = received - header_size;
              if (data_size > 0) {
                //-- Starting OTA
                update_partition = esp_ota_get_next_update_partition(NULL);
                if (!update_partition) {
                  ESP_LOGE(TAG_MIKE_OTA, "~~~ No OTA partition found");
                  err = ESP_FAIL;
                  break;
                }
                
                err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
                if (err != ESP_OK) {
                  ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA begin failed");
                  break;
                }
                ota_active = true;
                
                //-- Writing the first data
                err = esp_ota_write(ota_handle, buffer + header_size, data_size);
                if (err != ESP_OK) {
                  ESP_LOGE(TAG_MIKE_OTA, "~~~ First OTA write failed");
                  break;
                }
                total_written += data_size;
              }
            }
          } else if (parser.in_data) {
            //-- Finding a boundary in the data (end of file)
            int boundary_pos = find_boundary(buffer, received, parser.boundary, parser.boundary_len);
            if (boundary_pos >= 0) {
              //-- Found a boundary - write data up to the boundary
              if (boundary_pos > 0) {
                err = esp_ota_write(ota_handle, buffer, boundary_pos - 2); // -2 для \r\n
                if (err != ESP_OK) {
                  ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA write failed before boundary");
                  break;
                }
                total_written += boundary_pos - 2;
              }
              break; //-- EOF
            } else {
              //-- Boundary not found - writing all data
              err = esp_ota_write(ota_handle, buffer, received);
              if (err != ESP_OK) {
                ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA write failed");
                break;
              }
              total_written += received;
            }
          }
        }
      } else {
        //-- Direct binary download (not multipart)
        if (!ota_active) {
          update_partition = esp_ota_get_next_update_partition(NULL);
          if (!update_partition) {
            ESP_LOGE(TAG_MIKE_OTA, "~~~ No OTA partition found");
            err = ESP_FAIL;
            break;
          }
          
          err = esp_ota_begin(update_partition, OTA_SIZE_UNKNOWN, &ota_handle);
          if (err != ESP_OK) {
            ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA begin failed");
            break;
          }
          ota_active = true;
        }
        
        //-- Checking the magic byte for raw binary
        if (total_received == received && received >= 4) {
          uint8_t magic_byte = (uint8_t)buffer[0];
          if (magic_byte != 0xE9 && magic_byte != 0xEA) {
            ESP_LOGE(TAG_MIKE_OTA, "~~~ Invalid magic byte: 0x%02x", magic_byte);
            err = ESP_ERR_OTA_VALIDATE_FAILED;
            break;
          }
        }
        
        err = esp_ota_write(ota_handle, buffer, received);
        if (err != ESP_OK) {
          ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA write failed");
          break;
        }
        total_written += received;
      }
      
    } else if (received == 0) {
      //-- End of transmission
      break;
    } else {
      //-- Error
      if (received == HTTPD_SOCK_ERR_TIMEOUT) continue;
      ESP_LOGE(TAG_MIKE_OTA, "~~~ Receive error: %d", received);
      err = ESP_FAIL;
      break;
    }
  }

  //-- Funishing the OTA process
  if (ota_active) {
    if (err == ESP_OK) {
      err = esp_ota_end(ota_handle);
      if (err == ESP_OK) {
        err = esp_ota_set_boot_partition(update_partition);
        if (err == ESP_OK) {
          ESP_LOGW(TAG_MIKE_OTA, "~~~ OTA successful! Written %d bytes", total_written);
          
          httpd_resp_set_type(req, "application/json");
          char success_msg[256];
          snprintf(success_msg, sizeof(success_msg),
              "{\"status\":\"success\",\"message\":\"OTA update completed\","
              "\"bytes_written\":%d}", total_written);
          httpd_resp_send(req, success_msg, strlen(success_msg));
          
          vTaskDelay(3000 / portTICK_PERIOD_MS);
          esp_restart();
          return ESP_OK;
        }
      }
    }
    esp_ota_abort(ota_handle);
  }

  //-- Error handling
  ESP_LOGE(TAG_MIKE_OTA, "~~~ OTA failed: %s", esp_err_to_name(err));
  httpd_resp_set_type(req, "application/json");
  char error_msg[256];
  snprintf(error_msg, sizeof(error_msg),
      "{\"status\":\"error\",\"message\":\"%s\",\"received\":%d,\"written\":%d}",
      esp_err_to_name(err), total_received, total_written);
  return httpd_resp_send(req, error_msg, strlen(error_msg));
}

//-- OTA handler registration function
void esp_br_register_ota_handlers(httpd_handle_t server)
{
  //-- OTA web page
  httpd_uri_t ota_uri = {
    .uri = "/ota",
    .method = HTTP_GET,
    .handler = ota_page_handler,
    .user_ctx = NULL
  };
  httpd_register_uri_handler(server, &ota_uri);

  //-- API device info
  httpd_uri_t device_info_uri = {
    .uri = "/api/device-info",
    .method = HTTP_GET,
    .handler = device_info_handler,
    .user_ctx = NULL
  };
  httpd_register_uri_handler(server, &device_info_uri);

  //-- API OTA update
  httpd_uri_t ota_update_uri = {
    .uri = "/api/ota-update",
    .method = HTTP_POST,
    .handler = ota_update_handler,
    .user_ctx = NULL
  };
  httpd_register_uri_handler(server, &ota_update_uri);

  ESP_LOGW(TAG_MIKE_OTA, "~~~ OTA handlers registered successfully");
}
