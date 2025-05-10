#pragma once

#include <esp_http_server.h>

void sse_init(void);
esp_err_t sse_handler(httpd_req_t *req);
