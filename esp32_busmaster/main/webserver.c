
#include "webserver.h"

#include "wifi_connect.h"

#include <esp_http_server.h>
#include <esp_log.h>
#include <esp_spiffs.h>
#include <esp_wifi.h>
#include "cJSON.h"

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

    if (len > 1 && httpd_req_get_url_query_str(req, buf, len) == ESP_OK)
    {
        char param[8];
        if (httpd_query_key_value(buf, "val", param, sizeof(param)) == ESP_OK)
        {
            int speed = atoi(param);
            ESP_LOGI("FAN", "Set fan speed to %d%%", speed);
            // TODO: Call fan control function here
        }
    }

    httpd_resp_send(req, NULL, 0);
    return ESP_OK;
}

esp_err_t spiffs_handler(httpd_req_t *req)
{
    char filepath[64];
    snprintf(filepath, sizeof(filepath), "/spiffs%.50s", req->uri);

    printf("spiffs_handler: uri = \"%s\"\n", req->uri);
    printf("spiffs_handler: filePath = \"%s\"\n", filepath);

    FILE *file = fopen(filepath, "r");
    if (!file)
    {
        printf("Couldn't open file\n");
        httpd_resp_send_404(req);
        return ESP_FAIL;
    }

    char chunk[512];
    size_t read_bytes;
    while ((read_bytes = fread(chunk, 1, sizeof(chunk), file)) > 0)
    {
        httpd_resp_send_chunk(req, chunk, read_bytes);
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // End response
    return ESP_OK;
}

esp_err_t get_status_handler(httpd_req_t *req) {
    // Simulated values — replace with actual sensor readings
    int fan_speed = 55;
    float temperature = 23.4;
    float humidity = 45.6;

    // Create JSON object
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "fan_speed", fan_speed);
    cJSON_AddNumberToObject(root, "temperature", temperature);
    cJSON_AddNumberToObject(root, "humidity", humidity);

    const char *json_str = cJSON_PrintUnformatted(root);

    httpd_resp_set_type(req, "application/json");
    httpd_resp_send(req, json_str, strlen(json_str));

    cJSON_Delete(root);
    free((void *)json_str);

    return ESP_OK;
}

esp_err_t sse_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");

    while (1) {
        char msg[64];
        float temp = 12.5;
        snprintf(msg, sizeof(msg), "data: { \"temperature\": %.2f }\n\n", temp);

        if (httpd_resp_send_chunk(req, msg, strlen(msg)) != ESP_OK) {
            break; // client disconnected
        }

        vTaskDelay(pdMS_TO_TICKS(2000));  // send every 2 seconds
    }

    httpd_resp_send_chunk(req, NULL, 0); // close chunked response
    return ESP_OK;
}

const char *TAG = "webserver";

esp_err_t scan_handler(httpd_req_t *req)
{
    wifi_scan_config_t scan_config = {
        .ssid = 0,
        .bssid = 0,
        .channel = 0,
        .show_hidden = true
    };

    ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));  // true = block until done

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);

    wifi_ap_record_t *ap_list = malloc(ap_num * sizeof(wifi_ap_record_t));
    if (ap_list == NULL) {
        httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Memory allocation failed");
        return ESP_FAIL;
    }

    ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_num, ap_list));

    // Create JSON array of networks
    cJSON *root = cJSON_CreateArray();
    for (int i = 0; i < ap_num; ++i) {
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
    if (received <= 0) {
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    content[received] = '\0'; // Null-terminate

    cJSON *json = cJSON_Parse(content);
    if (!json) {
        httpd_resp_send_err(req, HTTPD_400_BAD_REQUEST, "Invalid JSON");
        return ESP_FAIL;
    }

    const cJSON *ssid = cJSON_GetObjectItem(json, "ssid");
    const cJSON *password = cJSON_GetObjectItem(json, "password");

    if (!cJSON_IsString(ssid) || !cJSON_IsString(password)) {
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

esp_err_t init_fs(void)
{
    esp_vfs_spiffs_conf_t conf = {
        .base_path = "/spiffs",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = false};
    esp_err_t ret = esp_vfs_spiffs_register(&conf);

    if (ret != ESP_OK)
    {
        if (ret == ESP_FAIL)
        {
            ESP_LOGE(TAG, "Failed to mount or format filesystem");
        }
        else if (ret == ESP_ERR_NOT_FOUND)
        {
            ESP_LOGE(TAG, "Failed to find SPIFFS partition");
        }
        else
        {
            ESP_LOGE(TAG, "Failed to initialize SPIFFS (%s)", esp_err_to_name(ret));
        }
        return ESP_FAIL;
    }

    size_t total = 0, used = 0;
    ret = esp_spiffs_info(NULL, &total, &used);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to get SPIFFS partition information (%s)", esp_err_to_name(ret));
    }
    else
    {
        ESP_LOGI(TAG, "Partition size: total: %d, used: %d", total, used);
    }
    return ESP_OK;
}

void start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn = httpd_uri_match_wildcard;

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

        httpd_uri_t get_status_uri = {
            .uri = "/status",
            .method = HTTP_GET,
            .handler = get_status_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &get_status_uri);

        httpd_uri_t sse_uri = {
            .uri = "/events",
            .method = HTTP_GET,
            .handler = sse_handler,
            .user_ctx = NULL};
        httpd_register_uri_handler(server, &sse_uri);

        httpd_uri_t scan_uri = {
            .uri      = "/scan",
            .method   = HTTP_GET,
            .handler  = scan_handler,
            .user_ctx = NULL
        };
        httpd_register_uri_handler(server, &scan_uri);

        httpd_uri_t connect_uri = {
            .uri       = "/connect",
            .method    = HTTP_POST,
            .handler   = connect_handler,
            .user_ctx  = NULL
        };
        httpd_register_uri_handler(server, &connect_uri);        

        if (init_fs() == ESP_OK)
        {
            httpd_uri_t common_get_uri = {
                .uri = "/*",
                .method = HTTP_GET,
                .handler = spiffs_handler,
                .user_ctx = NULL
            };
            httpd_register_uri_handler(server, &common_get_uri);
        }
    }
}
