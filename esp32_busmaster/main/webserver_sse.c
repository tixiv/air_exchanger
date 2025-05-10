#include "webserver_sse.h"
#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

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

void createMessage(char *msg)
{
    float temp = 25.0; // Replace with actual sensor data
    snprintf(msg, 128, "data: { \"temperature\": %.2f }\n\n", temp);
}

static void sse_broadcast_task(void *param) {
    char msg[128];
    char buf[168];

    while (1) {
        createMessage(msg);
        snprintf(buf, sizeof(buf), "%x\r\n%s\r\n", strlen(msg), msg);

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

    char msg[128];
    createMessage(msg);

    httpd_resp_send_chunk(req, msg, strlen(msg));
    // httpd_resp_send_chunk(req, NULL, 0); // flush headers

    int fd = httpd_req_to_sockfd(req);
    add_client(fd);

    return ESP_OK; // don't block the handler!
}
