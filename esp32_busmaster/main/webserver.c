
#include "webserver.h"

#include "wifi_connect.h"
#include "webserver_spiffs.h"
#include "webserver_sse.h"
#include "ui_values.h"

#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include "cJSON.h"

esp_err_t index_handler(httpd_req_t *req)
{
    httpd_resp_set_status(req, "302 Found");
    httpd_resp_set_hdr(req, "Location", "/index.html");
    httpd_resp_send(req, NULL, 0); // No body needed
    return ESP_OK;
}

esp_err_t fan_speed_handler(httpd_req_t *req)
{
    char buf[32];
    int len = httpd_req_get_url_query_len(req) + 1;

    if (len > 1 && httpd_req_get_url_query_str(req, buf, len) == ESP_OK)
    {
        int fan = 0, speed = 0;
        char param[8];
        if (httpd_query_key_value(buf, "fan", param, sizeof(param)) == ESP_OK)
            fan = atoi(param);
        else
            return ESP_ERR_HTTPD_INVALID_REQ;

        if (httpd_query_key_value(buf, "value", param, sizeof(param)) == ESP_OK)
            speed = atoi(param);
        else
            return ESP_ERR_HTTPD_INVALID_REQ;
        
        if (fan < 1 || fan > 2 || speed < 0 || speed > 100)
            return ESP_ERR_HTTPD_INVALID_REQ;

        ESP_LOGI("FAN", "Set fan %d speed to %d%%", fan, speed);

        ui_values.fan_speeds[fan-1] = speed;
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

const char *TAG = "webserver";

esp_err_t scan_handler(httpd_req_t *req)
{
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true};

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true)); // true = block until done

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);

    wifi_ap_record_t *ap_list = malloc(ap_num * sizeof(wifi_ap_record_t));
    if (ap_list == NULL)
    {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_list));

    // Create JSON array of networks
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_num; ++i)
    {
        cJSON *ap_json = cJSON_CreateObject();
        cJSON_AddStringToObject(ap_json, "ssid", (const char *)ap_list[i].ssid);
        cJSON_AddNumberToObject(ap_json, "rssi", ap_list[i].rssi);
        cJSON_AddNumberToObject(ap_json, "authmode", ap_list[i].authmode);
        cJSON_AddItemToArray(root, ap_json);
    }

    const char *response = cJSON_PrintUnformatted(root);
    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, response, strlen(response));

    // Clean up
    free(ap_list);
    cJSON_Delete(root);
    free((void *)response);

    return ESP_OK;
}

esp_err_t connect_handler(httpd_req_t *req)
{
    char content[256];
    int received = httpd_req_recv(req, content, sizeof(content) - 1);
    if (received <= 0)
    {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[received] = '\0'; // Null-terminate

    cJSON *json = cJSON_Parse(content);
    if (!json)
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    const cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    const cJSON *password = cJSON_GetObjectItem(json, "password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(password))
    {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Missing ssid or password");
        cJSON_Delete(json);
        return ESP_FAIL;
    }

    wifi_config_t wifi_config = {0};
    strncpy((char *)wifi_config.sta.ssid, ssid->valuestring, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char *)wifi_config.sta.password, password->valuestring, sizeof(wifi_config.sta.password) - 1);

    ESP_LOGI("WIFI", "Connecting to SSID: %s", ssid->valuestring);

    ESP_ERROR_CHECK(esp_wifi_disconnect()); // In case already connected
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_connect());

    save_wifi_credentials(ssid->valuestring, password->valuestring);

    httpd_resp_sendstr(req, "Connecting to Wi-Fi...");

    cJSON_Delete(json);
    return ESP_OK;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;
    config.max_open_sockets = 7;
    config.stack_size = 8192;
    config.lru_purge_enable = true;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t index_uri = {
            .uri = "/",
            .method = HTTP_GET,
            .handler = index_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &index_uri);

        httpd_uri_t fan_uri = {
            .uri = "/set_fan_speed",
            .method = HTTP_GET,
            .handler = fan_speed_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &fan_uri);

        sse_init();
        httpd_uri_t sse_uri = {
            .uri = "/events",
            .method = HTTP_GET,
            .handler = sse_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &sse_uri);

        httpd_uri_t scan_uri = {
            .uri = "/scan",
            .method = HTTP_GET,
            .handler = scan_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &scan_uri);

        httpd_uri_t connect_uri = {
            .uri = "/connect",
            .method = HTTP_POST,
            .handler = connect_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &connect_uri);

        if (spiffs_init_fs() == ESP_OK)
        {
            httpd_uri_t common_get_uri = {
                .uri = "/*",
                .method = HTTP_GET,
                .handler = spiffs_get_handler,
                .user_ctx = NULL};
            httpd_register_uri_handler(server, &common_get_uri);
        }
    }
}
