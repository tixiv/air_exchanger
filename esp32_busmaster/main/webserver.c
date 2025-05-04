
#include <esp_http_server.h>
#include <esp_log.h>
#include "webserver.h"


static const char *html_page = 
"<!DOCTYPE html><html><body>"
"<h2>Fan Control</h2>"
"<label for='fan1'>Fan Speed:</label>"
"<input type='range' min='0' max='100' value='50' id='fan1' oninput='updateFan(this.value)'>"
"<p>Speed: <span id='speed'>50</span>%</p>"
"<script>"
"function updateFan(val) {"
"  document.getElementById('speed').innerText = val;"
"  fetch('/set_fan_speed?val=' + val);"
"}"
"</script>"
"</body></html>";

esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_send(req, html_page, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

esp_err_t fan_speed_handler(httpd_req_t *req)
{
    char buf[32];
    int len = httpd_req_get_url_query_len(req) + 1;

    if (len > 1 && httpd_req_get_url_query_str(req, buf, len) == ESP_OK) {
        char param[8];
        if (httpd_query_key_value(buf, "val", param, sizeof(param)) == ESP_OK) {
            int speed = atoi(param);
            ESP_LOGI("FAN", "Set fan speed to %d%%", speed);
            // TODO: Call fan control function here
        }
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_uri_t index_uri = {
            .uri      = "/",
            .method   = HTTP_GET,
            .handler  = index_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t fan_uri = {
            .uri      = "/set_fan_speed",
            .method   = HTTP_GET,
            .handler  = fan_speed_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &fan_uri);
    }
}
