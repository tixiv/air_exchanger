
#include "telnet_server.h"
#include "bus_master.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <string.h>
#include "shell_task.h"

#define TELNET_PORT 23
static const char *TAG = "telnet_server";

void telnet_server_task(void *pvParameters)
{
    int listen_sock = -1;
    int client_sock = -1;

    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    listen_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_IP);
    if (listen_sock < 0) {
        ESP_LOGE(TAG, "Unable to create socket: errno %d", errno);
        vTaskDelete(NULL);
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(TELNET_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        ESP_LOGE(TAG, "Socket unable to bind: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    if (listen(listen_sock, 1) < 0) {
        ESP_LOGE(TAG, "Error during listen: errno %d", errno);
        close(listen_sock);
        vTaskDelete(NULL);
        return;
    }

    ESP_LOGI(TAG, "Telnet server listening on port %d", TELNET_PORT);

    while (1) {
        client_sock = accept(listen_sock, (struct sockaddr *)&client_addr, &client_addr_len);
        if (client_sock < 0) {
            ESP_LOGE(TAG, "Unable to accept connection: errno %d", errno);
            break;
        }

        ESP_LOGI(TAG, "New telnet client connected");

		int *parameter = malloc(sizeof(int));
		*parameter = client_sock;

		if (xTaskCreate(shell_task, "shell_task", 4096, parameter, 5, NULL) != pdPASS) {
			ESP_LOGE(TAG, "Failed to create shell task");
			close(client_sock);
			free(parameter);
		}

    }

    close(listen_sock);
    vTaskDelete(NULL);
}
