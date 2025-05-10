
#include "webserver_spiffs.h"
#include <esp_spiffs.h>
#include <esp_log.h>
#include <errno.h>

#define ESP_VFS_PATH_MAX 64

static const char *TAG = "webserver_spiffs";

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

esp_err_t spiffs_get_handler(httpd_req_t *req) {
    char filepath[ESP_VFS_PATH_MAX];
    const char *uri = req->uri;

    // Construct actual file path
    snprintf(filepath, sizeof(filepath), "/spiffs%.50s", uri);

    // Check for .gz version first
    char gz_filepath[ESP_VFS_PATH_MAX];
    snprintf(gz_filepath, sizeof(gz_filepath), "%.60s.gz", filepath);

    FILE *file = fopen(gz_filepath, "r");
    bool is_gzipped = false;

    if (file) {
        strcpy(filepath, gz_filepath);  // Use gz file
        is_gzipped = true;
    } else {
        file = fopen(filepath, "r");  // Fallback to non-gz
    }

    if (!file) {
        ESP_LOGE(TAG, "File %s not found. errno=%d", filepath, errno);
        ESP_ERROR_CHECK(httpd_resp_send_404(req));
        return ESP_FAIL;
    }

    ESP_LOGI(TAG, "Opened file %s", filepath);

    // Guess content type
    const char *content_type = "text/plain";
    if (strstr(uri, ".html")) content_type = "text/html";
    else if (strstr(uri, ".css")) content_type = "text/css";
    else if (strstr(uri, ".js")) content_type = "application/javascript";
    else if (strstr(uri, ".ico")) content_type = "image/x-icon";

    ESP_ERROR_CHECK(httpd_resp_set_type(req, content_type));

    if (is_gzipped) {
        ESP_ERROR_CHECK(httpd_resp_set_hdr(req, "Content-Encoding", "gzip"));
    }

    char chunk[1024];
    size_t read_bytes;
    while ((read_bytes = fread(chunk, 1, sizeof(chunk), file)) > 0) {
        if (httpd_resp_send_chunk(req, chunk, read_bytes) != ESP_OK)
        {
            ESP_LOGE(TAG, "httpd_resp_send_chunk failed.");
            break;
        }
    }

    if (read_bytes < 0)
    {
        ESP_LOGE(TAG, "Fread failed. errno=%d", errno);
    }

    fclose(file);
    httpd_resp_send_chunk(req, NULL, 0); // End response
    return ESP_OK;
}

esp_err_t spiffs_init_fs(void)
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
