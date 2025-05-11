
#include "webserver_sse.h"

#include "ui_values.h"
#include "bus_master.h"

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "cJSON.h"

struct UI_values ui_values;

#define MAX_CLIENTS 5

static int client_fds[MAX_CLIENTS];
static SemaphoreHandle_t client_mutex;

static void add_client(int fd) {
    xSemaphoreTake(client_mutex, portMAX_DELAY);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (client_fds[i] == -1) {
            client_fds[i] = fd;
            break;
        }
    }
    xSemaphoreGive(client_mutex);
}

const char *createMessage()
{
    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "fan_speed1", ui_values.fan_speeds[0]);
    cJSON_AddNumberToObject(root, "fan_speed2", ui_values.fan_speeds[1]);

    if (xSemaphoreTake(rs485_data_mutex, portMAX_DELAY))
	{
        cJSON_AddNumberToObject(root, "fan_rpm1", rs485_data.fan_rpms[0]);
        cJSON_AddNumberToObject(root, "fan_rpm2", rs485_data.fan_rpms[1]);
        cJSON_AddNumberToObject(root, "temperature1", rs485_data.temperatures[0]);
        cJSON_AddNumberToObject(root, "temperature2", rs485_data.temperatures[1]);
        cJSON_AddNumberToObject(root, "temperature3", rs485_data.temperatures[2]);
        cJSON_AddNumberToObject(root, "temperature4", rs485_data.temperatures[3]);
        xSemaphoreGive(rs485_data_mutex);
    }

    const char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    int alloc_size = strlen(json_str) + 10;
    char *sse_str = malloc(alloc_size);
    snprintf(sse_str, alloc_size, "data: %s\n\n", json_str);
    
    free((void*)json_str);

    return sse_str;
}

static void sse_broadcast_task(void *param) {
    char buf[256];

    while (1) {
        const char * sse_str = createMessage();

        snprintf(buf, sizeof(buf), "%x\r\n%s\r\n", strlen(sse_str), sse_str);
        free((void *)sse_str);

        xSemaphoreTake(client_mutex, portMAX_DELAY);
        for (int i = 0; i < MAX_CLIENTS; i++) {
            int fd = client_fds[i];
            if (fd != -1) {
                int sent = send(fd, buf, strlen(buf), 0);
                if (sent < 0) {
                    close(fd);
                    client_fds[i] = -1;
                }
            }
        }
        xSemaphoreGive(client_mutex);

        vTaskDelay(pdMS_TO_TICKS(2000));
    }
}

void sse_init(void) {
    for (int i = 0; i < MAX_CLIENTS; i++) {
        client_fds[i] = -1;
    }
    client_mutex = xSemaphoreCreateMutex();
    xTaskCreate(sse_broadcast_task, "sse_broadcast", 4096, NULL, 5, NULL);
}

esp_err_t sse_handler(httpd_req_t *req) {
    httpd_resp_set_type(req, "text/event-stream");
    httpd_resp_set_hdr(req, "Cache-Control", "no-cache");
    httpd_resp_set_hdr(req, "Connection", "keep-alive");

    const char * json_str = createMessage();
    httpd_resp_send_chunk(req, json_str, strlen(json_str));

    free((void *)json_str);

    int fd = httpd_req_to_sockfd(req);
    add_client(fd);

    return ESP_OK; // don't block the handler!
}
