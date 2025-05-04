
#include "telnet_server.h"
#include "bus_master.h"
#include "esp_log.h"
#include "lwip/sockets.h"
#include <string.h>
#include "shell_task.h"

#define TELNET_PORT 23
static const char *TAG = "telnet_server";

void print_temperature(int16_t t)
{
	if (t < 0)
	{
		printf("-");
		t = -t;
	}

	printf("%d.%d°C", t / 10, t % 10);
}

void print_status(void)
{
	for (uint8_t i = 0; i < 4; i++)
	{
		printf("T%d: ", i + 1);
		print_temperature(fan_rx_data.temperatures[i]);
		printf(", ");
	}

	printf("Fan1: %d, Fan2: %d, Heater: %d, ", fan_tx_data.fan_pwm[0], fan_tx_data.fan_pwm[1], heater_rx_data.heater_duty_readback);

	printf("ADC0: %d, ADC1: %d\r\n", mainboard_rx_data.adc_values[0], mainboard_rx_data.adc_values[1]);
}



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

#if 0
#define PC_BUF_SIZE 100

void handle_pc()
{
	static char buffer[PC_BUF_SIZE];
	static char *pos = buffer;
	static int delay_count;

	uint8_t tmp;
	if (uart_0_getc_nb(&tmp))
	{
		uart_0_putc(tmp);

		if ((pos < buffer + PC_BUF_SIZE - 1)) // buffer full ?
		{
			*pos++ = tmp; // no: write character to buffer
		}

		if (tmp == '\r')
		{
			*pos = 0;	  // terminate line
			pos = buffer; // reset pointer

			/* line received from PC */

			// uart_0_putstr(buffer); //echo to PC

			int fan_id;
			int value;

			if (sscanf(buffer, "fan %d %d", &fan_id, &value) == 2 && (fan_id == 1 || fan_id == 2) && value >= 0 && value <= 1023)
			{
				fan_tx_data.fan_pwm[fan_id - 1] = value;
			}
			else if (sscanf(buffer, "heater %d", &value) == 1 && value >= 0 && value <= 100)
			{
				heater_tx_data.heater_duty = value;
			}
		}
	}

	if (pos == buffer)
	{
		if (delay_count++ == 10000)
		{
			delay_count = 0;
			print_status();
		}
	}
	else
	{
		delay_count = 0;
	}
}
#endif
