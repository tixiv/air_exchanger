#pragma once

#include <esp_http_server.h>

esp_err_t spiffs_init_fs(void);
esp_err_t spiffs_get_handler(httpd_req_t *req);
